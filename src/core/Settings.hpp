/**
 * @file Settings.hpp
 * @brief Core application settings for Ultimate Control
 *
 * This file defines the SettingsWindow class which provides a user interface
 * for configuring application-wide settings such as autostart, notifications,
 * and language preferences.
 */

#pragma once

#include <gtkmm.h>
#include <memory>
#include <string>
#include <map>

/**
 * @namespace Core
 * @brief Contains core application functionality
 */
namespace Core {

/**
 * @brief Get a setting value from the configuration file
 * @param key The setting key to retrieve
 * @param default_value The default value to return if the setting is not found
 * @return The setting value or the default value if not found
 */
std::string get_setting(const std::string& key, const std::string& default_value = "");


/**
 * @class SettingsWindow
 * @brief Dialog for configuring application settings
 *
 * Provides a user interface for configuring application-wide settings
 * such as autostart, notifications, and language preferences.
 * Settings are persisted to a configuration file.
 */
class SettingsWindow : public Gtk::Dialog {
public:
    /**
     * @brief Constructor
     * @param parent Parent window for the dialog
     *
     * Creates a dialog for configuring application settings.
     */
    SettingsWindow(Gtk::Window& parent);

    /**
     * @brief Virtual destructor
     *
     * Saves settings before destruction.
     */
    virtual ~SettingsWindow();

private:
    /**
     * @brief Load settings from the configuration file
     *
     * Reads settings from the configuration file and updates the UI.
     */
    void load_settings();

    /**
     * @brief Save settings to the configuration file
     *
     * Writes the current settings to the configuration file.
     */
    void save_settings();

    Gtk::CheckButton autostart_check_;      ///< Checkbox for enabling application autostart
    Gtk::CheckButton notifications_check_;   ///< Checkbox for enabling notifications
    Gtk::CheckButton floating_check_;        ///< Checkbox for enabling floating mode by default
    Gtk::ComboBoxText language_combo_;       ///< Dropdown for selecting application language

    std::map<std::string, std::string> settings_;  ///< Map of setting names to values
};

} // namespace Core

