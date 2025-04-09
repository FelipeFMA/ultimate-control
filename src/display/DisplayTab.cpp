#include "DisplayTab.hpp"
#include <iostream>

namespace Display {

DisplayTab::DisplayTab()
: manager_(std::make_shared<DisplayManager>()),
  brightness_scale_(Gtk::ORIENTATION_HORIZONTAL),
  bluelight_scale_(Gtk::ORIENTATION_HORIZONTAL)
{
    set_margin_start(10);
    set_margin_end(10);
    set_margin_top(10);
    set_margin_bottom(10);

    brightness_scale_.set_range(0, 100);
    brightness_scale_.set_increments(1, 10);
    brightness_scale_.set_size_request(300, -1);
    brightness_scale_.set_draw_value(true);

    bluelight_scale_.set_range(1000, 6500);
    bluelight_scale_.set_increments(100, 500);
    bluelight_scale_.set_size_request(300, -1);
    bluelight_scale_.set_draw_value(true);
    bluelight_scale_.set_value(6500);

    pack_start(brightness_scale_, Gtk::PACK_SHRINK);
    pack_start(bluelight_scale_, Gtk::PACK_SHRINK);

    brightness_signal_handler_id_ = brightness_scale_.signal_value_changed().connect(sigc::mem_fun(*this, &DisplayTab::on_slider_changed));
    bluelight_scale_.signal_value_changed().connect(sigc::mem_fun(*this, &DisplayTab::on_bluelight_changed));

    manager_->set_update_callback([this](int value) {
        on_brightness_changed(value);
    });

    show_all_children();
}

DisplayTab::~DisplayTab() = default;

void DisplayTab::on_brightness_changed(int value) {
    brightness_signal_handler_id_.block();
    brightness_scale_.set_value(value);
    brightness_signal_handler_id_.unblock();
}

void DisplayTab::on_slider_changed() {
    int value = static_cast<int>(brightness_scale_.get_value());
    manager_->set_brightness(value);
}

void DisplayTab::on_bluelight_changed() {
    int temp = static_cast<int>(bluelight_scale_.get_value());
    std::string cmd = "gammastep -O " + std::to_string(temp) + " &";
    std::system(cmd.c_str());
}

} // namespace Display
