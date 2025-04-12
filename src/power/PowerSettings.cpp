/**
 * @file PowerSettings.cpp
 * @brief Implementation of power settings management
 *
 * This file implements the PowerSettings class which provides functionality
 * for loading, saving, and accessing power-related settings such as
 * commands for shutdown, reboot, suspend, etc.
 */

#include "PowerSettings.hpp"
#include <fstream>    // for std::ifstream, std::ofstream
#include <iostream>   // for std::cerr
#include <sys/stat.h> // for mkdir
#include <sys/types.h>

namespace Power
{

    /**
     * @brief Constructor for the power settings manager
     *
     * Initializes default power commands and loads any existing settings
     * from the configuration file.
     */
    PowerSettings::PowerSettings()
    {
        // Determine the configuration file path based on the user's home directory
        const char *home_dir = getenv("HOME");
        if (home_dir)
        {
            config_path_ = std::string(home_dir) + "/.config/ultimate-control/power.conf";
        }
        else
        {
            config_path_ = "/tmp/ultimate-control-power.conf";
        }

        // Initialize default commands for power actions
        commands_["shutdown"] = "systemctl poweroff";
        commands_["reboot"] = "systemctl reboot";
        commands_["suspend"] = "systemctl suspend";
        commands_["hibernate"] = "systemctl hibernate";
        commands_["lock"] = "loginctl lock-session";

        // Initialize default keybinds (empty by default, can be set by user)
        keybinds_["shutdown"] = "";
        keybinds_["reboot"] = "";
        keybinds_["suspend"] = "";
        keybinds_["hibernate"] = "";
        keybinds_["lock"] = "";

        // Load any existing settings from the configuration file
        load();
    }

    /**
     * @brief Destructor for the power settings manager
     *
     * Saves settings to the configuration file before destruction.
     */
    PowerSettings::~PowerSettings()
    {
        save();
    }

    /**
     * @brief Get the command for a specific power action
     * @param action The action name (e.g., "shutdown", "reboot")
     * @return The command string for the specified action
     *
     * Returns the configured command for the specified power action.
     * If no command is configured for the action, returns a default command.
     */
    std::string PowerSettings::get_command(const std::string &action) const
    {
        // Look up the command in the map
        auto it = commands_.find(action);
        if (it != commands_.end())
        {
            return it->second; // Return the configured command if found
        }

        // Return default commands if not found in the map
        if (action == "shutdown")
            return "systemctl poweroff";
        if (action == "reboot")
            return "systemctl reboot";
        if (action == "suspend")
            return "systemctl suspend";
        if (action == "hibernate")
            return "systemctl hibernate";
        if (action == "lock")
            return "loginctl lock-session";

        // Return empty string for unknown actions
        return "";
    }

    std::string PowerSettings::get_keybind(const std::string &action) const
    {
        auto it = keybinds_.find(action);
        if (it != keybinds_.end())
        {
            return it->second;
        }
        return "";
    }

    void PowerSettings::set_keybind(const std::string &action, const std::string &keybind)
    {
        keybinds_[action] = keybind;
    }

    /**
     * @brief Set the command for a specific power action
     * @param action The action name (e.g., "shutdown", "reboot")
     * @param command The command string to execute for the action
     *
     * Sets the command to be executed for the specified power action.
     */
    void PowerSettings::set_command(const std::string &action, const std::string &command)
    {
        commands_[action] = command; // Store the command in the map
    }

    /**
     * @brief Load settings from the configuration file
     *
     * Reads power settings from the configuration file. If the file
     * doesn't exist or can't be read, default settings are used.
     */
    void PowerSettings::load()
    {
        // Try to open the configuration file
        std::ifstream infile(config_path_);
        if (!infile.is_open())
        {
            // File doesn't exist or can't be opened, use defaults
            return;
        }

        // Read the file line by line
        std::string line;
        while (std::getline(infile, line))
        {
            // Skip empty lines and comment lines (starting with #)
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            // Find the delimiter between key and value (format: "key=value")
            size_t delimiter_pos = line.find('=');
            if (delimiter_pos == std::string::npos)
            {
                // Skip lines without a delimiter
                continue;
            }

            // Extract the key and value
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);

            // Store keybinds and commands separately
            if (key.rfind("keybind_", 0) == 0)
            {
                // key starts with "keybind_"
                std::string action = key.substr(8); // after "keybind_"
                keybinds_[action] = value;
            }
            else
            {
                commands_[key] = value;
            }
        }
    }

    /**
     * @brief Save settings to the configuration file
     *
     * Writes the current power settings to the configuration file.
     * Creates the configuration directory if it doesn't exist.
     */
    void PowerSettings::save() const
    {
        // Make sure the configuration directory exists
        ensure_config_dir();

        // Try to open the configuration file for writing
        std::ofstream outfile(config_path_);
        if (!outfile.is_open())
        {
            std::cerr << "Failed to save power settings to " << config_path_ << std::endl;
            return;
        }

        // Write a header comment to the file
        outfile << "# Ultimate Control Power Settings\n";

        // Write each command as a key-value pair
        for (const auto &pair : commands_)
        {
            outfile << pair.first << "=" << pair.second << "\n";
        }

        // Write each keybind as a key-value pair with "keybind_" prefix
        for (const auto &pair : keybinds_)
        {
            outfile << "keybind_" << pair.first << "=" << pair.second << "\n";
        }
    }

    /**
     * @brief Create the configuration directory if it doesn't exist
     *
     * Creates all directories in the path to the configuration file
     * if they don't already exist.
     */
    void PowerSettings::ensure_config_dir() const
    {
        // Extract the directory path from the configuration file path
        std::string dir_path;
        size_t last_slash = config_path_.find_last_of('/');
        if (last_slash != std::string::npos)
        {
            dir_path = config_path_.substr(0, last_slash); // Get everything before the last slash
        }

        // Create the directory if it's not empty
        if (!dir_path.empty())
        {
            // Create directory with parent directories using mkdir -p
            std::string cmd = "mkdir -p " + dir_path;
            std::system(cmd.c_str());
        }
    }

} // namespace Power