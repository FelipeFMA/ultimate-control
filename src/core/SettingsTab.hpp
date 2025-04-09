#ifndef CORE_SETTINGSTAB_HPP
#define CORE_SETTINGSTAB_HPP

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <map>
#include <string>

namespace Core {

class SettingsTab : public Gtk::Box {
public:
    SettingsTab();
    ~SettingsTab();

    void load_settings();
    void save_settings();

private:
    Gtk::CheckButton autostart_check_;
    Gtk::CheckButton notifications_check_;
    Gtk::ComboBoxText language_combo_;

    std::map<std::string, std::string> settings_;
};

} // namespace Core

#endif // CORE_SETTINGSTAB_HPP
