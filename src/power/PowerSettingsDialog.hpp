/**
 * @file PowerSettingsDialog.hpp
 * @brief Dialog for configuring power commands
 *
 * This file defines the PowerSettingsDialog class which provides a user interface
 * for configuring the commands executed by power operations like shutdown,
 * reboot, suspend, etc.
 */

#pragma once

#include <gtkmm.h>
#include "PowerSettings.hpp"
#include <memory>
#include <map>

/**
 * @namespace Power
 * @brief Contains power management functionality
 */
namespace Power
{

    /**
     * @class PowerSettingsDialog
     * @brief Dialog for configuring power commands
     *
     * Provides a user interface for configuring the commands executed by
     * power operations like shutdown, reboot, suspend, etc.
     */
    class PowerSettingsDialog : public Gtk::Dialog
    {
    public:
        /**
         * @brief Constructor
         * @param parent Parent window for the dialog
         * @param settings Shared pointer to the power settings object
         *
         * Creates a dialog for configuring power commands.
         */
        PowerSettingsDialog(Gtk::Window &parent, std::shared_ptr<PowerSettings> settings);

        /**
         * @brief Virtual destructor
         */
        virtual ~PowerSettingsDialog();

        /**
         * @brief Save the configured settings
         *
         * Saves the command settings from the dialog to the PowerSettings object.
         */
        void save_settings();

    private:
        /**
         * @brief Load settings into the dialog
         *
         * Loads the current command settings from the PowerSettings object
         * into the dialog's entry fields.
         */
        void load_settings();

        /**
         * @brief Reset settings to defaults
         *
         * Resets all command and keybind entry fields to their default values.
         */
        void reset_to_defaults();

        std::shared_ptr<PowerSettings> settings_; ///< Power settings object

        /**
         * @struct CommandEntry
         * @brief Represents a command or keybind entry field
         *
         * Contains a label and text entry field for a single command or keybind.
         */
        struct CommandEntry
        {
            Gtk::Label label; ///< Label for the command or keybind
            Gtk::Entry entry; ///< Text entry field for the command or keybind
        };

        std::map<std::string, CommandEntry> command_entries_;  ///< Map of command names to entry fields
        std::map<std::string, CommandEntry> keybind_entries_;  ///< Map of action names to keybind entry fields
        Gtk::Button *reset_button_;                            ///< Button to reset settings to defaults
        Gtk::Notebook *notebook_;                              ///< Notebook for tabbed settings (commands/keybinds)
        Gtk::CheckButton *show_keybind_hints_check_ = nullptr; ///< Toggle for showing keybind hints
    };

} // namespace Power
