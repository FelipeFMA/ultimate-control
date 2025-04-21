/**
 * @file SettingsTab.cpp
 * @brief Implementation of the settings tab for Ultimate Control
 *
 * This file implements the SettingsTab class which provides a user interface
 * for configuring application settings, including tab visibility and order.
 */

#include "SettingsTab.hpp"
#include "../core/Settings.hpp" // for Core::get_setting
#include <iostream>
#include <cstdlib>  // for std::exit
#include <unistd.h> // for execl
#include <limits.h> // for PATH_MAX
#include <errno.h>  // for errno
#include <string.h> // for strerror
#include <fstream>  // for std::ifstream, std::ofstream

namespace Settings
{

    /**
     * @brief Constructor for the settings tab
     *
     * Initializes the settings manager and creates the UI components.
     */
    SettingsTab::SettingsTab()
        : settings_(std::make_shared<TabSettings>()),            // Create settings manager
          main_box_(Gtk::ORIENTATION_VERTICAL, 10),              // Main container with 10px spacing
          general_settings_box_(Gtk::ORIENTATION_VERTICAL, 8),   // General settings section with 8px spacing
          general_header_box_(Gtk::ORIENTATION_HORIZONTAL, 8),   // General header with 8px spacing
          tab_order_box_(Gtk::ORIENTATION_VERTICAL, 8),          // Tab order section with 8px spacing
          tab_order_header_box_(Gtk::ORIENTATION_HORIZONTAL, 8), // Tab order header with 8px spacing
          tab_list_box_(Gtk::ORIENTATION_VERTICAL, 5),           // Tab list with 5px spacing
          buttons_box_(Gtk::ORIENTATION_HORIZONTAL, 10)          // Buttons container with 10px spacing
    {
        // Set up the main container orientation
        set_orientation(Gtk::ORIENTATION_VERTICAL);

        // Add a scrolled window to contain all settings
        scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

        // Set up the main box inside the scrolled window with margins
        main_box_.set_margin_start(10);
        main_box_.set_margin_end(10);
        main_box_.set_margin_top(10);
        main_box_.set_margin_bottom(10);
        scrolled_window_.add(main_box_);

        // Create the general settings section
        create_general_settings_section();

        // Create the tab order configuration section
        create_tab_order_section();

        // Create and configure the save button
        save_button_.set_label("Save Settings");
        save_button_.set_image_from_icon_name("document-save-symbolic", Gtk::ICON_SIZE_BUTTON);
        save_button_.set_always_show_image(true);
        save_button_.signal_clicked().connect(sigc::mem_fun(*this, &SettingsTab::on_save_clicked));
        save_button_.set_can_focus(false); // Prevent tab navigation to this button

        buttons_box_.set_halign(Gtk::ALIGN_END);
        buttons_box_.pack_start(save_button_, Gtk::PACK_SHRINK);

        main_box_.pack_start(buttons_box_, Gtk::PACK_SHRINK);

        // Initialize the tab list with current settings
        update_tab_list();

        show_all_children();
        std::cout << "Settings tab loaded!" << std::endl;
    }

    /**
     * @brief Destructor for the settings tab
     */
    SettingsTab::~SettingsTab() = default;

    /**
     * @brief Create the general settings section
     *
     * Creates the UI components for configuring general application settings.
     */
    void SettingsTab::create_general_settings_section()
    {
        // Configure the frame for the general settings section
        general_settings_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        general_settings_frame_.set_margin_bottom(10);

        // Configure the general settings box with margins (same as tab settings)
        general_settings_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        general_settings_box_.set_spacing(8);
        general_settings_box_.set_margin_start(10);
        general_settings_box_.set_margin_end(10);
        general_settings_box_.set_margin_top(10);
        general_settings_box_.set_margin_bottom(10);

        // Add settings icon and title label to the header
        general_icon_.set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_LARGE_TOOLBAR);
        general_label_.set_markup("<span size='large'><b>General Settings</b></span>");
        general_label_.set_halign(Gtk::ALIGN_START);
        general_label_.set_valign(Gtk::ALIGN_CENTER);
        general_label_.set_margin_start(10);

