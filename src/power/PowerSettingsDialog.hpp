#ifndef ULTIMATE_CONTROL_POWER_SETTINGS_DIALOG_HPP
#define ULTIMATE_CONTROL_POWER_SETTINGS_DIALOG_HPP

#include <gtkmm.h>
#include "PowerSettings.hpp"
#include <memory>
#include <map>

namespace Power {

class PowerSettingsDialog : public Gtk::Dialog {
public:
    PowerSettingsDialog(Gtk::Window& parent, std::shared_ptr<PowerSettings> settings);
    virtual ~PowerSettingsDialog();
    void save_settings();

private:
    void load_settings();
    void reset_to_defaults();

    std::shared_ptr<PowerSettings> settings_;

    // Command entry fields
    struct CommandEntry {
        Gtk::Label label;
        Gtk::Entry entry;
    };

    std::map<std::string, CommandEntry> command_entries_;
    Gtk::Button* reset_button_;
};

} // namespace Power

#endif // ULTIMATE_CONTROL_POWER_SETTINGS_DIALOG_HPP
