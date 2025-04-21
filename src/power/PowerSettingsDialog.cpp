/**
 * @file PowerSettingsDialog.cpp
 * @brief Implementation of the power settings dialog
 *
 * This file implements the PowerSettingsDialog class which provides a user interface
 * for configuring the commands executed by power operations like shutdown,
 * reboot, suspend, etc.
 */

#include "PowerSettingsDialog.hpp"
#include <iostream>

namespace Power
{

    /**
     * @brief Constructor for the power settings dialog
     * @param parent Parent window for the dialog
     * @param settings Shared pointer to the power settings object
     *
     * Creates a dialog for configuring power commands.
     */
    PowerSettingsDialog::PowerSettingsDialog(Gtk::Window &parent, std::shared_ptr<PowerSettings> settings)
        : Gtk::Dialog("Power Command Settings", parent, true), // Modal dialog with title
          settings_(settings)                                  // Store settings object
    {
        // Set initial dialog size
        set_default_size(500, 300);

        // Get the content area and configure its margins and spacing
        auto content = get_content_area();
        content->set_margin_top(20);
        content->set_margin_bottom(20);
        content->set_margin_start(20);
        content->set_margin_end(20);
        content->set_spacing(15);

        // Create and configure the header with icon and title
        auto header_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 15);
        auto header_icon = Gtk::make_managed<Gtk::Image>();
        header_icon->set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_DIALOG);
        header_box->pack_start(*header_icon, Gtk::PACK_SHRINK);

        auto header_label = Gtk::make_managed<Gtk::Label>();
        header_label->set_markup("<span size='large' weight='bold'>Power Command Settings</span>");
        header_label->set_halign(Gtk::ALIGN_START);
        header_label->set_valign(Gtk::ALIGN_CENTER);
        header_box->pack_start(*header_label, Gtk::PACK_EXPAND_WIDGET);

        content->pack_start(*header_box, Gtk::PACK_SHRINK);
        // Create the notebook for tabbed settings
        notebook_ = Gtk::manage(new Gtk::Notebook());
        notebook_->set_can_focus(false); // Prevent tab navigation to the notebook

        // --- Commands Tab ---
        auto commands_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);

        // Description for commands
        auto description = Gtk::make_managed<Gtk::Label>();
        description->set_markup("Configure the commands that will be executed when pressing the power buttons:");
        description->set_halign(Gtk::ALIGN_START);
        description->set_line_wrap(true);
        description->set_margin_top(5);
        description->set_margin_bottom(15);
        commands_box->pack_start(*description, Gtk::PACK_SHRINK);

        // Command grid
        auto grid = Gtk::make_managed<Gtk::Grid>();
        grid->set_row_spacing(15);
        grid->set_column_spacing(15);
        grid->set_margin_top(10);
        grid->set_margin_bottom(15);
        grid->set_margin_start(5);
        grid->set_margin_end(5);

        int row = 0;
        command_entries_["shutdown"].label.set_text("Shutdown command:");
        command_entries_["shutdown"].label.set_halign(Gtk::ALIGN_START);
        command_entries_["shutdown"].label.set_margin_start(5);
        command_entries_["shutdown"].entry.set_hexpand(true);
        command_entries_["shutdown"].entry.set_margin_end(5);
        grid->attach(command_entries_["shutdown"].label, 0, row, 1, 1);
        grid->attach(command_entries_["shutdown"].entry, 1, row, 1, 1);
        row++;

        command_entries_["reboot"].label.set_text("Reboot command:");
        command_entries_["reboot"].label.set_halign(Gtk::ALIGN_START);
        command_entries_["reboot"].label.set_margin_start(5);
        command_entries_["reboot"].entry.set_hexpand(true);
        command_entries_["reboot"].entry.set_margin_end(5);
        grid->attach(command_entries_["reboot"].label, 0, row, 1, 1);
        grid->attach(command_entries_["reboot"].entry, 1, row, 1, 1);
        row++;

        command_entries_["suspend"].label.set_text("Suspend command:");
        command_entries_["suspend"].label.set_halign(Gtk::ALIGN_START);
        command_entries_["suspend"].label.set_margin_start(5);
        command_entries_["suspend"].entry.set_hexpand(true);
        command_entries_["suspend"].entry.set_margin_end(5);
        grid->attach(command_entries_["suspend"].label, 0, row, 1, 1);
        grid->attach(command_entries_["suspend"].entry, 1, row, 1, 1);
        row++;

        command_entries_["hibernate"].label.set_text("Hibernate command:");
        command_entries_["hibernate"].label.set_halign(Gtk::ALIGN_START);
        command_entries_["hibernate"].label.set_margin_start(5);
        command_entries_["hibernate"].entry.set_hexpand(true);
        command_entries_["hibernate"].entry.set_margin_end(5);
        grid->attach(command_entries_["hibernate"].label, 0, row, 1, 1);
        grid->attach(command_entries_["hibernate"].entry, 1, row, 1, 1);
        row++;

        command_entries_["lock"].label.set_text("Lock screen command:");
        command_entries_["lock"].label.set_halign(Gtk::ALIGN_START);
        command_entries_["lock"].label.set_margin_start(5);
        command_entries_["lock"].entry.set_hexpand(true);
        command_entries_["lock"].entry.set_margin_end(5);
        grid->attach(command_entries_["lock"].label, 0, row, 1, 1);
        grid->attach(command_entries_["lock"].entry, 1, row, 1, 1);

        commands_box->pack_start(*grid, Gtk::PACK_EXPAND_WIDGET);

        // Helper to create a tab label with icon and text, matching main window
        auto create_tab_label = [](const std::string &icon_name, const std::string &label_text) -> Gtk::EventBox *
        {
            auto event_box = Gtk::make_managed<Gtk::EventBox>();
            auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
            auto icon = Gtk::make_managed<Gtk::Image>();
            icon->set_from_icon_name(icon_name, Gtk::ICON_SIZE_SMALL_TOOLBAR);
            auto label = Gtk::make_managed<Gtk::Label>(label_text);
            box->pack_start(*icon, Gtk::PACK_SHRINK);
            box->pack_start(*label, Gtk::PACK_SHRINK);
            event_box->add(*box);
            event_box->show_all();
            return event_box;
        };

        // Create tab label for Commands
        auto commands_event_box = create_tab_label("utilities-terminal-symbolic", "Commands");
        notebook_->append_page(*commands_box, *commands_event_box);

        // --- Keybinds Tab ---
        auto keybinds_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);

        // Description for keybinds
        auto keybinds_desc = Gtk::make_managed<Gtk::Label>();
        keybinds_desc->set_markup("Configure the keyboard shortcuts for each power action (e.g., <i>Ctrl+Alt+S</i>). Leave blank to disable a keybind.");
        keybinds_desc->set_halign(Gtk::ALIGN_START);
        keybinds_desc->set_line_wrap(true);
        keybinds_desc->set_margin_bottom(10);
        keybinds_box->pack_start(*keybinds_desc, Gtk::PACK_SHRINK);

        // Keybinds grid
        auto keybinds_grid = Gtk::make_managed<Gtk::Grid>();
        keybinds_grid->set_row_spacing(10);
        keybinds_grid->set_column_spacing(15);
        keybinds_grid->set_margin_start(5);
        keybinds_grid->set_margin_end(5);

        // Show keybind hints toggle
        show_keybind_hints_check_ = Gtk::make_managed<Gtk::CheckButton>("Show keybind hints on buttons");
        show_keybind_hints_check_->set_active(settings_->get_show_keybind_hints());
        show_keybind_hints_check_->set_margin_bottom(10);
        show_keybind_hints_check_->set_can_focus(false); // Prevent tab navigation to this checkbox
        keybinds_box->pack_start(*show_keybind_hints_check_, Gtk::PACK_SHRINK);

        int keybind_row = 0;
        keybind_entries_["shutdown"].label.set_text("Shutdown keybind:");
        keybind_entries_["shutdown"].label.set_halign(Gtk::ALIGN_START);
        keybind_entries_["shutdown"].label.set_margin_start(5);
        keybind_entries_["shutdown"].entry.set_hexpand(true);
        keybind_entries_["shutdown"].entry.set_margin_end(5);
        keybinds_grid->attach(keybind_entries_["shutdown"].label, 0, keybind_row, 1, 1);
        keybinds_grid->attach(keybind_entries_["shutdown"].entry, 1, keybind_row, 1, 1);
        keybind_row++;

        keybind_entries_["reboot"].label.set_text("Reboot keybind:");
        keybind_entries_["reboot"].label.set_halign(Gtk::ALIGN_START);
        keybind_entries_["reboot"].label.set_margin_start(5);
        keybind_entries_["reboot"].entry.set_hexpand(true);
        keybind_entries_["reboot"].entry.set_margin_end(5);
        keybinds_grid->attach(keybind_entries_["reboot"].label, 0, keybind_row, 1, 1);
        keybinds_grid->attach(keybind_entries_["reboot"].entry, 1, keybind_row, 1, 1);
        keybind_row++;

        keybind_entries_["suspend"].label.set_text("Suspend keybind:");
        keybind_entries_["suspend"].label.set_halign(Gtk::ALIGN_START);
        keybind_entries_["suspend"].label.set_margin_start(5);
        keybind_entries_["suspend"].entry.set_hexpand(true);
        keybind_entries_["suspend"].entry.set_margin_end(5);
        keybinds_grid->attach(keybind_entries_["suspend"].label, 0, keybind_row, 1, 1);
        keybinds_grid->attach(keybind_entries_["suspend"].entry, 1, keybind_row, 1, 1);
        keybind_row++;

        keybind_entries_["hibernate"].label.set_text("Hibernate keybind:");
        keybind_entries_["hibernate"].label.set_halign(Gtk::ALIGN_START);
        keybind_entries_["hibernate"].label.set_margin_start(5);
        keybind_entries_["hibernate"].entry.set_hexpand(true);
        keybind_entries_["hibernate"].entry.set_margin_end(5);
        keybinds_grid->attach(keybind_entries_["hibernate"].label, 0, keybind_row, 1, 1);
        keybinds_grid->attach(keybind_entries_["hibernate"].entry, 1, keybind_row, 1, 1);
        keybind_row++;

        keybind_entries_["lock"].label.set_text("Lock screen keybind:");
        keybind_entries_["lock"].label.set_halign(Gtk::ALIGN_START);
        keybind_entries_["lock"].label.set_margin_start(5);
        keybind_entries_["lock"].entry.set_hexpand(true);
        keybind_entries_["lock"].entry.set_margin_end(5);
        keybinds_grid->attach(keybind_entries_["lock"].label, 0, keybind_row, 1, 1);
        keybinds_grid->attach(keybind_entries_["lock"].entry, 1, keybind_row, 1, 1);

        keybinds_box->pack_start(*keybinds_grid, Gtk::PACK_EXPAND_WIDGET);

        // Create tab label for Keybinds
        auto keybinds_event_box = create_tab_label("input-keyboard-symbolic", "Keybinds");
        notebook_->append_page(*keybinds_box, *keybinds_event_box);

        // Ensure all notebook children (including tab labels) are visible
        notebook_->show_all_children();

        // Add the notebook to the content area
        content->pack_start(*notebook_, Gtk::PACK_EXPAND_WIDGET);

        // Create a button box for the reset button
        auto button_box = Gtk::manage(new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL));
        button_box->set_layout(Gtk::BUTTONBOX_END);
        button_box->set_spacing(10);
        button_box->set_margin_top(5);

        // Create and configure the reset to defaults button
        reset_button_ = Gtk::manage(new Gtk::Button("_Reset to Defaults"));
        reset_button_->set_use_underline(true);
        reset_button_->signal_clicked().connect(sigc::mem_fun(*this, &PowerSettingsDialog::reset_to_defaults));
        reset_button_->set_can_focus(false); // Prevent tab navigation to this button
        button_box->pack_start(*reset_button_, Gtk::PACK_SHRINK);

        // Add the button box below the notebook
        content->pack_start(*button_box, Gtk::PACK_SHRINK);

        // Add standard dialog buttons (Cancel and Save)
        Gtk::Button *cancel_button = add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        Gtk::Button *save_button = add_button("_Save", Gtk::RESPONSE_OK);
        cancel_button->set_can_focus(false); // Prevent tab navigation to this button
        save_button->set_can_focus(false);   // Prevent tab navigation to this button
        set_default_response(Gtk::RESPONSE_OK);

        // Load current settings into the entry fields
        load_settings();

        show_all_children();
    }

    /**
     * @brief Destructor for the power settings dialog
     */
    PowerSettingsDialog::~PowerSettingsDialog() = default;

    /**
     * @brief Load settings into the dialog
     *
     * Loads the current command settings from the PowerSettings object
     * into the dialog's entry fields.
     */
    void PowerSettingsDialog::load_settings()
    {
        command_entries_["shutdown"].entry.set_text(settings_->get_command("shutdown"));
        command_entries_["shutdown"].entry.set_can_focus(false); // Prevent tab navigation
        command_entries_["reboot"].entry.set_text(settings_->get_command("reboot"));
        command_entries_["reboot"].entry.set_can_focus(false); // Prevent tab navigation
        command_entries_["suspend"].entry.set_text(settings_->get_command("suspend"));
        command_entries_["suspend"].entry.set_can_focus(false); // Prevent tab navigation
        command_entries_["hibernate"].entry.set_text(settings_->get_command("hibernate"));
        command_entries_["hibernate"].entry.set_can_focus(false); // Prevent tab navigation
        command_entries_["lock"].entry.set_text(settings_->get_command("lock"));
        command_entries_["lock"].entry.set_can_focus(false); // Prevent tab navigation

        keybind_entries_["shutdown"].entry.set_text(settings_->get_keybind("shutdown"));
        keybind_entries_["shutdown"].entry.set_can_focus(false); // Prevent tab navigation
        keybind_entries_["reboot"].entry.set_text(settings_->get_keybind("reboot"));
        keybind_entries_["reboot"].entry.set_can_focus(false); // Prevent tab navigation
        keybind_entries_["suspend"].entry.set_text(settings_->get_keybind("suspend"));
        keybind_entries_["suspend"].entry.set_can_focus(false); // Prevent tab navigation
        keybind_entries_["hibernate"].entry.set_text(settings_->get_keybind("hibernate"));
        keybind_entries_["hibernate"].entry.set_can_focus(false); // Prevent tab navigation
        keybind_entries_["lock"].entry.set_text(settings_->get_keybind("lock"));
        keybind_entries_["lock"].entry.set_can_focus(false); // Prevent tab navigation

        if (show_keybind_hints_check_)
            show_keybind_hints_check_->set_active(settings_->get_show_keybind_hints());
    }

    /**
     * @brief Save the configured settings
     *
     * Saves the command settings from the dialog to the PowerSettings object
     * and writes them to the configuration file.
     */
    void PowerSettingsDialog::save_settings()
    {
        settings_->set_command("shutdown", command_entries_["shutdown"].entry.get_text());
        settings_->set_command("reboot", command_entries_["reboot"].entry.get_text());
        settings_->set_command("suspend", command_entries_["suspend"].entry.get_text());
        settings_->set_command("hibernate", command_entries_["hibernate"].entry.get_text());
        settings_->set_command("lock", command_entries_["lock"].entry.get_text());

        settings_->set_keybind("shutdown", keybind_entries_["shutdown"].entry.get_text());
        settings_->set_keybind("reboot", keybind_entries_["reboot"].entry.get_text());
        settings_->set_keybind("suspend", keybind_entries_["suspend"].entry.get_text());
        settings_->set_keybind("hibernate", keybind_entries_["hibernate"].entry.get_text());
        settings_->set_keybind("lock", keybind_entries_["lock"].entry.get_text());

        if (show_keybind_hints_check_)
            settings_->set_show_keybind_hints(show_keybind_hints_check_->get_active());

        settings_->save();
    }

    /**
     * @brief Reset settings to defaults
     *
     * Resets all command entry fields to their default values.
     * Does not save the changes until the user clicks Save.
     */
    void PowerSettingsDialog::reset_to_defaults()
    {
        // Set default commands for all entry fields
        command_entries_["shutdown"].entry.set_text("systemctl poweroff");
        command_entries_["reboot"].entry.set_text("systemctl reboot");
        command_entries_["suspend"].entry.set_text("systemctl suspend");
        command_entries_["hibernate"].entry.set_text("systemctl hibernate");
        command_entries_["lock"].entry.set_text("loginctl lock-session");

        // Set default keybinds for all entry fields
        keybind_entries_["shutdown"].entry.set_text("S");
        keybind_entries_["reboot"].entry.set_text("R");
        keybind_entries_["suspend"].entry.set_text("U");
        keybind_entries_["hibernate"].entry.set_text("H");
        keybind_entries_["lock"].entry.set_text("L");

        if (show_keybind_hints_check_)
            show_keybind_hints_check_->set_active(true);
    }

} // namespace Power
