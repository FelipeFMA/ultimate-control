/**
 * @file VolumeManager.cpp
 * @brief Implementation of the audio volume management functionality
 *
 * This file implements the VolumeManager class which provides an interface
 * for controlling audio devices (sinks and sources) using PulseAudio.
 * It uses the PIMPL idiom to hide implementation details.
 */

#include "VolumeManager.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>

namespace Volume {

/**
 * @class VolumeManager::Impl
 * @brief Private implementation of the VolumeManager class
 *
 * This class implements the actual audio device management functionality using
 * PulseAudio's command-line interface (pactl).
 */
class VolumeManager::Impl {
public:
    /**
     * @brief Constructor for the implementation class
     */
    Impl() = default;

    /**
     * @brief Destructor for the implementation class
     */
    ~Impl() = default;

    /**
     * @brief Scan for available audio devices
     *
     * Uses pactl to scan for available audio sinks and sources.
     * Populates the sinks_ vector with the results.
     * Calls the update callback when finished if one is registered.
     */
    void refresh_sinks() {
        sinks_.clear();

        // Use pactl to list audio output devices (sinks)
        std::string cmd = "pactl list sinks short";
        std::array<char, 4096> buffer;
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run pactl command\n";
            return;
        }
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);

        // Parse output lines to extract sink information
        size_t pos = 0;
        while ((pos = result.find('\n')) != std::string::npos) {
            std::string line = result.substr(0, pos);
            result.erase(0, pos + 1);

            auto tokens = split(line, '\t');
            if (tokens.size() >= 2) {
                AudioSink sink;
                sink.name = tokens[1];

                // Query human-friendly description for better display
                std::string desc_cmd = "pactl list sinks | grep -A10 'Name: " + sink.name + "' | grep 'Description:' | head -1 | cut -d':' -f2-";
                std::array<char, 512> desc_buffer;
                std::string desc_result;
                FILE* desc_pipe = popen(desc_cmd.c_str(), "r");
                if (desc_pipe) {
                    if (fgets(desc_buffer.data(), desc_buffer.size(), desc_pipe) != nullptr) {
                        desc_result = desc_buffer.data();
                    }
                    pclose(desc_pipe);
                }
                // Trim whitespace from the description
                desc_result.erase(0, desc_result.find_first_not_of(" \t\n\r"));
                desc_result.erase(desc_result.find_last_not_of(" \t\n\r") + 1);

                sink.description = desc_result.empty() ? sink.name : desc_result;
                sink.volume = get_sink_volume(sink.name);
                sink.muted = is_sink_muted(sink.name);
                sinks_.push_back(sink);
            }
        }

        // Use pactl to list audio input devices (sources)
        cmd = "pactl list sources short";
        result.clear();

        pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run pactl command for sources\n";
            return;
        }
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);

        pos = 0;
        while ((pos = result.find('\n')) != std::string::npos) {
            std::string line = result.substr(0, pos);
            result.erase(0, pos + 1);

            auto tokens = split(line, '\t');
            if (tokens.size() >= 2) {
                AudioSink source;
                source.name = tokens[1];

                std::string desc_cmd = "pactl list sources | grep -A10 'Name: " + source.name + "' | grep 'Description:' | head -1 | cut -d':' -f2-";
                std::array<char, 512> desc_buffer;
                std::string desc_result;
                FILE* desc_pipe = popen(desc_cmd.c_str(), "r");
                if (desc_pipe) {
                    if (fgets(desc_buffer.data(), desc_buffer.size(), desc_pipe) != nullptr) {
                        desc_result = desc_buffer.data();
                    }
                    pclose(desc_pipe);
                }
                desc_result.erase(0, desc_result.find_first_not_of(" \t\n\r"));
                desc_result.erase(desc_result.find_last_not_of(" \t\n\r") + 1);

                source.description = desc_result.empty() ? source.name : desc_result;
                source.volume = get_source_volume(source.name);
                source.muted = is_source_muted(source.name);
                sinks_.push_back(source);
            }
        }

        if (update_callback_) {
            update_callback_(sinks_);
        }
    }

    /**
     * @brief Set the volume level for an audio device
     * @param sink_name The name of the device to adjust
     * @param volume The volume level to set (0-100)
     *
     * Sets the volume of the specified audio device. The volume is clamped
     * to the range 0-100. Detects whether the device is a sink or source
     * and uses the appropriate pactl command.
     */
    void set_volume(const std::string& sink_name, int volume) {
        int vol = std::max(0, std::min(100, volume));
        std::string cmd;
        if (sink_name.find("input") != std::string::npos || sink_name.find("source") != std::string::npos) {
            cmd = "pactl set-source-volume " + sink_name + " " + std::to_string(vol) + "%";
        } else {
            cmd = "pactl set-sink-volume " + sink_name + " " + std::to_string(vol) + "%";
        }
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            std::cerr << "Failed to set volume for " << sink_name << std::endl;
        }
        // Note: We don't refresh sinks immediately to avoid widget destruction during slider drag
    }

    /**
     * @brief Toggle the mute state of an audio device
     * @param sink_name The name of the device to toggle
     *
     * Toggles the mute state of the specified audio device. Detects whether
     * the device is a sink or source and uses the appropriate pactl command.
     */
    void toggle_mute(const std::string& sink_name) {
        std::string cmd;
        if (sink_name.find("input") != std::string::npos || sink_name.find("source") != std::string::npos) {
            cmd = "pactl set-source-mute " + sink_name + " toggle";
        } else {
            cmd = "pactl set-sink-mute " + sink_name + " toggle";
        }
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
            std::cerr << "Failed to toggle mute for " << sink_name << std::endl;
        }
        // Note: We don't refresh sinks immediately to avoid widget destruction during toggle
    }

    /**
     * @brief Set the callback for device list updates
     * @param cb The callback function to be called when the device list changes
     */
    void set_update_callback(VolumeManager::SinkUpdateCallback cb) {
        update_callback_ = cb;
    }

