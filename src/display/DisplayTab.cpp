/**
 * @file DisplayTab.cpp
 * @brief Implementation of the display settings tab
 *
 * This file implements the DisplayTab class which provides a user interface
 * for adjusting screen brightness and color temperature.
 */

#include "DisplayTab.hpp"
#include <iostream>  // for std::cout
#include <iomanip>   // for std::setprecision
#include <sstream>   // for std::stringstream

namespace Display {

/**
 * @brief Constructor for the display tab
 *
 * Initializes the display manager and creates the UI components.
 */
DisplayTab::DisplayTab()
: manager_(std::make_shared<DisplayManager>()),                // Initialize display manager
  main_box_(Gtk::ORIENTATION_VERTICAL, 15),                    // Main container with 15px spacing
  brightness_box_(Gtk::ORIENTATION_VERTICAL, 10),              // Brightness section with 10px spacing
  brightness_header_box_(Gtk::ORIENTATION_HORIZONTAL, 10),     // Brightness header with 10px spacing
  brightness_scale_(Gtk::ORIENTATION_HORIZONTAL),              // Horizontal brightness slider
  bluelight_box_(Gtk::ORIENTATION_VERTICAL, 10),               // Color temperature section with 10px spacing
  bluelight_header_box_(Gtk::ORIENTATION_HORIZONTAL, 10),      // Color temperature header with 10px spacing
  bluelight_scale_(Gtk::ORIENTATION_HORIZONTAL)                // Horizontal color temperature slider
{
    // Set up the main container orientation
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    // Add a scrolled window to contain all sections
    scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the main box inside the scrolled window with margins
    main_box_.set_margin_start(20);
    main_box_.set_margin_end(20);
    main_box_.set_margin_top(20);
    main_box_.set_margin_bottom(20);
    scrolled_window_.add(main_box_);

    // Configure the frame and container for the brightness section
    brightness_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    brightness_box_.set_margin_start(15);
    brightness_box_.set_margin_end(15);
    brightness_box_.set_margin_top(15);
    brightness_box_.set_margin_bottom(15);

    // Configure the header for the brightness section
    brightness_icon_.set_from_icon_name("display-brightness-symbolic", Gtk::ICON_SIZE_DIALOG);
    brightness_label_.set_markup("<span size='large' weight='bold'>Screen Brightness</span>");
    brightness_label_.set_halign(Gtk::ALIGN_START);
    brightness_label_.set_valign(Gtk::ALIGN_CENTER);
    brightness_value_label_.set_text("50%");
    brightness_value_label_.set_halign(Gtk::ALIGN_END);

    brightness_header_box_.pack_start(brightness_icon_, Gtk::PACK_SHRINK);
    brightness_header_box_.pack_start(brightness_label_, Gtk::PACK_EXPAND_WIDGET);
    brightness_header_box_.pack_end(brightness_value_label_, Gtk::PACK_SHRINK);

    // Configure the slider for adjusting brightness
    brightness_scale_.set_range(0, 100);
    brightness_scale_.set_increments(1, 10);
    brightness_scale_.set_size_request(300, -1);
    brightness_scale_.set_draw_value(false);
    brightness_scale_.add_mark(0, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(25, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(50, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(75, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(100, Gtk::POS_BOTTOM, "");

    // Assemble the brightness section components
    brightness_box_.pack_start(brightness_header_box_, Gtk::PACK_SHRINK);
    brightness_box_.pack_start(brightness_scale_, Gtk::PACK_SHRINK);

    // Add the assembled brightness box to the frame
    brightness_frame_.add(brightness_box_);

    // Configure the frame and container for the color temperature section
    bluelight_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    bluelight_box_.set_margin_start(15);
    bluelight_box_.set_margin_end(15);
    bluelight_box_.set_margin_top(15);
    bluelight_box_.set_margin_bottom(15);

    // Configure the header for the color temperature section
    bluelight_icon_.set_from_icon_name("night-light-symbolic", Gtk::ICON_SIZE_DIALOG);
    bluelight_label_.set_markup("<span size='large' weight='bold'>Color Temperature</span>");
    bluelight_label_.set_halign(Gtk::ALIGN_START);
    bluelight_label_.set_valign(Gtk::ALIGN_CENTER);
    bluelight_value_label_.set_text("6500K");
    bluelight_value_label_.set_halign(Gtk::ALIGN_END);

    bluelight_header_box_.pack_start(bluelight_icon_, Gtk::PACK_SHRINK);
    bluelight_header_box_.pack_start(bluelight_label_, Gtk::PACK_EXPAND_WIDGET);
    bluelight_header_box_.pack_end(bluelight_value_label_, Gtk::PACK_SHRINK);

    // Configure the slider for adjusting color temperature
    bluelight_scale_.set_range(1000, 6500);
    bluelight_scale_.set_increments(100, 500);
    bluelight_scale_.set_size_request(300, -1);
    bluelight_scale_.set_draw_value(false);
    bluelight_scale_.add_mark(1000, Gtk::POS_BOTTOM, "Warm");
    bluelight_scale_.add_mark(3750, Gtk::POS_BOTTOM, "");
    bluelight_scale_.add_mark(6500, Gtk::POS_BOTTOM, "Cool");
    bluelight_scale_.set_value(6500);

    // Assemble the color temperature section components
    bluelight_box_.pack_start(bluelight_header_box_, Gtk::PACK_SHRINK);
    bluelight_box_.pack_start(bluelight_scale_, Gtk::PACK_SHRINK);

    // Add the assembled color temperature box to the frame
    bluelight_frame_.add(bluelight_box_);

    // Add both section frames to the main box
    main_box_.pack_start(brightness_frame_, Gtk::PACK_SHRINK);
    main_box_.pack_start(bluelight_frame_, Gtk::PACK_SHRINK);

    // Connect signal handlers for the sliders
    brightness_signal_handler_id_ = brightness_scale_.signal_value_changed().connect(sigc::mem_fun(*this, &DisplayTab::on_slider_changed));
    bluelight_scale_.signal_value_changed().connect(sigc::mem_fun(*this, &DisplayTab::on_bluelight_changed));

    // Set up the callback for brightness updates from the display manager
    manager_->set_update_callback([this](int value) {
        on_brightness_changed(value);
    });

    // Update initial icon and label values
    update_brightness_icon(manager_->get_brightness());
    update_bluelight_icon(static_cast<int>(bluelight_scale_.get_value()));

    show_all_children();
    std::cout << "Display tab loaded!" << std::endl;
}

/**
 * @brief Destructor for the display tab
 */
DisplayTab::~DisplayTab() = default;

/**
 * @brief Handler for brightness changes from the display manager
 * @param value The new brightness value (0-100)
 *
 * Updates the UI when the brightness is changed externally.
 */
void DisplayTab::on_brightness_changed(int value) {
    // Block the signal handler to prevent feedback loops
    brightness_signal_handler_id_.block();

    // Update the slider value
    brightness_scale_.set_value(value);

    // Unblock the signal handler
    brightness_signal_handler_id_.unblock();

    // Update the icon and label
    update_brightness_icon(value);
}

/**
 * @brief Handler for brightness slider changes
 *
 * Updates the brightness when the slider is moved by the user.
 */
void DisplayTab::on_slider_changed() {
    // Get the current slider value
    int value = static_cast<int>(brightness_scale_.get_value());

    // Update the system brightness
    manager_->set_brightness(value);

    // Update the icon and label
    update_brightness_icon(value);

    // Update the brightness value label with percentage
    std::stringstream ss;
    ss << value << "%";
    brightness_value_label_.set_text(ss.str());
}

/**
 * @brief Handler for color temperature slider changes
 *
 * Updates the color temperature when the slider is moved by the user.
 * Uses the gammastep utility to adjust the screen color temperature.
 */
void DisplayTab::on_bluelight_changed() {
    // Get the current slider value
    int temp = static_cast<int>(bluelight_scale_.get_value());

    // Execute gammastep to change the color temperature
    std::string cmd = "gammastep -O " + std::to_string(temp) + " &";
    std::system(cmd.c_str());

    // Update the icon and label
    update_bluelight_icon(temp);

    // Update the color temperature value label with Kelvin units
    std::stringstream ss;
    ss << temp << "K";
    bluelight_value_label_.set_text(ss.str());
}

/**
 * @brief Update the brightness icon and label
 * @param value The brightness value (0-100)
 *
 * Updates the brightness icon and value label based on the current brightness.
 */
void DisplayTab::update_brightness_icon(int value) {
    // Always use the same brightness icon regardless of value
    brightness_icon_.set_from_icon_name("display-brightness-symbolic", Gtk::ICON_SIZE_DIALOG);

    // Update the brightness value label with percentage
    std::stringstream ss;
    ss << value << "%";
    brightness_value_label_.set_text(ss.str());
}

/**
 * @brief Update the color temperature icon and label
 * @param value The color temperature value (1000-6500K)
 *
 * Updates the color temperature icon and value label based on the current temperature.
 */
void DisplayTab::update_bluelight_icon(int value) {
    // Always use the same night light icon regardless of value
    bluelight_icon_.set_from_icon_name("night-light-symbolic", Gtk::ICON_SIZE_DIALOG);

    // Update the color temperature value label with Kelvin units
    std::stringstream ss;
    ss << value << "K";
    bluelight_value_label_.set_text(ss.str());
}

} // namespace Display
