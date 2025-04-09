#include "PowerSettingsDialog.hpp"
#include <iostream>

namespace Power {

PowerSettingsDialog::PowerSettingsDialog(Gtk::Window& parent, std::shared_ptr<PowerSettings> settings)
: Gtk::Dialog("Power Command Settings", parent, true),
  settings_(settings)
{
    set_default_size(500, 300);

    auto content = get_content_area();
    content->set_margin_top(10);
    content->set_margin_bottom(10);
    content->set_margin_start(10);
    content->set_margin_end(10);
    content->set_spacing(10);

    // Add header with icon and title
    auto header_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 10);
    auto header_icon = Gtk::make_managed<Gtk::Image>();
    header_icon->set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_DIALOG);
    header_box->pack_start(*header_icon, Gtk::PACK_SHRINK);

    auto header_label = Gtk::make_managed<Gtk::Label>();
    header_label->set_markup("<span size='large' weight='bold'>Power Command Settings</span>");
    header_label->set_halign(Gtk::ALIGN_START);
    header_label->set_valign(Gtk::ALIGN_CENTER);
    header_box->pack_start(*header_label, Gtk::PACK_EXPAND_WIDGET);

    content->pack_start(*header_box, Gtk::PACK_SHRINK);

    // Add description
    auto description = Gtk::make_managed<Gtk::Label>();
    description->set_markup("Configure the commands that will be executed when pressing the power buttons:");
    description->set_halign(Gtk::ALIGN_START);
    description->set_line_wrap(true);
    content->pack_start(*description, Gtk::PACK_SHRINK);

    // Create a grid for the command entries
    auto grid = Gtk::make_managed<Gtk::Grid>();
    grid->set_row_spacing(10);
    grid->set_column_spacing(10);
    grid->set_margin_top(10);

    // Add command entries
    int row = 0;

    // Shutdown command
    command_entries_["shutdown"].label.set_text("Shutdown command:");
    command_entries_["shutdown"].label.set_halign(Gtk::ALIGN_START);
    command_entries_["shutdown"].entry.set_hexpand(true);
    grid->attach(command_entries_["shutdown"].label, 0, row, 1, 1);
    grid->attach(command_entries_["shutdown"].entry, 1, row, 1, 1);
    row++;

    // Reboot command
    command_entries_["reboot"].label.set_text("Reboot command:");
    command_entries_["reboot"].label.set_halign(Gtk::ALIGN_START);
    command_entries_["reboot"].entry.set_hexpand(true);
    grid->attach(command_entries_["reboot"].label, 0, row, 1, 1);
    grid->attach(command_entries_["reboot"].entry, 1, row, 1, 1);
    row++;

    // Suspend command
    command_entries_["suspend"].label.set_text("Suspend command:");
    command_entries_["suspend"].label.set_halign(Gtk::ALIGN_START);
    command_entries_["suspend"].entry.set_hexpand(true);
    grid->attach(command_entries_["suspend"].label, 0, row, 1, 1);
    grid->attach(command_entries_["suspend"].entry, 1, row, 1, 1);
    row++;

    // Hibernate command
    command_entries_["hibernate"].label.set_text("Hibernate command:");
    command_entries_["hibernate"].label.set_halign(Gtk::ALIGN_START);
    command_entries_["hibernate"].entry.set_hexpand(true);
    grid->attach(command_entries_["hibernate"].label, 0, row, 1, 1);
    grid->attach(command_entries_["hibernate"].entry, 1, row, 1, 1);
    row++;

    // Lock command
    command_entries_["lock"].label.set_text("Lock screen command:");
    command_entries_["lock"].label.set_halign(Gtk::ALIGN_START);
    command_entries_["lock"].entry.set_hexpand(true);
    grid->attach(command_entries_["lock"].label, 0, row, 1, 1);
    grid->attach(command_entries_["lock"].entry, 1, row, 1, 1);

    content->pack_start(*grid, Gtk::PACK_EXPAND_WIDGET);

    // Create a button box for custom buttons
    auto button_box = Gtk::manage(new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL));
    button_box->set_layout(Gtk::BUTTONBOX_END);
    button_box->set_spacing(5);

    // Create reset button
    reset_button_ = Gtk::manage(new Gtk::Button("_Reset to Defaults"));
    reset_button_->set_use_underline(true);
    reset_button_->signal_clicked().connect(sigc::mem_fun(*this, &PowerSettingsDialog::reset_to_defaults));
    button_box->pack_start(*reset_button_, Gtk::PACK_SHRINK);

    // Add the button box to the content area
    content->pack_start(*button_box, Gtk::PACK_SHRINK);

    // Add standard dialog buttons
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    add_button("_Save", Gtk::RESPONSE_OK);
    set_default_response(Gtk::RESPONSE_OK);

    // Load current settings
    load_settings();

    show_all_children();
}

PowerSettingsDialog::~PowerSettingsDialog() = default;

void PowerSettingsDialog::load_settings() {
    command_entries_["shutdown"].entry.set_text(settings_->get_command("shutdown"));
    command_entries_["reboot"].entry.set_text(settings_->get_command("reboot"));
    command_entries_["suspend"].entry.set_text(settings_->get_command("suspend"));
    command_entries_["hibernate"].entry.set_text(settings_->get_command("hibernate"));
    command_entries_["lock"].entry.set_text(settings_->get_command("lock"));
}

void PowerSettingsDialog::save_settings() {
    settings_->set_command("shutdown", command_entries_["shutdown"].entry.get_text());
    settings_->set_command("reboot", command_entries_["reboot"].entry.get_text());
    settings_->set_command("suspend", command_entries_["suspend"].entry.get_text());
    settings_->set_command("hibernate", command_entries_["hibernate"].entry.get_text());
    settings_->set_command("lock", command_entries_["lock"].entry.get_text());
    settings_->save();
}

void PowerSettingsDialog::reset_to_defaults() {
    // Set default commands
    command_entries_["shutdown"].entry.set_text("systemctl poweroff");
    command_entries_["reboot"].entry.set_text("systemctl reboot");
    command_entries_["suspend"].entry.set_text("systemctl suspend");
    command_entries_["hibernate"].entry.set_text("systemctl hibernate");
    command_entries_["lock"].entry.set_text("loginctl lock-session");
}

} // namespace Power
