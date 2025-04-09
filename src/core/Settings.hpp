#ifndef ULTIMATE_CONTROL_SETTINGS_HPP
#define ULTIMATE_CONTROL_SETTINGS_HPP

#include <gtkmm.h>
#include <memory>
#include <string>
#include <map>

namespace Core {

class SettingsWindow : public Gtk::Dialog {
public:
    SettingsWindow(Gtk::Window& parent);
    virtual ~SettingsWindow();

private:
    void load_settings();
    void save_settings();

    Gtk::CheckButton autostart_check_;
    Gtk::CheckButton notifications_check_;
    Gtk::ComboBoxText language_combo_;

    std::map<std::string, std::string> settings_;
};

} // namespace Core

#endif // ULTIMATE_CONTROL_SETTINGS_HPP
