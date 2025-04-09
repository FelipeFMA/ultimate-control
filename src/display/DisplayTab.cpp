#include "DisplayTab.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Display {

DisplayTab::DisplayTab()
: manager_(std::make_shared<DisplayManager>()),
  main_box_(Gtk::ORIENTATION_VERTICAL, 15),
  brightness_box_(Gtk::ORIENTATION_VERTICAL, 10),
  brightness_header_box_(Gtk::ORIENTATION_HORIZONTAL, 10),
  brightness_scale_(Gtk::ORIENTATION_HORIZONTAL),
  bluelight_box_(Gtk::ORIENTATION_VERTICAL, 10),
  bluelight_header_box_(Gtk::ORIENTATION_HORIZONTAL, 10),
  bluelight_scale_(Gtk::ORIENTATION_HORIZONTAL)
{
    // Set up the main container
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    // Add a scrolled window
    scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the main box inside the scrolled window
    main_box_.set_margin_start(20);
    main_box_.set_margin_end(20);
    main_box_.set_margin_top(20);
    main_box_.set_margin_bottom(20);
    scrolled_window_.add(main_box_);

    // Set up the brightness section
    brightness_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    brightness_box_.set_margin_start(15);
    brightness_box_.set_margin_end(15);
    brightness_box_.set_margin_top(15);
    brightness_box_.set_margin_bottom(15);

    // Set up the brightness header
    brightness_icon_.set_from_icon_name("display-brightness-symbolic", Gtk::ICON_SIZE_DIALOG);
    brightness_label_.set_markup("<span size='large' weight='bold'>Screen Brightness</span>");
    brightness_label_.set_halign(Gtk::ALIGN_START);
    brightness_label_.set_valign(Gtk::ALIGN_CENTER);
    brightness_value_label_.set_text("50%");
    brightness_value_label_.set_halign(Gtk::ALIGN_END);

    brightness_header_box_.pack_start(brightness_icon_, Gtk::PACK_SHRINK);
    brightness_header_box_.pack_start(brightness_label_, Gtk::PACK_EXPAND_WIDGET);
    brightness_header_box_.pack_end(brightness_value_label_, Gtk::PACK_SHRINK);

    // Set up the brightness scale
    brightness_scale_.set_range(0, 100);
    brightness_scale_.set_increments(1, 10);
    brightness_scale_.set_size_request(300, -1);
    brightness_scale_.set_draw_value(false);
    brightness_scale_.add_mark(0, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(25, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(50, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(75, Gtk::POS_BOTTOM, "");
    brightness_scale_.add_mark(100, Gtk::POS_BOTTOM, "");

    // Add brightness components to the brightness box
    brightness_box_.pack_start(brightness_header_box_, Gtk::PACK_SHRINK);
    brightness_box_.pack_start(brightness_scale_, Gtk::PACK_SHRINK);

    // Add the brightness box to the frame
    brightness_frame_.add(brightness_box_);

    // Set up the bluelight section
    bluelight_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    bluelight_box_.set_margin_start(15);
    bluelight_box_.set_margin_end(15);
    bluelight_box_.set_margin_top(15);
    bluelight_box_.set_margin_bottom(15);

    // Set up the bluelight header
    bluelight_icon_.set_from_icon_name("night-light-symbolic", Gtk::ICON_SIZE_DIALOG);
    bluelight_label_.set_markup("<span size='large' weight='bold'>Color Temperature</span>");
    bluelight_label_.set_halign(Gtk::ALIGN_START);
    bluelight_label_.set_valign(Gtk::ALIGN_CENTER);
    bluelight_value_label_.set_text("6500K");
    bluelight_value_label_.set_halign(Gtk::ALIGN_END);

    bluelight_header_box_.pack_start(bluelight_icon_, Gtk::PACK_SHRINK);
    bluelight_header_box_.pack_start(bluelight_label_, Gtk::PACK_EXPAND_WIDGET);
    bluelight_header_box_.pack_end(bluelight_value_label_, Gtk::PACK_SHRINK);

    // Set up the bluelight scale
    bluelight_scale_.set_range(1000, 6500);
    bluelight_scale_.set_increments(100, 500);
    bluelight_scale_.set_size_request(300, -1);
    bluelight_scale_.set_draw_value(false);
    bluelight_scale_.add_mark(1000, Gtk::POS_BOTTOM, "Warm");
    bluelight_scale_.add_mark(3750, Gtk::POS_BOTTOM, "");
    bluelight_scale_.add_mark(6500, Gtk::POS_BOTTOM, "Cool");
    bluelight_scale_.set_value(6500);

    // Add bluelight components to the bluelight box
    bluelight_box_.pack_start(bluelight_header_box_, Gtk::PACK_SHRINK);
    bluelight_box_.pack_start(bluelight_scale_, Gtk::PACK_SHRINK);

    // Add the bluelight box to the frame
    bluelight_frame_.add(bluelight_box_);

    // Add frames to the main box
    main_box_.pack_start(brightness_frame_, Gtk::PACK_SHRINK);
    main_box_.pack_start(bluelight_frame_, Gtk::PACK_SHRINK);

    // Connect signals
    brightness_signal_handler_id_ = brightness_scale_.signal_value_changed().connect(sigc::mem_fun(*this, &DisplayTab::on_slider_changed));
    bluelight_scale_.signal_value_changed().connect(sigc::mem_fun(*this, &DisplayTab::on_bluelight_changed));

    // Set up the callback for brightness updates
    manager_->set_update_callback([this](int value) {
        on_brightness_changed(value);
    });

    // Update initial values
    update_brightness_icon(manager_->get_brightness());
    update_bluelight_icon(static_cast<int>(bluelight_scale_.get_value()));

    show_all_children();
}

DisplayTab::~DisplayTab() = default;

void DisplayTab::on_brightness_changed(int value) {
    brightness_signal_handler_id_.block();
    brightness_scale_.set_value(value);
    brightness_signal_handler_id_.unblock();
    update_brightness_icon(value);
}

void DisplayTab::on_slider_changed() {
    int value = static_cast<int>(brightness_scale_.get_value());
    manager_->set_brightness(value);
    update_brightness_icon(value);

    // Update the brightness value label
    std::stringstream ss;
    ss << value << "%";
    brightness_value_label_.set_text(ss.str());
}

void DisplayTab::on_bluelight_changed() {
    int temp = static_cast<int>(bluelight_scale_.get_value());
    std::string cmd = "gammastep -O " + std::to_string(temp) + " &";
    std::system(cmd.c_str());
    update_bluelight_icon(temp);

    // Update the bluelight value label
    std::stringstream ss;
    ss << temp << "K";
    bluelight_value_label_.set_text(ss.str());
}

void DisplayTab::update_brightness_icon(int value) {
    std::string icon_name;

    if (value < 30) {
        icon_name = "display-brightness-symbolic";
    } else if (value < 70) {
        icon_name = "display-brightness-symbolic";
    } else {
        icon_name = "display-brightness-symbolic";
    }

    brightness_icon_.set_from_icon_name(icon_name, Gtk::ICON_SIZE_DIALOG);

    // Update the brightness value label
    std::stringstream ss;
    ss << value << "%";
    brightness_value_label_.set_text(ss.str());
}

void DisplayTab::update_bluelight_icon(int value) {
    std::string icon_name;

    if (value < 3000) {
        icon_name = "night-light-symbolic";
    } else if (value < 5000) {
        icon_name = "night-light-symbolic";
    } else {
        icon_name = "night-light-symbolic";
    }

    bluelight_icon_.set_from_icon_name(icon_name, Gtk::ICON_SIZE_DIALOG);

    // Update the bluelight value label
    std::stringstream ss;
    ss << value << "K";
    bluelight_value_label_.set_text(ss.str());
}

} // namespace Display
