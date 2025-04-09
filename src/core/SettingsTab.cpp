#include "SettingsTab.hpp"
#include <gtkmm/label.h>
#include <fstream>
#include <iostream>

namespace Core {

SettingsTab::SettingsTab()
: Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10),
  autostart_check_("Enable autostart"),
  notifications_check_("Enable notifications")
{
    set_margin_top(10);
    set_margin_bottom(10);
    set_margin_start(10);
    set_margin_end(10);

    pack_start(autostart_check_, Gtk::PACK_SHRINK);
    pack_start(notifications_check_, Gtk::PACK_SHRINK);

    load_settings();

    show_all_children();
}

SettingsTab::~SettingsTab() {
    save_settings();
}

void SettingsTab::load_settings() {
    std::ifstream infile("/home/felipe/.config/ultimate-control/settings.conf");
    if (!infile.is_open()) return;

    std::string key, value;
    while (infile >> key >> value) {
        settings_[key] = value;
    }

    autostart_check_.set_active(settings_["autostart"] == "1");
    notifications_check_.set_active(settings_["notifications"] == "1");
}

void SettingsTab::save_settings() {
    settings_["autostart"] = autostart_check_.get_active() ? "1" : "0";
    settings_["notifications"] = notifications_check_.get_active() ? "1" : "0";

    std::ofstream outfile("/home/felipe/.config/ultimate-control/settings.conf");
    if (!outfile.is_open()) {
        std::cerr << "Failed to save settings\n";
        return;
    }

    for (const auto& pair : settings_) {
        outfile << pair.first << " " << pair.second << "\n";
    }
}

} // namespace Core
