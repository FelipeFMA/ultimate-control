#include "Settings.hpp"
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <fstream>
#include <iostream>

namespace Core {

SettingsWindow::SettingsWindow(Gtk::Window& parent)
: Gtk::Dialog("Settings", parent, true),
  autostart_check_("Enable autostart"),
  notifications_check_("Enable notifications")
{
    set_default_size(400, 200);

    auto content = get_content_area();

    auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    vbox->set_margin_top(10);
    vbox->set_margin_bottom(10);
    vbox->set_margin_start(10);
    vbox->set_margin_end(10);

    vbox->pack_start(autostart_check_, Gtk::PACK_SHRINK);
    vbox->pack_start(notifications_check_, Gtk::PACK_SHRINK);

    auto lang_label = Gtk::make_managed<Gtk::Label>("Language:");
    lang_label->set_halign(Gtk::ALIGN_START);
    vbox->pack_start(*lang_label, Gtk::PACK_SHRINK);

    language_combo_.append("en");
    language_combo_.append("pt");
    language_combo_.append("es");
    language_combo_.set_active_text("en");
    vbox->pack_start(language_combo_, Gtk::PACK_SHRINK);

    content->pack_start(*vbox, Gtk::PACK_EXPAND_WIDGET);

    add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    add_button("_Save", Gtk::RESPONSE_OK);

    load_settings();

    show_all_children();
}

SettingsWindow::~SettingsWindow() {
    save_settings();
}

void SettingsWindow::load_settings() {
    std::ifstream infile("/home/felipe/.config/ultimate-control/settings.conf");
    if (!infile.is_open()) return;

    std::string key, value;
    while (infile >> key >> value) {
        settings_[key] = value;
    }

    autostart_check_.set_active(settings_["autostart"] == "1");
    notifications_check_.set_active(settings_["notifications"] == "1");
    if (!settings_["language"].empty()) {
        language_combo_.set_active_text(settings_["language"]);
    }
}

void SettingsWindow::save_settings() {
    settings_["autostart"] = autostart_check_.get_active() ? "1" : "0";
    settings_["notifications"] = notifications_check_.get_active() ? "1" : "0";
    settings_["language"] = language_combo_.get_active_text();

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
