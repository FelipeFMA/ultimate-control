/**
 * @file PowerSettings.hpp
 * @brief Power settings management for Ultimate Control
 *
 * This file defines the PowerSettings class which provides functionality
 * for loading, saving, and accessing power-related settings such as
 * commands for shutdown, reboot, suspend, etc.
 */

#pragma once

#include <string>
#include <map>

/**
 * @namespace Power
 * @brief Contains power management functionality
 */
namespace Power
{

    /**
     * @class PowerSettings
     * @brief Manages power-related settings
     *
     * Provides functionality for loading, saving, and accessing power-related
     * settings such as commands for shutdown, reboot, suspend, etc.
     * Settings are persisted to a configuration file.
     */
    class PowerSettings
    {
    public:
        /**
         * @brief Constructor
         *
         * Initializes default power commands and loads any existing settings
         * from the configuration file.
         */
        PowerSettings();

        /**
         * @brief Destructor
         *
         * Saves settings to the configuration file before destruction.
         */
        ~PowerSettings();

        /**
         * @brief Get the command for a specific power action
         * @param action The action name (e.g., "shutdown", "reboot")
         * @return The command string for the specified action
         *
         * Returns the configured command for the specified power action.
         * If no command is configured for the action, returns a default command.
         */
        std::string get_command(const std::string &action) const;

        /**
         * @brief Set the command for a specific power action
         * @param action The action name (e.g., "shutdown", "reboot")
         * @param command The command string to execute for the action
         *
         * Sets the command to be executed for the specified power action.
         */
        void set_command(const std::string &action, const std::string &command);

        /**
         * @brief Get the keybind for a specific power action
         * @param action The action name (e.g., "shutdown", "reboot")
         * @return The keybind string for the specified action (e.g., "Ctrl+Alt+S")
         *
         * Returns the configured keybind for the specified power action.
         * If no keybind is configured for the action, returns an empty string.
         */
        std::string get_keybind(const std::string &action) const;

        /**
         * @brief Set the keybind for a specific power action
         * @param action The action name (e.g., "shutdown", "reboot")
         * @param keybind The keybind string to use for the action (e.g., "Ctrl+Alt+S")
         *
         * Sets the keybind to be used for the specified power action.
         */
        void set_keybind(const std::string &action, const std::string &keybind);

        /**
         * @brief Load settings from the configuration file
         *
         * Reads power settings from the configuration file. If the file
         * doesn't exist or can't be read, default settings are used.
         */
        void load();

        /**
         * @brief Save settings to the configuration file
         *
         * Writes the current power settings to the configuration file.
         * Creates the configuration directory if it doesn't exist.
         */
        void save() const;

    private:
        /**
         * @brief Create the configuration directory if it doesn't exist
         *
         * Creates all directories in the path to the configuration file
         * if they don't already exist.
         */
        void ensure_config_dir() const;

        std::string config_path_;                     ///< Path to the configuration file
        std::map<std::string, std::string> commands_; ///< Map of action names to command strings
        std::map<std::string, std::string> keybinds_; ///< Map of action names to keybind strings
    };

} // namespace Power
