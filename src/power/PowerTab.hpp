/**
 * @file PowerTab.hpp
 * @brief Power management tab for Ultimate Control
 *
 * This file defines the PowerTab class which provides a user interface
 * for system power operations like shutdown, reboot, suspend, and hibernate,
 * as well as power profile management.
 */

#pragma once

#include <gtkmm.h>
#include "PowerManager.hpp"
#include "PowerSettingsDialog.hpp"
#include <memory>

/**
 * @namespace Power
 * @brief Contains power management functionality
 */
namespace Power
{

    /**
     * @class PowerTab
     * @brief Tab for power management operations
     *
     * Provides a user interface for system power operations (shutdown, reboot, etc.),
     * session actions (suspend, hibernate, lock), and power profile management.
     */
    class PowerTab : public Gtk::Box
    {
    public:
        /**
         * @brief Constructor
         *
         * Initializes the power manager and creates the UI components.
         */
        PowerTab();

        /**
         * @brief Virtual destructor
         */
        virtual ~PowerTab();

    private:
        /**
         * @brief Accelerator group for keyboard shortcuts (user-configurable)
         */
        Glib::RefPtr<Gtk::AccelGroup> accel_group_;

        /**
         * @brief Create the system power section
         *
         * Creates the UI components for system power operations
         * (shutdown and reboot).
         */
        void create_system_section();

        /**
         * @brief Create the session actions section
         *
         * Creates the UI components for session actions
         * (suspend, hibernate, and lock screen).
         */
        void create_session_section();

        /**
         * @brief Create the power profiles section
         *
         * Creates the UI components for power profile management.
         */
        void create_power_profiles_section();

        /**
         * @brief Handler for settings button clicks
         *
         * Opens the power settings dialog to configure power commands and keybinds.
         */
        void on_settings_clicked();

        /**
         * @brief Add a settings button to a section header
         * @param header_box The header box to add the button to
         *
         * Adds a settings button with a cog icon to a section header.
         */
        void add_settings_button_to_header(Gtk::Box &header_box);

        /**
         * @brief Set up accelerators for all power actions based on user keybinds
         *
         * Reads keybinds from settings and binds them to the appropriate buttons.
         * Should be called after loading or updating settings.
         */
        void setup_action_keybinds();

        /**
         * @brief Helper to parse a keybind string (e.g., "Ctrl+Alt+S") into keyval and modifier
         * @param keybind The keybind string
         * @param[out] keyval The resulting GDK key value
         * @param[out] modifier The resulting Gdk::ModifierType
         * @return true if parsing was successful, false otherwise
         */
        bool parse_keybind(const std::string &keybind, guint &keyval, Gdk::ModifierType &modifier);

        std::shared_ptr<PowerManager> manager_; ///< Power manager for executing power operations

        // Main containers
        Gtk::ScrolledWindow scrolled_window_; ///< Scrollable container for the tab
        Gtk::Box main_box_;                   ///< Main vertical box for all sections

        // System power section
        Gtk::Frame system_frame_;     ///< Frame around the system power section
        Gtk::Box system_box_;         ///< Container for system power components
        Gtk::Box system_header_box_;  ///< Container for section header
        Gtk::Image system_icon_;      ///< Icon for the system power section
        Gtk::Label system_label_;     ///< Label for the system power section
        Gtk::Box system_buttons_box_; ///< Container for system power buttons
        Gtk::Button shutdown_button_; ///< Button to shut down the system
        Gtk::Button reboot_button_;   ///< Button to reboot the system

        // Session section
        Gtk::Frame session_frame_;     ///< Frame around the session section
        Gtk::Box session_box_;         ///< Container for session components
        Gtk::Box session_header_box_;  ///< Container for section header
        Gtk::Image session_icon_;      ///< Icon for the session section
        Gtk::Label session_label_;     ///< Label for the session section
        Gtk::Box session_buttons_box_; ///< Container for session buttons
        Gtk::Button suspend_button_;   ///< Button to suspend the system
        Gtk::Button hibernate_button_; ///< Button to hibernate the system
        Gtk::Button lock_button_;      ///< Button to lock the screen

        // Power profiles section
        Gtk::Frame profiles_frame_;       ///< Frame around the power profiles section
        Gtk::Box profiles_box_;           ///< Container for power profiles components
        Gtk::Box profiles_header_box_;    ///< Container for section header
        Gtk::Image profiles_icon_;        ///< Icon for the power profiles section
        Gtk::Label profiles_label_;       ///< Label for the power profiles section
        Gtk::Box profiles_content_box_;   ///< Container for power profiles content
        Gtk::ComboBoxText profile_combo_; ///< Dropdown for selecting power profiles
    };

} // namespace Power
