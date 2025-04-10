/**
 * @file VolumeSettings.hpp
 * @brief Settings management for the volume module
 *
 * This file defines the VolumeSettings class which provides functionality
 * for loading, saving, and accessing volume-related settings.
 */

#pragma once

#include <string>
#include <map>

namespace Volume {

/**
 * @class VolumeSettings
 * @brief Manages volume-related settings
 *
 * Provides functionality for loading, saving, and accessing
 * volume-related settings such as default volume levels.
 * Settings are persisted to a configuration file.
 */
class VolumeSettings {
public:
    /**
     * @brief Constructor
     *
     * Initializes the settings manager and loads existing settings.
     */
    VolumeSettings();

    /**
     * @brief Destructor
     *
     * Saves settings before destruction.
     */
    ~VolumeSettings();

    /**
     * @brief Load settings from the configuration file
     *
     * Reads settings from the configuration file and populates
     * the settings map. If the file doesn't exist, uses defaults.
     */
    void load();

    /**
     * @brief Save settings to the configuration file
     *
     * Writes the current settings to the configuration file.
     */
    void save() const;

    /**
     * @brief Get the default volume level
     * @return The default volume level (0-100)
     *
     * Returns the configured default volume level or 50 if not set.
     */
    int get_default_volume() const;

    /**
     * @brief Set the default volume level
     * @param volume The volume level to set as default (0-100)
     *
     * Sets the default volume level used for new devices.
     */
    void set_default_volume(int volume);

private:
    std::map<std::string, int> settings_;  ///< Map of setting names to values
    std::string config_path_;              ///< Path to the configuration file
};

} // namespace Volume

