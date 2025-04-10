/**
 * @file DisplayManager.cpp
 * @brief Implementation of display brightness management
 *
 * This file implements the DisplayManager class which provides functionality
 * for getting and setting display brightness using the brightnessctl utility.
 */

#include "DisplayManager.hpp"
#include <cstdlib>   // for std::system
#include <cstdio>    // for popen, pclose
#include <array>     // for std::array
#include <iostream>  // for std::cout, std::cerr
#include <algorithm> // for std::clamp

namespace Display {

/**
 * @brief Constructor for the display manager
 *
 * Initializes the display manager and gets the current brightness.
 */
DisplayManager::DisplayManager() {
    brightness_ = get_brightness();  // Initialize with current brightness
}

/**
 * @brief Destructor for the display manager
 */
DisplayManager::~DisplayManager() = default;

/**
 * @brief Get the current display brightness
 * @return The current brightness level (0-100)
 *
 * Retrieves the current brightness level as a percentage of maximum brightness
 * by executing the brightnessctl utility and parsing its output.
 */
int DisplayManager::get_brightness() const {
    // Command to get current brightness value
    std::string cmd = "brightnessctl get";
    std::array<char, 128> buffer;  // Buffer for command output
    std::string result;            // Result string

    // Execute the command and read its output
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return 0;  // Return 0 if command fails

    // Read the first line of output (should contain the brightness value)
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    // Parse the current brightness value
    int current = 0;
    try {
        current = std::stoi(result);  // Convert string to integer
    } catch (...) {
        return 0;  // Return 0 if parsing fails
    }

    // Get the maximum brightness value for percentage calculation
    cmd = "brightnessctl max";
    result.clear();
    pipe = popen(cmd.c_str(), "r");
    if (!pipe) return 0;  // Return 0 if command fails

    // Read the first line of output (should contain the max brightness value)
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    // Parse the maximum brightness value
    int max = 1;  // Default to 1 to avoid division by zero
    try {
        max = std::stoi(result);  // Convert string to integer
    } catch (...) {
        max = 1;  // Use default if parsing fails
    }

    // Calculate brightness as a percentage of maximum
    int percent = static_cast<int>(100.0 * current / max);
    return std::clamp(percent, 0, 100);  // Ensure value is between 0 and 100
}

/**
 * @brief Set the display brightness
 * @param value The brightness level to set (0-100)
 *
 * Sets the display brightness to the specified percentage of maximum brightness
 * by executing the brightnessctl utility. Values outside the 0-100 range will be clamped.
 */
void DisplayManager::set_brightness(int value) {
    // Ensure value is between 0 and 100
    int clamped = std::clamp(value, 0, 100);

    // Construct and execute the command to set brightness
    std::string cmd = "brightnessctl set " + std::to_string(clamped) + "%";
    std::system(cmd.c_str());

    // Update stored brightness and notify listeners
    brightness_ = clamped;
    notify();
}

/**
 * @brief Set the callback for brightness updates
 * @param cb The callback function to call when brightness changes
 *
 * Sets the callback function that will be called when brightness changes.
 * Immediately calls the callback with the current brightness value.
 */
void DisplayManager::set_update_callback(BrightnessCallback cb) {
    callback_ = cb;  // Store the callback function
    notify();        // Notify with current brightness
}

/**
 * @brief Notify listeners of brightness changes
 *
 * Calls the registered callback function if one is set.
 */
void DisplayManager::notify() {
    if (callback_) {
        callback_(brightness_);  // Call the callback with current brightness
    }
}

} // namespace Display
