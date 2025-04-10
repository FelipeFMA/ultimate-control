/**
 * @file SettingsTab.hpp
 * @brief Settings tab for the Ultimate Control application
 *
 * This file defines the SettingsTab class which provides a user interface
 * for configuring application settings, including tab visibility and order.
 */

#ifndef ULTIMATE_CONTROL_SETTINGS_TAB_HPP
#define ULTIMATE_CONTROL_SETTINGS_TAB_HPP

#include <gtkmm.h>
#include "TabSettings.hpp"
#include <memory>
#include <vector>
#include <functional>

/**
 * @namespace Settings
 * @brief Contains settings-related UI components
 */
namespace Settings {

/**
 * @class SettingsTab
 * @brief Tab for configuring application settings
 *
 * Provides a user interface for configuring application settings,
 * including tab visibility and order. Changes are saved to a
 * configuration file and can trigger application restart.
 */
class SettingsTab : public Gtk::Box {
public:
    /**
     * @brief Constructor for the settings tab
     *
     * Initializes the settings manager and creates the UI components.
     */
    SettingsTab();

    /**
     * @brief Virtual destructor
     */
    virtual ~SettingsTab();

    /**
     * @brief Type definition for settings changed callback
     *
     * Function type that will be called when settings are changed.
     */
    using SettingsChangedCallback = std::function<void()>;

    /**
     * @brief Set the callback for settings changes
     * @param callback Function to call when settings are changed
     *
     * Sets the callback function that will be called when settings are changed.
     */
    void set_settings_changed_callback(SettingsChangedCallback callback);

    /**
     * @brief Get the tab settings object
     * @return Shared pointer to the TabSettings object
     *
     * Returns the TabSettings object that manages tab configuration.
     */
    std::shared_ptr<TabSettings> get_tab_settings() const;

private:
    /**
     * @brief Create the tab order configuration section
     *
     * Creates the UI components for configuring tab order and visibility.
     */
    void create_tab_order_section();

    /**
     * @brief Update the tab list display
     *
     * Updates the list of tabs based on the current settings.
     */
    void update_tab_list();

    /**
     * @brief Handler for move up button clicks
     *
     * Moves the selected tab up in the order.
     */
    void on_move_up_clicked();

    /**
     * @brief Handler for move down button clicks
     *
     * Moves the selected tab down in the order.
     */
    void on_move_down_clicked();

    /**
     * @brief Handler for tab enabled checkbox toggles
     * @param tab_id ID of the tab whose enabled state changed
     *
     * Updates the enabled state of a tab when its checkbox is toggled.
     */
    void on_tab_enabled_toggled(const std::string& tab_id);

    /**
     * @brief Handler for save button clicks
     *
     * Saves the settings and restarts the application to apply changes.
     */
    void on_save_clicked();

    std::shared_ptr<TabSettings> settings_;                ///< Tab settings manager
    SettingsChangedCallback settings_changed_callback_;    ///< Callback for settings changes

    // Main containers
    Gtk::ScrolledWindow scrolled_window_;  ///< Scrollable container for the settings
    Gtk::Box main_box_;                    ///< Main vertical box for all settings

    // Tab order section
    Gtk::Frame tab_order_frame_;       ///< Frame around the tab order section
    Gtk::Box tab_order_box_;           ///< Container for tab order components
    Gtk::Box tab_order_header_box_;    ///< Container for section header
    Gtk::Image tab_order_icon_;        ///< Icon for the tab order section
    Gtk::Label tab_order_label_;       ///< Label for the tab order section

    // Tab list
    Gtk::Box tab_list_box_;            ///< Container for tab rows

    /**
     * @struct TabRow
     * @brief Represents a row in the tab list
     *
     * Contains all the UI components for a single tab in the settings list.
     */
    struct TabRow {
        std::string id;              ///< ID of the tab
        Gtk::Box row_box;              ///< Container for the row
        Gtk::CheckButton enabled_check; ///< Checkbox for enabling/disabling the tab
        Gtk::Label name_label;          ///< Label showing the tab name
        Gtk::Button up_button;          ///< Button to move the tab up
        Gtk::Button down_button;        ///< Button to move the tab down
    };
    std::vector<std::unique_ptr<TabRow>> tab_rows_;

    // Buttons
    Gtk::Box buttons_box_;     ///< Container for buttons at the bottom
    Gtk::Button save_button_;  ///< Button to save settings
};

} // namespace Settings

#endif // ULTIMATE_CONTROL_SETTINGS_TAB_HPP
