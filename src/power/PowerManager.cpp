/**
 * @file PowerManager.cpp
 * @brief Implementation of power management functionality
 *
 * This file implements the PowerManager class which provides an interface for
 * system power operations like shutdown, reboot, suspend, and hibernate,
 * as well as power profile management.
 */

#include "PowerManager.hpp"
#include <cstdlib>   // for std::system
#include <cstdio>    // for popen, pclose
#include <array>     // for std::array
#include <string>    // for std::string
#include <vector>    // for std::vector
#include <iostream>  // for std::cout, std::cerr

namespace Power {

/**
 * @brief Constructor for the power manager
 *
 * Initializes the power manager and loads power settings.
 */
PowerManager::PowerManager()
: settings_(std::make_shared<PowerSettings>()) // Initialize power settings
{
}

/**
 * @brief Destructor for the power manager
 */
PowerManager::~PowerManager() = default;

/**
 * @brief Shut down the system
 *
 * Executes the configured shutdown command and notifies listeners.
 */
void PowerManager::shutdown() {
    std::system(settings_->get_command("shutdown").c_str());
    notify();
}

/**
 * @brief Reboot the system
 *
 * Executes the configured reboot command and notifies listeners.
 */
void PowerManager::reboot() {
    std::system(settings_->get_command("reboot").c_str());
    notify();
}

/**
 * @brief Suspend the system
 *
 * Executes the configured suspend command and notifies listeners.
 */
void PowerManager::suspend() {
    std::system(settings_->get_command("suspend").c_str());
    notify();
}

/**
 * @brief Hibernate the system
 *
 * Executes the configured hibernate command and notifies listeners.
 */
void PowerManager::hibernate() {
    std::system(settings_->get_command("hibernate").c_str());
    notify();
}

/**
 * @brief List available power profiles
 * @return Vector of available power profile names
 *
 * Retrieves the list of available power profiles from the system
 * by parsing the output of the powerprofilesctl command.
 */
std::vector<std::string> PowerManager::list_power_profiles() {
    std::vector<std::string> profiles;           // List to store profile names
    std::string cmd = "powerprofilesctl list";   // Command to list profiles
    std::array<char, 2048> buffer;               // Buffer for command output
    std::string result;                          // Complete command output

    // Execute the command and read its output
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return profiles;  // Return empty list if command fails

    // Read all output into the result string
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    // Parse the output line by line
    size_t pos = 0;
    while ((pos = result.find('\n')) != std::string::npos) {
        std::string line = result.substr(0, pos);  // Extract one line
        result.erase(0, pos + 1);                  // Remove processed line

        // Remove leading/trailing whitespace from the line
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines
        if (line.empty()) continue;

        // If line starts with '*' (current profile), remove the asterisk
        if (line[0] == '*') {
            line = line.substr(1);  // Remove the asterisk
            line.erase(0, line.find_first_not_of(" \t"));  // Remove whitespace after asterisk
        }

        // If line ends with ':', it's a profile name (format: "profile_name:")
        if (!line.empty() && line.back() == ':') {
            line.pop_back();  // Remove the colon

            // Clean up the profile name
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            // Add non-empty profile names to the list
            if (!line.empty()) {
                profiles.push_back(line);
            }
        }
    }
    return profiles;
}

/**
 * @brief Set the current power profile
 * @param profile The name of the profile to set
 *
 * Sets the system power profile to the specified profile
 * using the powerprofilesctl command.
 */
void PowerManager::set_power_profile(const std::string& profile) {
    // Construct and execute the command to set the profile
    std::string cmd = "powerprofilesctl set " + profile;
    std::system(cmd.c_str());

    // Notify listeners that the profile has changed
    notify();
}

/**
 * @brief Get the current power profile
 * @return The name of the current power profile
 *
 * Retrieves the name of the currently active power profile
 * by executing the powerprofilesctl get command.
 */
std::string PowerManager::get_current_power_profile() {
    std::string cmd = "powerprofilesctl get";  // Command to get current profile
    std::array<char, 128> buffer;               // Buffer for command output
    std::string result;                         // Result string

    // Execute the command and read its output
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";  // Return empty string if command fails

    // Read the first line of output (should contain the profile name)
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    // Remove trailing newline from the result
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;  // Return the profile name
}

/**
 * @brief Set the update callback function
 * @param cb The callback function to call when power operations are performed
 *
 * Sets the callback function that will be called when power operations are performed.
 */
void PowerManager::set_update_callback(Callback cb) {
    callback_ = cb;
}

/**
 * @brief Notify listeners of power operations
 *
 * Calls the registered callback function if one is set.
 */
void PowerManager::notify() {
    if (callback_) {
        callback_();  // Call the callback function if it exists
    }
}

/**
 * @brief Get the power settings object
 * @return Shared pointer to the PowerSettings object
 *
 * Returns the PowerSettings object that contains the configured power commands.
 */
std::shared_ptr<PowerSettings> PowerManager::get_settings() const {
    return settings_;
}

} // namespace Power
