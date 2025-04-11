/**
 * @file SettingsWindow.hpp
 * @brief Settings window for the Ultimate Control application
 *
 * This file defines the SettingsWindow class which provides a user interface
 * for configuring application settings, including tab visibility and order.
 */

#pragma once

#include <gtkmm.h>
#include "TabSettings.hpp"
#include <memory>
#include <vector>
#include <functional>

/**
 * @namespace Settings
 * @brief Contains settings-related UI components
 */
namespace Settings
{

    /**
     * @class SettingsWindow
     * @brief Modal dialog for configuring application settings
     *
     * Provides a user interface for configuring application settings,
     * including tab visibility and order. Changes are saved to a
     * configuration file and can trigger application restart.
     * Implemented as a modal dialog that blocks interaction with the parent window.
     */
    class SettingsWindow : public Gtk::Dialog
    {
    public:
        /**
         * @brief Constructor for the settings window
         * @param parent The parent window that this dialog will be modal to
         *
         * Initializes the settings manager and creates the UI components.
         * Creates a modal dialog that blocks interaction with the parent window.
         */
        SettingsWindow(Gtk::Window &parent);

        /**
         * @brief Virtual destructor
         */
        virtual ~SettingsWindow();

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
         * @brief Create the general settings section
         *
         * Creates the UI components for configuring general application settings.
         */
        void create_general_settings_section();

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
        void on_tab_enabled_toggled(const std::string &tab_id);

        /**
         * @brief Handler for dialog response signals
         * @param response_id The ID of the response (e.g., RESPONSE_CANCEL, RESPONSE_APPLY)
         *
         * Handles the dialog response signals. If RESPONSE_APPLY, saves the settings
         * and restarts the application to apply changes. Otherwise, just closes the dialog.
         */
        void on_response(int response_id);

        std::shared_ptr<TabSettings> settings_;             ///< Tab settings manager
        SettingsChangedCallback settings_changed_callback_; ///< Callback for settings changes

        // Main containers
        Gtk::Box main_box_;                   ///< Main vertical box for all settings
        Gtk::ScrolledWindow scrolled_window_; ///< Scrollable container for the settings
        Gtk::Box content_box_;                ///< Content box for all settings

        // General settings section
        Gtk::Frame general_settings_frame_; ///< Frame around the general settings section
        Gtk::Box general_settings_box_;     ///< Container for general settings components
        Gtk::Box general_header_box_;       ///< Container for general section header
        Gtk::Image general_icon_;           ///< Icon for the general settings section
        Gtk::Label general_label_;          ///< Label for the general settings section
        Gtk::CheckButton floating_check_;   ///< Checkbox for enabling floating mode by default

        // Tab order section
        Gtk::Frame tab_order_frame_;    ///< Frame around the tab order section
        Gtk::Box tab_order_box_;        ///< Container for tab order components
        Gtk::Box tab_order_header_box_; ///< Container for section header
        Gtk::Image tab_order_icon_;     ///< Icon for the tab order section
        Gtk::Label tab_order_label_;    ///< Label for the tab order section

        // Tab list
        Gtk::Box tab_list_box_; ///< Container for tab rows

        /**
         * @struct TabRow
         * @brief Represents a row in the tab list
         *
         * Contains all the UI components for a single tab in the settings list.
         */
        struct TabRow
        {
            std::string id;                 ///< ID of the tab
            Gtk::Box row_box;               ///< Container for the row
            Gtk::CheckButton enabled_check; ///< Checkbox for enabling/disabling the tab
            Gtk::Label name_label;          ///< Label showing the tab name
            Gtk::Button up_button;          ///< Button to move the tab up
            Gtk::Button down_button;        ///< Button to move the tab down
        };
        std::vector<std::unique_ptr<TabRow>> tab_rows_;

        // No longer need separate buttons as we use the dialog's built-in buttons
    };

} // namespace Settings
