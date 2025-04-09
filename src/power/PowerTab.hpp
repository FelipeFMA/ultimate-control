#ifndef ULTIMATE_CONTROL_POWER_TAB_HPP
#define ULTIMATE_CONTROL_POWER_TAB_HPP

#include <gtkmm.h>
#include "PowerManager.hpp"
#include <memory>

namespace Power {

class PowerTab : public Gtk::Box {
public:
    PowerTab();
    virtual ~PowerTab();

private:
    std::shared_ptr<PowerManager> manager_;

    Gtk::Button shutdown_button_;
    Gtk::Button reboot_button_;
    Gtk::Button suspend_button_;
    Gtk::Button hibernate_button_;
    Gtk::Button lock_button_;
};

} // namespace Power

#endif // ULTIMATE_CONTROL_POWER_TAB_HPP
