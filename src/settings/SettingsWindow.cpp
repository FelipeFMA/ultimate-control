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

namespace Settings {

/**
 * @brief Constructor for the settings window
 *
 * Initializes the settings manager and creates the UI components.
 */
SettingsWindow::SettingsWindow()
: settings_(std::make_shared<TabSettings>()),           // Create settings manager
  main_box_(Gtk::ORIENTATION_VERTICAL),                 // Main container
  content_box_(Gtk::ORIENTATION_VERTICAL, 10),          // Content container with 10px spacing
  general_settings_box_(Gtk::ORIENTATION_VERTICAL, 8),  // General settings section with 8px spacing
  general_header_box_(Gtk::ORIENTATION_HORIZONTAL, 8),  // General header with 8px spacing
  tab_order_box_(Gtk::ORIENTATION_VERTICAL, 8),        // Tab order section with 8px spacing
  tab_order_header_box_(Gtk::ORIENTATION_HORIZONTAL, 8), // Tab order header with 8px spacing
  tab_list_box_(Gtk::ORIENTATION_VERTICAL, 5),          // Tab list with 5px spacing
  buttons_box_(Gtk::ORIENTATION_HORIZONTAL, 10)         // Buttons container with 10px spacing
{
    // Set up the window properties
    set_title("Ultimate Control Settings");
    set_default_size(500, 400);
    set_position(Gtk::WIN_POS_CENTER);
    set_border_width(10);

    // Set up the main container
    add(main_box_);

    // Add a scrolled window to contain all settings
    scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    scrolled_window_.set_shadow_type(Gtk::SHADOW_NONE);
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

    // Create and configure the buttons
    save_button_.set_label("Save Settings");
    save_button_.set_image_from_icon_name("document-save-symbolic", Gtk::ICON_SIZE_BUTTON);
    save_button_.set_always_show_image(true);
    save_button_.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_save_clicked));

    close_button_.set_label("Close");
    close_button_.set_image_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_BUTTON);
    close_button_.set_always_show_image(true);
    close_button_.signal_clicked().connect(sigc::mem_fun(*this, &SettingsWindow::on_close_clicked));

    buttons_box_.set_halign(Gtk::ALIGN_END);
    buttons_box_.pack_start(close_button_, Gtk::PACK_SHRINK);
    buttons_box_.pack_start(save_button_, Gtk::PACK_SHRINK);

    content_box_.pack_start(buttons_box_, Gtk::PACK_SHRINK);

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
void SettingsWindow::set_settings_changed_callback(SettingsChangedCallback callback) {
    settings_changed_callback_ = callback;
}

/**
 * @brief Get the tab settings object
 * @return Shared pointer to the TabSettings object
 *
 * Returns the TabSettings object that manages tab configuration.
 */
std::shared_ptr<TabSettings> SettingsWindow::get_tab_settings() const {
    return settings_;
}

/**
 * @brief Create the general settings section
 *
 * Creates the UI components for configuring general application settings.
 */
void SettingsWindow::create_general_settings_section() {
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
    Gtk::Label* description = Gtk::manage(new Gtk::Label());
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
    if (getenv("HOME")) {
        std::string config_path = std::string(getenv("HOME")) + "/.config/ultimate-control/general.conf";

        std::ifstream config_file(config_path);
        if (config_file.is_open()) {
            std::string line;
            while (std::getline(config_file, line)) {
                if (line.find("floating 1") != std::string::npos) {
                    floating_enabled = true;
                    break;
                }
            }
            config_file.close();
        }
    }
    floating_check_.set_active(floating_enabled);

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
void SettingsWindow::create_tab_order_section() {
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
    Gtk::Label* description = Gtk::manage(new Gtk::Label());
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
void SettingsWindow::update_tab_list() {
    // Remove and clear all existing tab rows
    for (auto& row : tab_rows_) {
        tab_list_box_.remove(row->row_box);
    }
    tab_rows_.clear();

    // Get the current list of tabs from settings
    auto tabs = settings_->get_all_tabs();

    // Create a UI row for each tab in the settings
    for (const auto& tab : tabs) {
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
            [this, id = tab.id]() { on_tab_enabled_toggled(id); });

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
    if (!tab_rows_.empty()) {
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
void SettingsWindow::on_move_up_clicked() {
    // Determine which tab's up button was clicked
    for (size_t i = 0; i < tab_rows_.size(); ++i) {
        if (tab_rows_[i]->up_button.has_focus()) {
            // Attempt to move this tab up in the order
            if (settings_->move_tab_up(tab_rows_[i]->id)) {
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
void SettingsWindow::on_move_down_clicked() {
    // Determine which tab's down button was clicked
    for (size_t i = 0; i < tab_rows_.size(); ++i) {
        if (tab_rows_[i]->down_button.has_focus()) {
            // Attempt to move this tab down in the order
            if (settings_->move_tab_down(tab_rows_[i]->id)) {
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
void SettingsWindow::on_tab_enabled_toggled(const std::string& tab_id) {
    // Find the row corresponding to this tab ID
    for (const auto& row : tab_rows_) {
        if (row->id == tab_id) {
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
void SettingsWindow::on_save_clicked() {
    // Save tab settings to disk
    settings_->save();

    // Save general settings to disk
    std::string config_path = "/home/felipe/.config/ultimate-control/general.conf";
    std::ofstream outfile(config_path);
    if (outfile.is_open()) {
        // Read existing settings first
        std::map<std::string, std::string> settings;
        std::ifstream infile(config_path);
        if (infile.is_open()) {
            std::string key, value;
            while (infile >> key >> value) {
                settings[key] = value;
            }
            infile.close();
        }

        // Update with new floating setting
        settings["floating"] = floating_check_.get_active() ? "1" : "0";

        // Write all settings back
        for (const auto& pair : settings) {
            outfile << pair.first << " " << pair.second << "\n";
        }
        outfile.close();
    }

    // Notify that settings have changed
    if (settings_changed_callback_) {
        settings_changed_callback_();
    }

    // Close the window
    hide();
}

/**
 * @brief Handler for close button clicks
 *
 * Closes the settings window without saving changes.
 */
void SettingsWindow::on_close_clicked() {
    // Simply close the window
    hide();
}

} // namespace Settings
