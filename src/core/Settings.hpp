/**
 * @file Settings.hpp
 * @brief Core application settings for Ultimate Control
 *
 * This file defines the SettingsWindow class which provides a user interface
 * for configuring application-wide settings such as autostart, notifications,
 * and language preferences.
 */

#ifndef ULTIMATE_CONTROL_SETTINGS_HPP
#define ULTIMATE_CONTROL_SETTINGS_HPP

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
    Gtk::ComboBoxText language_combo_;       ///< Dropdown for selecting application language

    std::map<std::string, std::string> settings_;  ///< Map of setting names to values
};

} // namespace Core

#endif // ULTIMATE_CONTROL_SETTINGS_HPP
