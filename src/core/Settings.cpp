/**
 * @file Settings.cpp
 * @brief Implementation of core application settings
 *
 * This file implements the SettingsWindow class which provides a user interface
 * for configuring application-wide settings such as autostart, notifications,
 * and language preferences.
 */

#include "Settings.hpp"
#include <gtkmm/box.h>       // for Gtk::Box
#include <gtkmm/buttonbox.h> // for Gtk::ButtonBox
#include <gtkmm/label.h>     // for Gtk::Label
#include <fstream>           // for std::ifstream, std::ofstream
#include <iostream>          // for std::cerr

namespace Core {

/**
 * @brief Get a setting value from the configuration file
 * @param key The setting key to retrieve
 * @param default_value The default value to return if the setting is not found
 * @return The setting value or the default value if not found
 */
std::string get_setting(const std::string& key, const std::string& default_value) {
    // Try to open the configuration file
    std::string config_path = "/home/felipe/.config/ultimate-control/general.conf";
    std::ifstream infile(config_path);
    if (!infile.is_open()) return default_value;  // Return default if file doesn't exist

    // Read key-value pairs from the file
    std::string k, v;
    while (infile >> k >> v) {
        if (k == key) {
            return v;  // Return the value if the key is found
        }
    }

    return default_value;  // Return default if key not found
}

/**
 * @brief Constructor for the settings window
 * @param parent Parent window for the dialog
 *
 * Creates a dialog for configuring application settings.
 */
SettingsWindow::SettingsWindow(Gtk::Window& parent)
: Gtk::Dialog("Settings", parent, true),                // Modal dialog with title
  autostart_check_("Enable autostart"),                 // Checkbox for autostart
  notifications_check_("Enable notifications"),         // Checkbox for notifications
  floating_check_("Start in floating mode by default")  // Checkbox for floating mode
{
    // Set initial dialog size
    set_default_size(400, 200);

    // Get the content area of the dialog
    auto content = get_content_area();

    // Create a vertical box with margins for the settings
    auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    vbox->set_margin_top(10);
    vbox->set_margin_bottom(10);
    vbox->set_margin_start(10);
    vbox->set_margin_end(10);

    // Add checkboxes for autostart, notifications, and floating mode
    vbox->pack_start(autostart_check_, Gtk::PACK_SHRINK);
    vbox->pack_start(notifications_check_, Gtk::PACK_SHRINK);
    vbox->pack_start(floating_check_, Gtk::PACK_SHRINK);

    // Add language selection label
    auto lang_label = Gtk::make_managed<Gtk::Label>("Language:");
    lang_label->set_halign(Gtk::ALIGN_START);
    vbox->pack_start(*lang_label, Gtk::PACK_SHRINK);

    // Add language options to the dropdown
    language_combo_.append("en");  // English
    language_combo_.append("pt");  // Portuguese
    language_combo_.append("es");  // Spanish
    language_combo_.set_active_text("en");  // Default to English
    vbox->pack_start(language_combo_, Gtk::PACK_SHRINK);

    // Add the settings box to the dialog content area
    content->pack_start(*vbox, Gtk::PACK_EXPAND_WIDGET);

    // Add standard dialog buttons (Cancel and Save)
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    add_button("_Save", Gtk::RESPONSE_OK);

    // Load existing settings from the configuration file
    load_settings();

    // Show all UI components
    show_all_children();
}

/**
 * @brief Destructor for the settings window
 *
 * Saves settings to the configuration file before destruction.
 */
SettingsWindow::~SettingsWindow() {
    save_settings();  // Save settings when the dialog is closed
}

/**
 * @brief Load settings from the configuration file
 *
 * Reads settings from the configuration file and updates the UI.
 */
void SettingsWindow::load_settings() {
    // Try to open the configuration file
    std::string config_path = "/home/felipe/.config/ultimate-control/general.conf";
    std::ifstream infile(config_path);
    if (!infile.is_open()) return;  // Return if file doesn't exist or can't be opened

    // Read key-value pairs from the file
    std::string key, value;
    while (infile >> key >> value) {
        settings_[key] = value;  // Store each setting in the map
    }

    // Update UI components with loaded settings
    autostart_check_.set_active(settings_["autostart"] == "1");
    notifications_check_.set_active(settings_["notifications"] == "1");
    floating_check_.set_active(settings_["floating"] == "1");

    // Set the language dropdown if a language is specified
    if (!settings_["language"].empty()) {
        language_combo_.set_active_text(settings_["language"]);
    }
}

/**
 * @brief Save settings to the configuration file
 *
 * Writes the current settings to the configuration file.
 */
void SettingsWindow::save_settings() {
    // Update settings map from UI components
    settings_["autostart"] = autostart_check_.get_active() ? "1" : "0";
    settings_["notifications"] = notifications_check_.get_active() ? "1" : "0";
    settings_["floating"] = floating_check_.get_active() ? "1" : "0";
    settings_["language"] = language_combo_.get_active_text();

    // Try to open the configuration file for writing
    std::ofstream outfile("/home/felipe/.config/ultimate-control/general.conf");
    if (!outfile.is_open()) {
        std::cerr << "Failed to save settings\n";
        return;  // Return if file can't be opened for writing
    }

    // Write each setting as a key-value pair
    for (const auto& pair : settings_) {
        outfile << pair.first << " " << pair.second << "\n";
    }
}

} // namespace Core
