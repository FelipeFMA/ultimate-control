/**
 * @file TabSettings.cpp
 * @brief Implementation of tab configuration management
 *
 * This file implements the TabSettings class which provides functionality
 * for managing tab order, visibility, and other tab-related settings.
 * Settings are persisted to a configuration file.
 */

#include "TabSettings.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

namespace Settings {

/**
 * @brief Constructor for the tab settings manager
 *
 * Initializes default tab settings and loads any existing settings
 * from the configuration file.
 */
TabSettings::TabSettings() {
    // Determine the configuration file path based on the user's home directory
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        config_path_ = std::string(home_dir) + "/.config/ultimate-control/settings.json";
    } else {
        config_path_ = "/tmp/ultimate-control-settings.json";
    }

    // Configuration path is now set

    // Initialize default tab information (ID, name, icon, enabled state)
    tab_info_["volume"] = {"volume", "Volume", "audio-volume-high-symbolic", true};
    tab_info_["wifi"] = {"wifi", "WiFi", "network-wireless-symbolic", true};
    tab_info_["display"] = {"display", "Display", "video-display-symbolic", true};
    tab_info_["power"] = {"power", "Power", "system-shutdown-symbolic", true};
    tab_info_["settings"] = {"settings", "Settings", "preferences-system-symbolic", true};

    // Set default tab display order
    tab_order_ = {"volume", "wifi", "display", "power", "settings"};

    // Set all tabs to be enabled by default
    for (const auto& tab : tab_info_) {
        tab_enabled_[tab.first] = true;
    }

    // Load any existing settings from the configuration file
    load();
}

/**
 * @brief Destructor for the tab settings manager
 *
 * Saves settings to the configuration file before destruction.
 */
TabSettings::~TabSettings() {
    save();
}

/**
 * @brief Create the configuration directory if it doesn't exist
 *
 * Creates all directories in the path to the configuration file
 * if they don't already exist.
 */
void TabSettings::ensure_config_dir() const {
    std::string dir_path = config_path_.substr(0, config_path_.find_last_of('/'));

    // Parse and create each directory in the path recursively
    std::string current_path;
    std::istringstream path_stream(dir_path);
    std::string path_part;

    while (std::getline(path_stream, path_part, '/')) {
        if (path_part.empty()) {
            current_path = "/";
            continue;
        }

        current_path += path_part + "/";

        struct stat info;
        if (stat(current_path.c_str(), &info) != 0) {
            // Directory doesn't exist, create it
            if (mkdir(current_path.c_str(), 0755) != 0) {
                std::cerr << "Failed to create directory: " << current_path << std::endl;
                return;
            }
        }
    }
}

/**
 * @brief Load settings from the configuration file
 *
 * Reads tab settings from the configuration file. If the file
 * doesn't exist or can't be read, default settings are used.
 */
void TabSettings::load() {
    std::ifstream infile(config_path_);
    if (!infile.is_open()) {
        // File doesn't exist or can't be opened, use defaults
        return;
    }
    std::string line;
    while (std::getline(infile, line)) {
        // Skip empty lines and comment lines (starting with #)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t delimiter_pos = line.find('=');
        if (delimiter_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, delimiter_pos);
        std::string value = line.substr(delimiter_pos + 1);

        if (key == "tab_order") {
            // Parse comma-separated list of tab IDs for the display order
            tab_order_.clear();
            std::istringstream order_stream(value);
            std::string tab_id;

            while (std::getline(order_stream, tab_id, ',')) {
                if (!tab_id.empty()) {
                    tab_order_.push_back(tab_id);
                }
            }
        } else if (key.substr(0, 4) == "tab_") {
            // Parse individual tab enabled/disabled state
            std::string tab_id = key.substr(4);
            bool enabled = (value == "1" || value == "true");
            tab_enabled_[tab_id] = enabled;
        }
    }

    // Ensure all known tabs are included in the order list (append any missing ones)
    for (const auto& tab : tab_info_) {
        if (std::find(tab_order_.begin(), tab_order_.end(), tab.first) == tab_order_.end()) {
            tab_order_.push_back(tab.first);
        }
    }
}

/**
 * @brief Save settings to the configuration file
 *
 * Writes the current tab settings to the configuration file.
 * Creates the configuration directory if it doesn't exist.
 */
void TabSettings::save() const {
    ensure_config_dir();

    std::ofstream outfile(config_path_);
    if (!outfile.is_open()) {
        std::cerr << "Failed to save tab settings to " << config_path_ << std::endl;
        return;
    }

    // Write a header comment to the file
    outfile << "# Ultimate Control Tab Settings\n";

    // Write the tab order as a comma-separated list
    outfile << "tab_order=";
    for (size_t i = 0; i < tab_order_.size(); ++i) {
        outfile << tab_order_[i];
        if (i < tab_order_.size() - 1) {
            outfile << ",";
        }
    }
    outfile << "\n";

    // Write each tab's enabled/disabled state
    for (const auto& tab : tab_enabled_) {
        outfile << "tab_" << tab.first << "=" << (tab.second ? "1" : "0") << "\n";
    }
}

/**
 * @brief Get the current tab order
 * @return Vector of tab IDs in display order
 */
std::vector<std::string> TabSettings::get_tab_order() const {
    return tab_order_;
}

/**
 * @brief Set the tab order
 * @param order Vector of tab IDs in the desired order
 */
void TabSettings::set_tab_order(const std::vector<std::string>& order) {
    tab_order_ = order;
}

/**
 * @brief Check if a tab is enabled
 * @param tab_id The ID of the tab to check
 * @return true if the tab is enabled, false otherwise
 */
bool TabSettings::is_tab_enabled(const std::string& tab_id) const {
    auto it = tab_enabled_.find(tab_id);
    if (it != tab_enabled_.end()) {
        return it->second;
    }
    return true; // Default to enabled if not found in the map
}

/**
 * @brief Enable or disable a tab
 * @param tab_id The ID of the tab to modify
 * @param enabled Whether the tab should be enabled
 */
void TabSettings::set_tab_enabled(const std::string& tab_id, bool enabled) {
    tab_enabled_[tab_id] = enabled;
}

/**
 * @brief Get information about all tabs
 * @return Vector of TabInfo structures for all tabs
 */
std::vector<TabInfo> TabSettings::get_all_tabs() const {
    std::vector<TabInfo> result;

    for (const auto& tab_id : tab_order_) {
        auto it = tab_info_.find(tab_id);
        if (it != tab_info_.end()) {
            TabInfo info = it->second;
            info.enabled = is_tab_enabled(tab_id);
            result.push_back(info);
        }
    }

    return result;
}

/**
 * @brief Move a tab up in the order
 * @param tab_id The ID of the tab to move
 * @return true if the tab was moved, false if it couldn't be moved
 */
bool TabSettings::move_tab_up(const std::string& tab_id) {
    auto it = std::find(tab_order_.begin(), tab_order_.end(), tab_id);
    if (it == tab_order_.end() || it == tab_order_.begin()) {
        return false;
    }

    std::iter_swap(it, it - 1);
    return true;
}

/**
 * @brief Move a tab down in the order
 * @param tab_id The ID of the tab to move
 * @return true if the tab was moved, false if it couldn't be moved
 */
bool TabSettings::move_tab_down(const std::string& tab_id) {
    auto it = std::find(tab_order_.begin(), tab_order_.end(), tab_id);
    if (it == tab_order_.end() || it == tab_order_.end() - 1) {
        return false;
    }

    std::iter_swap(it, it + 1);
    return true;
}

} // namespace Settings
