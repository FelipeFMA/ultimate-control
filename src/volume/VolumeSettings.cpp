/**
 * @file VolumeSettings.cpp
 * @brief Implementation of the volume settings management
 *
 * This file implements the VolumeSettings class which provides functionality
 * for loading, saving, and accessing volume-related settings.
 */

#include "VolumeSettings.hpp"
#include <fstream>
#include <iostream>

namespace Volume {

/**
 * @brief Constructor for the volume settings manager
 *
 * Initializes the settings manager with the configuration file path
 * and loads existing settings.
 */
VolumeSettings::VolumeSettings() {
    // Set the path to the configuration file
    config_path_ = "/home/felipe/.config/ultimate-control/volume.conf";
    load();
}

/**
 * @brief Destructor for the volume settings manager
 *
 * Saves settings before destruction to ensure they persist.
 */
VolumeSettings::~VolumeSettings() {
    save();
}

/**
 * @brief Load settings from the configuration file
 *
 * Reads settings from the configuration file and populates
 * the settings map. If the file doesn't exist, uses defaults.
 */
void VolumeSettings::load() {
    // Clear any existing settings
    settings_.clear();

    // Try to open the configuration file
    std::ifstream infile(config_path_);
    if (!infile.is_open()) {
        // File doesn't exist or can't be opened, use defaults
        return;
    }

    // Read key-value pairs from the file
    std::string key;
    int value;
    while (infile >> key >> value) {
        settings_[key] = value;
    }
}

/**
 * @brief Save settings to the configuration file
 *
 * Writes the current settings to the configuration file.
 * Logs an error if the file can't be written.
 */
void VolumeSettings::save() const {
    // Try to open the configuration file for writing
    std::ofstream outfile(config_path_);
    if (!outfile.is_open()) {
        std::cerr << "Failed to save volume settings\n";
        return;
    }

    // Write each setting as a key-value pair
    for (const auto& pair : settings_) {
        outfile << pair.first << " " << pair.second << "\n";
    }
}

/**
 * @brief Get the default volume level
 * @return The default volume level (0-100)
 *
 * Returns the configured default volume level or 50 if not set.
 */
int VolumeSettings::get_default_volume() const {
    // Look up the default volume setting
    auto it = settings_.find("default_volume");
    if (it != settings_.end()) {
        return it->second;
    }
    return 50; // Default fallback value if not configured
}

/**
 * @brief Set the default volume level
 * @param volume The volume level to set as default (0-100)
 *
 * Sets the default volume level used for new devices.
 */
void VolumeSettings::set_default_volume(int volume) {
    // Store the default volume setting
    settings_["default_volume"] = volume;
}

} // namespace Volume