        general_header_box_.pack_start(general_icon_, Gtk::PACK_SHRINK);
        general_header_box_.pack_start(general_label_, Gtk::PACK_SHRINK);

        // Add descriptive text explaining the settings
        Gtk::Label *description = Gtk::manage(new Gtk::Label());
        description->set_markup("Configure general application settings:");
        description->set_halign(Gtk::ALIGN_START);
        description->set_margin_bottom(8);
        description->set_margin_top(3);

        // Configure the floating mode checkbox
        floating_check_.set_label("Start in floating mode by default");
        floating_check_.set_margin_start(8);
        floating_check_.set_margin_top(3);
        floating_check_.set_margin_bottom(3);
        floating_check_.set_active(Core::get_setting("floating", "0") == "1");
        floating_check_.set_can_focus(false); // Prevent tab navigation to this checkbox

        // Add a tooltip with more information
        floating_check_.set_tooltip_text("When enabled, the application will start as a floating window. \nMay not work with all tiling window managers — tested only on Hyprland for now.");

        // Create a note label for the floating mode
        auto floating_note = Gtk::manage(new Gtk::Label());
        floating_note->set_markup("<span size='small' style='italic'>Note: Full support on Hyprland, partial support on other tiling WMs</span>");
        floating_note->set_halign(Gtk::ALIGN_START);
        floating_note->set_margin_start(20);
        floating_note->set_margin_top(3);
        floating_note->set_margin_bottom(8);

        // Assemble the general settings section components
        general_settings_box_.pack_start(general_header_box_, Gtk::PACK_SHRINK);
        general_settings_box_.pack_start(*description, Gtk::PACK_SHRINK);
        general_settings_box_.pack_start(floating_check_, Gtk::PACK_SHRINK);
        general_settings_box_.pack_start(*floating_note, Gtk::PACK_SHRINK);

        // Add the assembled general settings box to the frame
        general_settings_frame_.add(general_settings_box_);

