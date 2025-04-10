/**
 * @file VolumeManager.hpp
 * @brief Audio volume management functionality for Ultimate Control
 *
 * This file defines the VolumeManager class which provides an interface
 * for controlling audio devices (sinks and sources) using PulseAudio.
 * It uses the PIMPL idiom to hide implementation details.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

/**
 * @namespace Volume
 * @brief Contains audio volume management functionality
 */
namespace Volume {

/**
 * @struct AudioSink
 * @brief Represents an audio device (sink or source)
 */
struct AudioSink {
    std::string name;        ///< Device identifier used by PulseAudio
    std::string description;  ///< Human-readable device description
    int volume;              ///< Current volume level (0-100)
    bool muted;              ///< Whether the device is currently muted
};

/**
 * @class VolumeManager
 * @brief Manages audio device volume and mute state
 *
 * Provides an interface for listing audio devices, controlling their volume,
 * and toggling their mute state. Uses PulseAudio via its command-line
 * interface (pactl) for all operations.
 */
class VolumeManager {
public:
    using SinkList = std::vector<AudioSink>;  ///< Type alias for a list of audio devices
    using SinkUpdateCallback = std::function<void(const SinkList&)>;  ///< Callback type for device list updates

    /**
     * @brief Constructor
     *
     * Initializes the volume manager.
     */
    VolumeManager();

    /**
     * @brief Destructor
     */
    ~VolumeManager();

    /**
     * @brief Scan for available audio devices
     *
     * Scans for available audio sinks and sources. When the scan is complete,
     * the update callback (if set) will be called with the list of devices.
     */
    void refresh_sinks();

    /**
     * @brief Set the volume level for an audio device
     * @param sink_name The name of the device to adjust
     * @param volume The volume level to set (0-100)
     *
     * Sets the volume of the specified audio device. The volume is clamped
     * to the range 0-100.
     */
    void set_volume(const std::string& sink_name, int volume);

    /**
     * @brief Toggle the mute state of an audio device
     * @param sink_name The name of the device to toggle
     *
     * Toggles the mute state of the specified audio device.
     */
    void toggle_mute(const std::string& sink_name);

    /**
     * @brief Set the callback for device list updates
     * @param cb The callback function to be called when the device list changes
     *
     * The callback will be called after each successful device scan.
     */
    void set_update_callback(SinkUpdateCallback cb);

private:
    class Impl;  ///< Forward declaration of implementation class
    std::unique_ptr<Impl> impl_;  ///< Pointer to implementation (PIMPL idiom)
};

} // namespace Volume

