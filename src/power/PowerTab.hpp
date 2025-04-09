#ifndef ULTIMATE_CONTROL_POWER_TAB_HPP
#define ULTIMATE_CONTROL_POWER_TAB_HPP

#include <gtkmm.h>
#include "PowerManager.hpp"
#include "PowerSettingsDialog.hpp"
#include <memory>

namespace Power {

class PowerTab : public Gtk::Box {
public:
    PowerTab();
    virtual ~PowerTab();

private:
    void create_system_section();
    void create_session_section();
    void create_power_profiles_section();
    void on_settings_clicked();
    void add_settings_button_to_header(Gtk::Box& header_box);

    std::shared_ptr<PowerManager> manager_;

    // Main containers
    Gtk::ScrolledWindow scrolled_window_;
    Gtk::Box main_box_;

    // System power section
    Gtk::Frame system_frame_;
    Gtk::Box system_box_;
    Gtk::Box system_header_box_;
    Gtk::Image system_icon_;
    Gtk::Label system_label_;
    Gtk::Box system_buttons_box_;
    Gtk::Button shutdown_button_;
    Gtk::Button reboot_button_;

    // Session section
    Gtk::Frame session_frame_;
    Gtk::Box session_box_;
    Gtk::Box session_header_box_;
    Gtk::Image session_icon_;
    Gtk::Label session_label_;
    Gtk::Box session_buttons_box_;
    Gtk::Button suspend_button_;
    Gtk::Button hibernate_button_;
    Gtk::Button lock_button_;

    // Power profiles section
    Gtk::Frame profiles_frame_;
    Gtk::Box profiles_box_;
    Gtk::Box profiles_header_box_;
    Gtk::Image profiles_icon_;
    Gtk::Label profiles_label_;
    Gtk::Box profiles_content_box_;
    Gtk::ComboBoxText profile_combo_;
};

} // namespace Power

#endif // ULTIMATE_CONTROL_POWER_TAB_HPP