        // Add the completed frame to the main container
        main_box_.pack_start(general_settings_frame_, Gtk::PACK_SHRINK);
    }

    /**
     * @brief Create the tab order configuration section
     *
     * Creates the UI components for configuring tab order and visibility.
     */
    void SettingsTab::create_tab_order_section()
    {
        // Configure the frame and container for the tab order section
        tab_order_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        tab_order_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        tab_order_box_.set_spacing(8);
        tab_order_box_.set_margin_start(10);
        tab_order_box_.set_margin_end(10);
        tab_order_box_.set_margin_top(10);
        tab_order_box_.set_margin_bottom(10);

        // Configure the header for the tab order section
        tab_order_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        tab_order_header_box_.set_spacing(8);

        // Add settings icon and title label to the header
        tab_order_icon_.set_from_icon_name("view-list-symbolic", Gtk::ICON_SIZE_LARGE_TOOLBAR);
        tab_order_label_.set_markup("<span size='large'><b>Tab Settings</b></span>");
        tab_order_label_.set_halign(Gtk::ALIGN_START);
        tab_order_label_.set_valign(Gtk::ALIGN_CENTER);
        tab_order_label_.set_margin_start(10);

        tab_order_header_box_.pack_start(tab_order_icon_, Gtk::PACK_SHRINK);
        tab_order_header_box_.pack_start(tab_order_label_, Gtk::PACK_SHRINK);

        // Add descriptive text explaining the settings
        Gtk::Label *description = Gtk::manage(new Gtk::Label());
        description->set_markup("Configure which tabs are visible and their order:");
        description->set_halign(Gtk::ALIGN_START);
        description->set_margin_bottom(8);
        description->set_margin_top(3);

        // Configure the container for the list of tabs
        tab_list_box_.set_margin_start(8);
        tab_list_box_.set_margin_end(8);
        tab_list_box_.set_margin_top(3);
        tab_list_box_.set_margin_bottom(3);

        // Assemble the tab order section components
        tab_order_box_.pack_start(tab_order_header_box_, Gtk::PACK_SHRINK);
        tab_order_box_.pack_start(*description, Gtk::PACK_SHRINK);
        tab_order_box_.pack_start(tab_list_box_, Gtk::PACK_SHRINK);

        // Add the assembled tab order box to the frame
        tab_order_frame_.add(tab_order_box_);

        // Add the completed frame to the main container
        main_box_.pack_start(tab_order_frame_, Gtk::PACK_SHRINK);
    }

    /**
     * @brief Update the tab list display
     *
     * Updates the list of tabs based on the current settings.
     * Creates a row for each tab with controls for enabling/disabling
     * and changing the order.
     */
    void SettingsTab::update_tab_list()
    {
        // Remove and clear all existing tab rows
        for (auto &row : tab_rows_)
        {
            tab_list_box_.remove(row->row_box);
        }
        tab_rows_.clear();

        // Get the current list of tabs from settings
        auto tabs = settings_->get_all_tabs();

        // Create a UI row for each tab in the settings
        for (const auto &tab : tabs)
        {
            auto row = std::make_unique<TabRow>();
            row->id = tab.id;

            // Configure the horizontal container for this tab's row
            row->row_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
            row->row_box.set_spacing(8);
            row->row_box.set_margin_bottom(4);
            row->row_box.set_margin_top(4);
            row->row_box.set_margin_start(3);
            row->row_box.set_margin_end(3);

            // Create the checkbox for enabling/disabling the tab
            row->enabled_check.set_active(tab.enabled);
            row->enabled_check.signal_toggled().connect(
                [this, id = tab.id]()
                { on_tab_enabled_toggled(id); });
            row->enabled_check.set_can_focus(false); // Prevent tab navigation to this checkbox

            // Create the tab name label with its icon
            auto icon = Gtk::manage(new Gtk::Image());
            icon->set_from_icon_name(tab.icon_name, Gtk::ICON_SIZE_MENU);
            row->name_label.set_text(tab.name);
            row->name_label.set_xalign(0.0);

            // Create the buttons for changing tab order
            row->up_button.set_image_from_icon_name("go-up-symbolic", Gtk::ICON_SIZE_BUTTON);
            row->up_button.set_tooltip_text("Move up");
            row->up_button.signal_clicked().connect(sigc::mem_fun(*this, &SettingsTab::on_move_up_clicked));
            row->up_button.set_can_focus(false); // Prevent tab navigation to this button

            row->down_button.set_image_from_icon_name("go-down-symbolic", Gtk::ICON_SIZE_BUTTON);
            row->down_button.set_tooltip_text("Move down");
            row->down_button.signal_clicked().connect(sigc::mem_fun(*this, &SettingsTab::on_move_down_clicked));
            row->down_button.set_can_focus(false); // Prevent tab navigation to this button

            // Assemble all components into the row
            row->row_box.pack_start(row->enabled_check, Gtk::PACK_SHRINK);
            row->row_box.pack_start(*icon, Gtk::PACK_SHRINK);
            row->row_box.pack_start(row->name_label, Gtk::PACK_EXPAND_WIDGET);
            row->row_box.pack_end(row->down_button, Gtk::PACK_SHRINK);
            row->row_box.pack_end(row->up_button, Gtk::PACK_SHRINK);

            // Add the completed row to the tab list
            tab_list_box_.pack_start(row->row_box, Gtk::PACK_SHRINK);

            // Store the row for later reference
            tab_rows_.push_back(std::move(row));
        }

        // Disable up button for first tab and down button for last tab
        if (!tab_rows_.empty())
        {
            tab_rows_.front()->up_button.set_sensitive(false);
            tab_rows_.back()->down_button.set_sensitive(false);
        }

        show_all_children();
    }

    /**
     * @brief Handler for move up button clicks
     *
     * Moves the selected tab up in the order.
     */
    void SettingsTab::on_move_up_clicked()
    {
        // Determine which tab's up button was clicked
        for (size_t i = 0; i < tab_rows_.size(); ++i)
        {
            if (tab_rows_[i]->up_button.has_focus())
            {
                // Attempt to move this tab up in the order
                if (settings_->move_tab_up(tab_rows_[i]->id))
                {
                    // Save settings and update the UI immediately
                    settings_->save();
                    update_tab_list();
                }
                break;
            }
        }
    }

    /**
     * @brief Handler for move down button clicks
     *
     * Moves the selected tab down in the order.
     */
    void SettingsTab::on_move_down_clicked()
    {
        // Determine which tab's down button was clicked
        for (size_t i = 0; i < tab_rows_.size(); ++i)
        {
            if (tab_rows_[i]->down_button.has_focus())
            {
                // Attempt to move this tab down in the order
                if (settings_->move_tab_down(tab_rows_[i]->id))
                {
                    // Save settings and update the UI immediately
                    settings_->save();
                    update_tab_list();
                }
                break;
            }
        }
    }

    /**
     * @brief Handler for tab enabled checkbox toggles
     * @param tab_id ID of the tab whose enabled state changed
     *
     * Updates the enabled state of a tab when its checkbox is toggled.
     */
    void SettingsTab::on_tab_enabled_toggled(const std::string &tab_id)
    {
        // Find the row corresponding to this tab ID
        for (const auto &row : tab_rows_)
        {
            if (row->id == tab_id)
            {
                bool enabled = row->enabled_check.get_active();
                // Update the tab's enabled state in settings
                settings_->set_tab_enabled(tab_id, enabled);
                // Save settings to disk immediately
                settings_->save();
                break;
            }
        }
    }

    /**
     * @brief Handler for save button clicks
     *
     * Saves the settings and restarts the application to apply changes.
     */
    void SettingsTab::on_save_clicked()
    {

        // Save tab settings to disk
        settings_->save();

        // Save general settings to disk
        std::string config_path = "/home/felipe/.config/ultimate-control/general.conf";
        std::ofstream outfile(config_path);
        if (outfile.is_open())
        {
            // Read existing settings first
            std::map<std::string, std::string> settings;
            std::ifstream infile(config_path);
            if (infile.is_open())
            {
                std::string key, value;
                while (infile >> key >> value)
                {
                    settings[key] = value;
                }
                infile.close();
            }

            // Update with new floating setting
            settings["floating"] = floating_check_.get_active() ? "1" : "0";

            // Write all settings back
            for (const auto &pair : settings)
            {
                outfile << pair.first << " " << pair.second << "\n";
            }
            outfile.close();
        }

        // Show a message dialog informing the user about the restart
        Gtk::MessageDialog dialog(*dynamic_cast<Gtk::Window *>(get_toplevel()),
                                  "Settings saved. The application will now restart to apply changes.",
                                  false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        dialog.run();

        // Get the absolute path to the current executable for restart
        char exe_path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len != -1)
        {
            exe_path[len] = '\0';

            // Log the restart attempt
            std::cout << "Restarting application: " << exe_path << std::endl;

            // Use execl to replace the current process with a new instance
            // This is cleaner than using system() as it doesn't create a new process hierarchy
            execl(exe_path, exe_path, nullptr);

            // If execl fails (which it shouldn't if we reach this point), log the error
            std::cerr << "Failed to restart application: " << strerror(errno) << std::endl;
        }
        else
        {
            std::cerr << "Failed to get executable path: " << strerror(errno) << std::endl;
        }

        // As a fallback if we can't restart automatically,
        // exit the application so the user can restart it manually
        std::exit(0);
    }

    /**
     * @brief Set the callback for settings changes
     * @param callback Function to call when settings are changed
     */
    void SettingsTab::set_settings_changed_callback(SettingsChangedCallback callback)
    {
        settings_changed_callback_ = callback;
    }

    /**
     * @brief Get the tab settings object
     * @return Shared pointer to the TabSettings object
     */
    std::shared_ptr<TabSettings> SettingsTab::get_tab_settings() const
    {
        return settings_;
    }

} // namespace Settings
