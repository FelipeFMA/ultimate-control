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

    std::shared_ptr<DisplayManager> manager_;
    Gtk::Scale brightness_scale_;
    Gtk::Scale bluelight_scale_;
    sigc::connection brightness_signal_handler_id_;
};

} // namespace Display

#endif // ULTIMATE_CONTROL_DISPLAY_TAB_HPP
