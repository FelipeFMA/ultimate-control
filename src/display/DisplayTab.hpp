#ifndef ULTIMATE_CONTROL_DISPLAY_TAB_HPP
#define ULTIMATE_CONTROL_DISPLAY_TAB_HPP

#include <gtkmm.h>
#include "DisplayManager.hpp"
#include <memory>

namespace Display {

class DisplayTab : public Gtk::Box {
public:
    DisplayTab();
    virtual ~DisplayTab();

private:
    void on_brightness_changed(int value);
    void on_slider_changed();
    void on_bluelight_changed();
    void update_brightness_icon(int value);
    void update_bluelight_icon(int value);

    std::shared_ptr<DisplayManager> manager_;

    // Main containers
    Gtk::ScrolledWindow scrolled_window_;
    Gtk::Box main_box_;

    // Brightness section
    Gtk::Frame brightness_frame_;
    Gtk::Box brightness_box_;
    Gtk::Box brightness_header_box_;
    Gtk::Image brightness_icon_;
    Gtk::Label brightness_label_;
    Gtk::Label brightness_value_label_;
    Gtk::Scale brightness_scale_;

    // Color temperature section
    Gtk::Frame bluelight_frame_;
    Gtk::Box bluelight_box_;
    Gtk::Box bluelight_header_box_;
    Gtk::Image bluelight_icon_;
    Gtk::Label bluelight_label_;
    Gtk::Label bluelight_value_label_;
    Gtk::Scale bluelight_scale_;

    sigc::connection brightness_signal_handler_id_;
};

} // namespace Display

#endif // ULTIMATE_CONTROL_DISPLAY_TAB_HPP
