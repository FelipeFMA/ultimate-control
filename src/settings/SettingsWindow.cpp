/**
 * @file SettingsWindow.cpp
 * @brief Implementation of the settings window
 *
 * This file implements the SettingsWindow class which provides a user interface
 * for configuring application settings, including tab visibility and order.
 */

#include "SettingsWindow.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include <cstdlib>
#include <vector>
#include <glibmm/base64.h>           // For base64 decoding
#include <giomm/memoryinputstream.h> // For memory input stream

namespace Settings
{

    /**
     * @brief Constructor for the settings window
     * @param parent The parent window that this dialog will be modal to
     *
     * Initializes the settings manager and creates the UI components.
     * Creates a modal dialog that blocks interaction with the parent window.
     */
    SettingsWindow::SettingsWindow(Gtk::Window &parent)
        : Gtk::Dialog("Ultimate Control Settings", parent, true), // Modal dialog with title and parent
          settings_(std::make_shared<TabSettings>()),             // Create settings manager
          main_box_(Gtk::ORIENTATION_VERTICAL),                   // Main container
          content_box_(Gtk::ORIENTATION_VERTICAL, 10),            // Content container with 10px spacing
          general_settings_box_(Gtk::ORIENTATION_VERTICAL, 8),    // General settings section with 8px spacing
          general_header_box_(Gtk::ORIENTATION_HORIZONTAL, 8),    // General header with 8px spacing
          tab_order_box_(Gtk::ORIENTATION_VERTICAL, 8),           // Tab order section with 8px spacing
          tab_order_header_box_(Gtk::ORIENTATION_HORIZONTAL, 8),  // Tab order header with 8px spacing
          tab_list_box_(Gtk::ORIENTATION_VERTICAL, 5)             // Tab list with 5px spacing
    {
        // Set up the dialog properties
        set_default_size(500, 400);
        set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
        set_border_width(10);

        // Get the content area of the dialog
        auto content_area = get_content_area();

        // Add a scrolled window to contain all settings
        scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        scrolled_window_.set_shadow_type(Gtk::SHADOW_NONE);
        content_area->pack_start(main_box_, Gtk::PACK_EXPAND_WIDGET);
        main_box_.pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

        // Set up the content box inside the scrolled window with margins
        content_box_.set_margin_start(10);
        content_box_.set_margin_end(10);
        content_box_.set_margin_top(10);
        content_box_.set_margin_bottom(10);
        scrolled_window_.add(content_box_);

        // Create the general settings section
        create_general_settings_section();

        // Create the tab order configuration section
        create_tab_order_section();

        // Get the action area to customize the button layout
        Gtk::ButtonBox *action_area = get_action_area();
        action_area->set_layout(Gtk::BUTTONBOX_EDGE); // Use EDGE layout for proper spacing

        // Create and add the about button directly to the action area
        Gtk::Button *about_button = new Gtk::Button("About");
        about_button->set_tooltip_text("Show information about Ultimate Control");
        about_button->signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::show_about_dialog));

        // Create a box for the About button to match other buttons' sizing
        Gtk::Box *left_buttons = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 0);
        left_buttons->set_halign(Gtk::ALIGN_START);

        // Set minimum width to match standard buttons
        about_button->set_size_request(85, -1);

        // Add the About button to the left container
        left_buttons->pack_start(*about_button, false, false, 0);

        // Add the left button container to the action area
        action_area->pack_start(*left_buttons, false, false, 0);

        // Create a box to hold Cancel and Save buttons together on the right
        Gtk::Box *right_buttons = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6);
        right_buttons->set_halign(Gtk::ALIGN_END);

        // Add standard dialog buttons
        Gtk::Button *cancel_button = new Gtk::Button("_Cancel");
        cancel_button->set_use_underline(true);
        cancel_button->signal_clicked().connect([this]()
                                                { response(Gtk::RESPONSE_CANCEL); });
        cancel_button->set_size_request(85, -1); // Make same width as About button

        Gtk::Button *save_button = new Gtk::Button("_Save");
        save_button->set_use_underline(true);
        save_button->signal_clicked().connect([this]()
                                              { response(Gtk::RESPONSE_APPLY); });
        save_button->set_size_request(85, -1); // Make same width as About button

        // Add buttons to the right box
        right_buttons->pack_end(*save_button, false, false, 0);
        right_buttons->pack_end(*cancel_button, false, false, 0);

        // Add the right buttons container to the action area
        action_area->pack_end(*right_buttons, false, false, 0);

        // Make all widgets managed by their containers
        about_button->set_can_default(false);
        cancel_button->set_can_default(true);
        save_button->set_can_default(true);
        save_button->grab_default();

        // Connect the response signal
        signal_response().connect(sigc::mem_fun(*this, &SettingsWindow::on_response));

        // Set the default response
        set_default_response(Gtk::RESPONSE_APPLY);

        // Initialize the tab list with current settings
        update_tab_list();

        show_all_children();
        std::cout << "Settings window created!" << std::endl;
    }

    /**
     * @brief Destructor for the settings window
     */
    SettingsWindow::~SettingsWindow() = default;

    /**
     * @brief Set the callback for settings changes
     * @param callback Function to call when settings are changed
     *
     * Sets the callback function that will be called when settings are changed.
     */
    void SettingsWindow::set_settings_changed_callback(SettingsChangedCallback callback)
    {
        settings_changed_callback_ = callback;
    }

    /**
     * @brief Get the tab settings object
     * @return Shared pointer to the TabSettings object
     *
     * Returns the TabSettings object that manages tab configuration.
     */
    std::shared_ptr<TabSettings> SettingsWindow::get_tab_settings() const
    {
        return settings_;
    }

    /**
     * @brief Create the general settings section
     *
     * Creates the UI components for configuring general application settings.
     */
    void SettingsWindow::create_general_settings_section()
    {
        // Configure the frame for the general settings section
        general_settings_frame_.set_label("");
        general_settings_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        general_settings_frame_.set_margin_bottom(10);

        // Configure the general settings box with padding
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

        general_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        general_header_box_.set_spacing(8);
        general_header_box_.set_margin_bottom(5);

        general_header_box_.pack_start(general_icon_, Gtk::PACK_SHRINK);
        general_header_box_.pack_start(general_label_, Gtk::PACK_SHRINK);

        // Add descriptive text explaining the settings
        Gtk::Label *description = Gtk::manage(new Gtk::Label());
        description->set_markup("Configure general application settings:");
        description->set_halign(Gtk::ALIGN_START);
        description->set_margin_bottom(8);
        description->set_margin_top(3);

        // Create the floating mode checkbox
        floating_check_.set_label("Start as floating window on tiling window managers");
        floating_check_.set_margin_start(8);
        floating_check_.set_margin_top(3);
        floating_check_.set_margin_bottom(3);

        // Check if floating mode is enabled in settings
        bool floating_enabled = false;
        if (getenv("HOME"))
        {
            std::string config_path = std::string(getenv("HOME")) + "/.config/ultimate-control/general.conf";

            std::ifstream config_file(config_path);
            if (config_file.is_open())
            {
                std::string line;
                while (std::getline(config_file, line))
                {
                    if (line.find("floating 1") != std::string::npos)
                    {
                        floating_enabled = true;
                        break;
                    }
                }
                config_file.close();
            }
        }
        floating_check_.set_active(floating_enabled);

        // About button is now added to the action area

        // Add all components to the general settings box
        general_settings_box_.pack_start(general_header_box_, Gtk::PACK_SHRINK);
        general_settings_box_.pack_start(*description, Gtk::PACK_SHRINK);
        general_settings_box_.pack_start(floating_check_, Gtk::PACK_SHRINK);

        // Add the general settings box to the frame
        general_settings_frame_.add(general_settings_box_);

        // Add the frame to the main box
        content_box_.pack_start(general_settings_frame_, Gtk::PACK_SHRINK);
    }

    /**
     * @brief Create the tab order configuration section
     *
     * Creates the UI components for configuring tab order and visibility.
     */
    void SettingsWindow::create_tab_order_section()
    {
        // Configure the frame for the tab order section
        tab_order_frame_.set_label("");
        tab_order_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        tab_order_frame_.set_margin_bottom(10);

        // Configure the tab order box with padding
        tab_order_box_.set_spacing(8);
        tab_order_box_.set_margin_start(10);
        tab_order_box_.set_margin_end(10);
        tab_order_box_.set_margin_top(10);
        tab_order_box_.set_margin_bottom(10);

        // Add settings icon and title label to the header
        tab_order_icon_.set_from_icon_name("view-list-symbolic", Gtk::ICON_SIZE_LARGE_TOOLBAR);
        tab_order_label_.set_markup("<span size='large'><b>Tab Settings</b></span>");
        tab_order_label_.set_halign(Gtk::ALIGN_START);
        tab_order_label_.set_valign(Gtk::ALIGN_CENTER);
        tab_order_label_.set_margin_start(10);

        tab_order_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        tab_order_header_box_.set_spacing(8);
        tab_order_header_box_.set_margin_bottom(5);

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
        tab_list_box_.set_margin_top(5);
        tab_list_box_.set_margin_bottom(5);
        tab_list_box_.set_spacing(5);

        // Add all components to the tab order box
        tab_order_box_.pack_start(tab_order_header_box_, Gtk::PACK_SHRINK);
        tab_order_box_.pack_start(*description, Gtk::PACK_SHRINK);
        tab_order_box_.pack_start(tab_list_box_, Gtk::PACK_SHRINK);

        // Add the tab order box to the frame
        tab_order_frame_.add(tab_order_box_);

        // Add the frame to the main box
        content_box_.pack_start(tab_order_frame_, Gtk::PACK_EXPAND_WIDGET);
    }

    /**
     * @brief Update the tab list display
     *
     * Updates the list of tabs based on the current settings.
     * Creates a row for each tab with controls for enabling/disabling
     * and changing the order.
     */
    void SettingsWindow::update_tab_list()
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

            // Create the label showing the tab name
            row->name_label.set_text(tab.name);
            row->name_label.set_halign(Gtk::ALIGN_START);

            // Create an icon for the tab
            auto icon = Gtk::manage(new Gtk::Image());
            icon->set_from_icon_name(tab.icon_name, Gtk::ICON_SIZE_BUTTON);

            // Create buttons for moving the tab up and down in the order
            row->up_button.set_image_from_icon_name("go-up-symbolic", Gtk::ICON_SIZE_BUTTON);
            row->up_button.set_tooltip_text("Move up");
            row->up_button.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_move_up_clicked));

            row->down_button.set_image_from_icon_name("go-down-symbolic", Gtk::ICON_SIZE_BUTTON);
            row->down_button.set_tooltip_text("Move down");
            row->down_button.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_move_down_clicked));

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
    void SettingsWindow::on_move_up_clicked()
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
    void SettingsWindow::on_move_down_clicked()
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
    void SettingsWindow::on_tab_enabled_toggled(const std::string &tab_id)
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
     * @brief Handler for dialog response signals
     * @param response_id The ID of the response (e.g., RESPONSE_CANCEL, RESPONSE_APPLY)
     *
     * Handles the dialog response signals. If RESPONSE_APPLY, saves the settings
     * and restarts the application to apply changes. Otherwise, just closes the dialog.
     */
    void SettingsWindow::on_response(int response_id)
    {
        if (response_id == Gtk::RESPONSE_APPLY || response_id == GTK_RESPONSE_APPLY)
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

            // Notify that settings have changed
            if (settings_changed_callback_)
            {
                settings_changed_callback_();
            }
        }

        // Close the dialog
        hide();
    }

    // Base64 encoded SVG logo with catppuccin colors
    static const std::string LOGO_SVG_BASE64 =
        "PHN2ZyB2aWV3Qm94PSIwIDAgMzAwIDMwMCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KICA8IS0tIEVuZ3JlbmFnZW0gY29tIGZvcm1hdG8gdHJhZGljaW9uYWwgZSB0cmFwZXrDs2lkZXMgcGFyYSBvcyBkZW50ZXMgLS0+CiAgPGcgdHJhbnNmb3JtPSJ0cmFuc2xhdGUoMTUwLCAxNTApIj4KICAgIDwhLS0gQmFzZSBjaXJjdWxhciBkYSBlbmdyZW5hZ2VtIC0tPgogICAgPGNpcmNsZSBjeD0iMCIgY3k9IjAiIHI9Ijc1IiBmaWxsPSIjNDU0NzVhIiBzdHJva2U9IiM1ODViNzAiIHN0cm9rZS13aWR0aD0iMiIvPgogICAgCiAgICA8IS0tIERlbnRlcyBkYSBlbmdyZW5hZ2VtIC0gMTYgZGVudGVzIHRyYXBlem9pZGFpcyBtYWlzIHJlYWxpc3RhcyAtLT4KICAgIDxnIGZpbGw9IiMzMTMyNDQiIHN0cm9rZT0iIzU4NWI3MCIgc3Ryb2tlLXdpZHRoPSIxLjUiPgogICAgICA8IS0tIERlbnRlIDEgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgwKSIvPgogICAgICA8IS0tIERlbnRlIDIgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgyMi41KSIvPgogICAgICA8IS0tIERlbnRlIDMgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSg0NSkiLz4KICAgICAgPCEtLSBEZW50ZSA0IC0tPgogICAgICA8cGF0aCBkPSJNIC0xMCwwIEwgLTEwLC03NSBMIC0yNSwtOTAgTCAtNDAsLTc1IEwgLTQwLDAgWiIgdHJhbnNmb3JtPSJyb3RhdGUoNjcuNSkiLz4KICAgICAgPCEtLSBEZW50ZSA1IC0tPgogICAgICA8cGF0aCBkPSJNIC0xMCwwIEwgLTEwLC03NSBMIC0yNSwtOTAgTCAtNDAsLTc1IEwgLTQwLDAgWiIgdHJhbnNmb3JtPSJyb3RhdGUoOTApIi8+CiAgICAgIDwhLS0gRGVudGUgNiAtLT4KICAgICAgPHBhdGggZD0iTSAtMTAsMCBMIC0xMCwtNzUgTCAtMjUsLTkwIEwgLTQwLC03NSBMIC00MCwwIFoiIHRyYW5zZm9ybT0icm90YXRlKDExMi41KSIvPgogICAgICA8IS0tIERlbnRlIDcgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgxMzUpIi8+CiAgICAgIDwhLS0gRGVudGUgOCAtLT4KICAgICAgPHBhdGggZD0iTSAtMTAsMCBMIC0xMCwtNzUgTCAtMjUsLTkwIEwgLTQwLC03NSBMIC00MCwwIFoiIHRyYW5zZm9ybT0icm90YXRlKDE1Ny41KSIvPgogICAgICA8IS0tIERlbnRlIDkgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgxODApIi8+CiAgICAgIDwhLS0gRGVudGUgMTAgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgyMDIuNSkiLz4KICAgICAgPCEtLSBEZW50ZSAxMSAtLT4KICAgICAgPHBhdGggZD0iTSAtMTAsMCBMIC0xMCwtNzUgTCAtMjUsLTkwIEwgLTQwLC03NSBMIC00MCwwIFoiIHRyYW5zZm9ybT0icm90YXRlKDIyNSkiLz4KICAgICAgPCEtLSBEZW50ZSAxMiAtLT4KICAgICAgPHBhdGggZD0iTSAtMTAsMCBMIC0xMCwtNzUgTCAtMjUsLTkwIEwgLTQwLC03NSBMIC00MCwwIFoiIHRyYW5zZm9ybT0icm90YXRlKDI0Ny41KSIvPgogICAgICA8IS0tIERlbnRlIDEzIC0tPgogICAgICA8cGF0aCBkPSJNIC0xMCwwIEwgLTEwLC03NSBMIC0yNSwtOTAgTCAtNDAsLTc1IEwgLTQwLDAgWiIgdHJhbnNmb3JtPSJyb3RhdGUoMjcwKSIvPgogICAgICA8IS0tIERlbnRlIDE0IC0tPgogICAgICA8cGF0aCBkPSJNIC0xMCwwIEwgLTEwLC03NSBMIC0yNSwtOTAgTCAtNDAsLTc1IEwgLTQwLDAgWiIgdHJhbnNmb3JtPSJyb3RhdGUoMjkyLjUpIi8+CiAgICAgIDwhLS0gRGVudGUgMTUgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgzMTUpIi8+CiAgICAgIDwhLS0gRGVudGUgMTYgLS0+CiAgICAgIDxwYXRoIGQ9Ik0gLTEwLDAgTCAtMTAsLTc1IEwgLTI1LC05MCBMIC00MCwtNzUgTCAtNDAsMCBaIiB0cmFuc2Zvcm09InJvdGF0ZSgzMzcuNSkiLz4KICAgIDwvZz4KICAgIAogICAgPCEtLSBBbmVsIGludGVybm8gcGFyYSBkYXIgcHJvZnVuZGlkYWRlIC0tPgogICAgPGNpcmNsZSBjeD0iMCIgY3k9IjAiIHI9IjYwIiBmaWxsPSIjNTg1YjcwIiBzdHJva2U9IiM1ODViNzAiIHN0cm9rZS13aWR0aD0iMS41Ii8+CiAgPC9nPgogIAogIDwhLS0gTGV0cmEgVSAodmVyZGUgLSBDYXRwcHVjY2luIE1vY2hhIEdyZWVuKSAtLT4KICA8cGF0aCBkPSJNMTI1LDEyMCBMMTI1LDE3MCBRMTI1LDE4NSAxNDAsMTg1IEwxNjAsMTg1IFExNzUsMTg1IDE3NSwxNzAgTDE3NSwxMjAiIGZpbGw9Im5vbmUiIHN0cm9rZT0iI2E2ZTNhMSIgc3Ryb2tlLXdpZHRoPSIxNSIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIi8+CiAgCiAgPCEtLSBMZXRyYSBDICh2ZXJtZWxoYSAtIENhdHBwdWNjaW4gTW9jaGEgUmVkKSAtLT4KICA8cGF0aCBkPSJNMTYwLDEzMCBRMTYwLDExNSAxNTAsMTE1IFExNDAsMTE1IDE0MCwxMzAgTDE0MCwxNjAgUTE0MCwxNzUgMTUwLDE3NSBRMTYwLDE3NSAxNjAsMTYwIiBmaWxsPSJub25lIiBzdHJva2U9IiNmMzhiYTgiIHN0cm9rZS13aWR0aD0iMTUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPgogIAogIDwhLS0gQnVyYWNvIGNlbnRyYWwgY29tIGFwYXLDqm5jaWEgbWV0w6FsaWNhIC0tPgogIDxjaXJjbGUgY3g9IjE1MCIgY3k9IjE1MCIgcj0iMTUiIGZpbGw9IiNmNWUwZGMiIHN0cm9rZT0iIzU4NWI3MCIgc3Ryb2tlLXdpZHRoPSIyIi8+CiAgPGNpcmNsZSBjeD0iMTUwIiBjeT0iMTUwIiByPSI3IiBmaWxsPSIjYTZhZGM4Ii8+Cjwvc3ZnPgo=";

    /**
     * @brief Create and open the about dialog
     *
     * Opens a dialog showing information about the application.
     */
    void SettingsWindow::show_about_dialog()
    {
        // Create an about dialog
        Gtk::AboutDialog about_dialog;

        // Set dialog properties
        about_dialog.set_transient_for(*this);
        about_dialog.set_modal(true);

        // Set application information
        about_dialog.set_program_name("Ultimate Control");
        about_dialog.set_comments("A GTK control panel for Linux written in C++");
        about_dialog.set_website("https://github.com/FelipeFMA/ultimate-control");
        about_dialog.set_website_label("GitHub Repository");
        about_dialog.set_copyright("Made with ❤️ by Felipe Avelar");
        about_dialog.set_license_type(Gtk::LICENSE_GPL_3_0);

        // Load the logo from embedded base64 string
        try
        {
            // Decode the base64 SVG data
            std::string svg_data = Glib::Base64::decode(LOGO_SVG_BASE64);

            // Create a stream from the decoded data
            Glib::RefPtr<Gio::MemoryInputStream> stream =
                Gio::MemoryInputStream::create();
            stream->add_data(svg_data);

            // Load the SVG from the memory stream
            Glib::RefPtr<Gdk::Pixbuf> logo =
                Gdk::Pixbuf::create_from_stream_at_scale(stream, 200, 200, true);

            // Set the logo
            about_dialog.set_logo(logo);
        }
        catch (const Glib::Error &ex)
        {
            std::cerr << "Error loading embedded logo: " << ex.what() << std::endl;
            // Fallback to a default icon if loading fails
            about_dialog.set_logo_icon_name("help-about");
        }

        // Show the dialog
        about_dialog.run();
    }

} // namespace Settings