private:
    std::vector<AudioSink> sinks_;
    VolumeManager::SinkUpdateCallback update_callback_;

    /**
     * @brief Split a string by a delimiter character
     * @param s The string to split
     * @param delimiter The character to split on
     * @return A vector of substrings
     */
    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : s) {
            if (c == delimiter) {
                tokens.push_back(token);
                token.clear();
            } else {
                token += c;
            }
        }
        tokens.push_back(token);
        return tokens;
    }

    /**
     * @brief Get the current volume level of an audio output device
     * @param sink_name The name of the sink to query
     * @return The volume level as a percentage (0-100)
     */
    int get_sink_volume(const std::string& sink_name) {
        std::string cmd = "pactl get-sink-volume " + sink_name + " | grep -oP '\\d+%' | head -1";
        std::array<char, 128> buffer;
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return 0;
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result = buffer.data();
        }
        pclose(pipe);

        try {
            int percent = std::stoi(result);
            return percent;
        } catch (...) {
            return 0;
        }
    }

    /**
     * @brief Get the current volume level of an audio input device
     * @param source_name The name of the source to query
     * @return The volume level as a percentage (0-100)
     */
    int get_source_volume(const std::string& source_name) {
        std::string cmd = "pactl get-source-volume " + source_name + " | grep -oP '\\d+%' | head -1";
        std::array<char, 128> buffer;
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return 0;
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result = buffer.data();
        }
        pclose(pipe);

        try {
            int percent = std::stoi(result);
            return percent;
        } catch (...) {
            return 0;
        }
    }

    /**
     * @brief Check if an audio input device is muted
     * @param source_name The name of the source to query
     * @return true if the source is muted, false otherwise
     */
    bool is_source_muted(const std::string& source_name) {
        std::string cmd = "pactl get-source-mute " + source_name;
        std::array<char, 128> buffer;
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return false;
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result = buffer.data();
        }
        pclose(pipe);

        return result.find("yes") != std::string::npos;
    }

    /**
     * @brief Check if an audio output device is muted
     * @param sink_name The name of the sink to query
     * @return true if the sink is muted, false otherwise
     */
    bool is_sink_muted(const std::string& sink_name) {
        std::string cmd = "pactl get-sink-mute " + sink_name;
        std::array<char, 128> buffer;
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return false;
        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result = buffer.data();
        }
        pclose(pipe);

        return result.find("yes") != std::string::npos;
    }
};

/**
 * @brief Constructor for VolumeManager
 *
 * Creates the implementation object using the PIMPL idiom.
 */
VolumeManager::VolumeManager() : impl_(std::make_unique<Impl>()) {}

/**
 * @brief Destructor for VolumeManager
 *
 * Default implementation is sufficient since impl_ is a unique_ptr.
 */
VolumeManager::~VolumeManager() = default;

/**
 * @brief Scan for available audio devices
 *
 * Delegates to the implementation class.
 */
void VolumeManager::refresh_sinks() {
    impl_->refresh_sinks();
}

/**
 * @brief Set the volume level for an audio device
 * @param sink_name The name of the device to adjust
 * @param volume The volume level to set (0-100)
 *
 * Delegates to the implementation class.
 */
void VolumeManager::set_volume(const std::string& sink_name, int volume) {
    impl_->set_volume(sink_name, volume);
}

/**
 * @brief Toggle the mute state of an audio device
 * @param sink_name The name of the device to toggle
 *
 * Delegates to the implementation class.
 */
void VolumeManager::toggle_mute(const std::string& sink_name) {
    impl_->toggle_mute(sink_name);
}

/**
 * @brief Set the callback for device list updates
 * @param cb The callback function to be called when the device list changes
 *
 * Delegates to the implementation class.
 */
void VolumeManager::set_update_callback(SinkUpdateCallback cb) {
    impl_->set_update_callback(cb);
}

} // namespace Volume
