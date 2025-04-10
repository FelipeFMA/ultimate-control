/**
 * @file DisplayTab.hpp
 * @brief Display settings tab for Ultimate Control
 *
 * This file defines the DisplayTab class which provides a user interface
 * for adjusting screen brightness and color temperature.
 */

#ifndef ULTIMATE_CONTROL_DISPLAY_TAB_HPP
#define ULTIMATE_CONTROL_DISPLAY_TAB_HPP

#include <gtkmm.h>
#include "DisplayManager.hpp"
#include <memory>

/**
 * @namespace Display
 * @brief Contains display management functionality
 */
namespace Display {

/**
 * @class DisplayTab
 * @brief Tab for display settings
 *
 * Provides a user interface for adjusting screen brightness and
 * color temperature (blue light filter).
 */
class DisplayTab : public Gtk::Box {
public:
    /**
     * @brief Constructor
     *
     * Initializes the display manager and creates the UI components.
     */
    DisplayTab();

    /**
     * @brief Virtual destructor
     */
    virtual ~DisplayTab();

private:
    /**
     * @brief Handler for brightness changes from the display manager
     * @param value The new brightness value (0-100)
     *
     * Updates the UI when the brightness is changed externally.
     */
    void on_brightness_changed(int value);

    /**
     * @brief Handler for brightness slider changes
     *
     * Updates the brightness when the slider is moved by the user.
     */
    void on_slider_changed();

    /**
     * @brief Handler for color temperature slider changes
     *
     * Updates the color temperature when the slider is moved by the user.
     */
    void on_bluelight_changed();

    /**
     * @brief Update the brightness icon and label
     * @param value The brightness value (0-100)
     *
     * Updates the brightness icon and value label based on the current brightness.
     */
    void update_brightness_icon(int value);

    /**
     * @brief Update the color temperature icon and label
     * @param value The color temperature value (1000-6500K)
     *
     * Updates the color temperature icon and value label based on the current temperature.
     */
    void update_bluelight_icon(int value);

    std::shared_ptr<DisplayManager> manager_;  ///< Display manager for brightness control

    // Main containers
    Gtk::ScrolledWindow scrolled_window_;  ///< Scrollable container for the tab
    Gtk::Box main_box_;                    ///< Main vertical box for all sections

    // Brightness section
    Gtk::Frame brightness_frame_;         ///< Frame around the brightness section
    Gtk::Box brightness_box_;             ///< Container for brightness components
    Gtk::Box brightness_header_box_;      ///< Container for section header
    Gtk::Image brightness_icon_;          ///< Icon for the brightness section
    Gtk::Label brightness_label_;         ///< Label for the brightness section
    Gtk::Label brightness_value_label_;   ///< Label showing current brightness percentage
    Gtk::Scale brightness_scale_;         ///< Slider for adjusting brightness

    // Color temperature section
    Gtk::Frame bluelight_frame_;          ///< Frame around the color temperature section
    Gtk::Box bluelight_box_;              ///< Container for color temperature components
    Gtk::Box bluelight_header_box_;       ///< Container for section header
    Gtk::Image bluelight_icon_;           ///< Icon for the color temperature section
    Gtk::Label bluelight_label_;          ///< Label for the color temperature section
    Gtk::Label bluelight_value_label_;    ///< Label showing current color temperature
    Gtk::Scale bluelight_scale_;          ///< Slider for adjusting color temperature

    sigc::connection brightness_signal_handler_id_;  ///< Connection for brightness slider signal
};

} // namespace Display

#endif // ULTIMATE_CONTROL_DISPLAY_TAB_HPP
