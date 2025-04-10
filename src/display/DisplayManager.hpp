/**
 * @file DisplayManager.hpp
 * @brief Display brightness management for Ultimate Control
 *
 * This file defines the DisplayManager class which provides functionality
 * for getting and setting display brightness using the brightnessctl utility.
 */

#ifndef ULTIMATE_CONTROL_DISPLAY_MANAGER_HPP
#define ULTIMATE_CONTROL_DISPLAY_MANAGER_HPP

#include <functional>

/**
 * @namespace Display
 * @brief Contains display management functionality
 */
namespace Display {

/**
 * @class DisplayManager
 * @brief Manages display brightness
 *
 * Provides functionality for getting and setting display brightness
 * using the brightnessctl utility. Brightness values are normalized
 * to a 0-100 scale representing percentage of maximum brightness.
 */
class DisplayManager {
public:
    /**
     * @typedef BrightnessCallback
     * @brief Function type for brightness update notifications
     *
     * Callback function type used to notify when brightness changes.
     * The parameter is the new brightness value (0-100).
     */
    using BrightnessCallback = std::function<void(int)>; // 0-100

    /**
     * @brief Constructor
     *
     * Initializes the display manager and gets the current brightness.
     */
    DisplayManager();

    /**
     * @brief Destructor
     */
    ~DisplayManager();

    /**
     * @brief Get the current display brightness
     * @return The current brightness level (0-100)
     *
     * Retrieves the current brightness level as a percentage of maximum brightness.
     */
    int get_brightness() const;

    /**
     * @brief Set the display brightness
     * @param value The brightness level to set (0-100)
     *
     * Sets the display brightness to the specified percentage of maximum brightness.
     * Values outside the 0-100 range will be clamped.
     */
    void set_brightness(int value);

    /**
     * @brief Set the callback for brightness updates
     * @param cb The callback function to call when brightness changes
     *
     * Sets the callback function that will be called when brightness changes.
     */
    void set_update_callback(BrightnessCallback cb);

private:
    /**
     * @brief Notify listeners of brightness changes
     *
     * Calls the registered callback function if one is set.
     */
    void notify();

    int brightness_;                ///< Current brightness level (0-100)
    BrightnessCallback callback_;   ///< Callback function for brightness changes
};

} // namespace Display

#endif // ULTIMATE_CONTROL_DISPLAY_MANAGER_HPP
