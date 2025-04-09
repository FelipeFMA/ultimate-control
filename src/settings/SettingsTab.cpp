#include "SettingsTab.hpp"
#include <iostream>

namespace Settings {

SettingsTab::SettingsTab()
: settings_(std::make_shared<TabSettings>()),
  main_box_(Gtk::ORIENTATION_VERTICAL, 15),
  tab_order_box_(Gtk::ORIENTATION_VERTICAL, 10),
  tab_order_header_box_(Gtk::ORIENTATION_HORIZONTAL, 10),
  tab_list_box_(Gtk::ORIENTATION_VERTICAL, 5),
  buttons_box_(Gtk::ORIENTATION_HORIZONTAL, 10)
{
    // Set up the main container
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    // Add a scrolled window
    scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the main box inside the scrolled window
    main_box_.set_margin_start(20);
    main_box_.set_margin_end(20);
    main_box_.set_margin_top(20);
    main_box_.set_margin_bottom(20);
    scrolled_window_.add(main_box_);

    // Create the tab order section
    create_tab_order_section();

    // Add the save button
    save_button_.set_label("Save Settings");
    save_button_.set_image_from_icon_name("document-save-symbolic", Gtk::ICON_SIZE_BUTTON);
    save_button_.set_always_show_image(true);
    save_button_.signal_clicked().connect(sigc::mem_fun(*this, &SettingsTab::on_save_clicked));
    
    buttons_box_.set_halign(Gtk::ALIGN_END);
    buttons_box_.pack_start(save_button_, Gtk::PACK_SHRINK);
    
    main_box_.pack_start(buttons_box_, Gtk::PACK_SHRINK);

    // Update the tab list
    update_tab_list();

    show_all_children();
    std::cout << "Settings tab loaded!" << std::endl;
}

SettingsTab::~SettingsTab() = default;

void SettingsTab::create_tab_order_section() {
    // Set up the tab order section frame
    tab_order_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    tab_order_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
    tab_order_box_.set_spacing(10);
    tab_order_box_.set_margin_start(15);
    tab_order_box_.set_margin_end(15);
    tab_order_box_.set_margin_top(15);
    tab_order_box_.set_margin_bottom(15);

    // Set up the tab order header
    tab_order_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    tab_order_header_box_.set_spacing(10);

    // Add icon and label to the header
    tab_order_icon_.set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_DIALOG);
    tab_order_label_.set_markup("<span size='large'><b>Tab Settings</b></span>");
    tab_order_label_.set_halign(Gtk::ALIGN_START);
    tab_order_label_.set_valign(Gtk::ALIGN_CENTER);

    tab_order_header_box_.pack_start(tab_order_icon_, Gtk::PACK_SHRINK);
    tab_order_header_box_.pack_start(tab_order_label_, Gtk::PACK_SHRINK);

    // Add description label
    Gtk::Label* description = Gtk::manage(new Gtk::Label());
    description->set_markup("Configure which tabs are visible and their order:");
    description->set_halign(Gtk::ALIGN_START);
    description->set_margin_bottom(10);
    
    // Add the tab list box
    tab_list_box_.set_margin_start(10);
    tab_list_box_.set_margin_end(10);
    
    // Add components to the tab order box
    tab_order_box_.pack_start(tab_order_header_box_, Gtk::PACK_SHRINK);
    tab_order_box_.pack_start(*description, Gtk::PACK_SHRINK);
    tab_order_box_.pack_start(tab_list_box_, Gtk::PACK_SHRINK);

    // Add the tab order box to the frame
    tab_order_frame_.add(tab_order_box_);
    
    // Add the frame to the main box
    main_box_.pack_start(tab_order_frame_, Gtk::PACK_SHRINK);
}

void SettingsTab::update_tab_list() {
    // Clear existing rows
    for (auto& row : tab_rows_) {
        tab_list_box_.remove(row->row_box);
    }
    tab_rows_.clear();
    
    // Get all tabs
    auto tabs = settings_->get_all_tabs();
    
    // Create a row for each tab
    for (const auto& tab : tabs) {
        auto row = std::make_unique<TabRow>();
        row->id = tab.id;
        
        // Set up the row box
        row->row_box.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        row->row_box.set_spacing(10);
        row->row_box.set_margin_bottom(5);
        
        // Set up the enabled checkbox
        row->enabled_check.set_active(tab.enabled);
        row->enabled_check.signal_toggled().connect(
            [this, id = tab.id]() { on_tab_enabled_toggled(id); });
        
        // Set up the name label with icon
        auto icon = Gtk::manage(new Gtk::Image());
        icon->set_from_icon_name(tab.icon_name, Gtk::ICON_SIZE_MENU);
        row->name_label.set_text(tab.name);
        row->name_label.set_xalign(0.0);
        
        // Set up the up/down buttons
        row->up_button.set_image_from_icon_name("go-up-symbolic", Gtk::ICON_SIZE_BUTTON);
        row->up_button.set_tooltip_text("Move up");
        row->up_button.signal_clicked().connect(sigc::mem_fun(*this, &SettingsTab::on_move_up_clicked));
        
        row->down_button.set_image_from_icon_name("go-down-symbolic", Gtk::ICON_SIZE_BUTTON);
        row->down_button.set_tooltip_text("Move down");
        row->down_button.signal_clicked().connect(sigc::mem_fun(*this, &SettingsTab::on_move_down_clicked));
        
        // Add components to the row box
        row->row_box.pack_start(row->enabled_check, Gtk::PACK_SHRINK);
        row->row_box.pack_start(*icon, Gtk::PACK_SHRINK);
        row->row_box.pack_start(row->name_label, Gtk::PACK_EXPAND_WIDGET);
        row->row_box.pack_end(row->down_button, Gtk::PACK_SHRINK);
        row->row_box.pack_end(row->up_button, Gtk::PACK_SHRINK);
        
        // Add the row to the list
        tab_list_box_.pack_start(row->row_box, Gtk::PACK_SHRINK);
        
        // Store the row
        tab_rows_.push_back(std::move(row));
    }
    
    // Update button sensitivity
    if (!tab_rows_.empty()) {
        tab_rows_.front()->up_button.set_sensitive(false);
        tab_rows_.back()->down_button.set_sensitive(false);
    }
    
    show_all_children();
}

void SettingsTab::on_move_up_clicked() {
    // Find which button was clicked
    for (size_t i = 0; i < tab_rows_.size(); ++i) {
        if (tab_rows_[i]->up_button.has_focus()) {
            // Move this tab up
            if (settings_->move_tab_up(tab_rows_[i]->id)) {
                update_tab_list();
            }
            break;
        }
    }
}

void SettingsTab::on_move_down_clicked() {
    // Find which button was clicked
    for (size_t i = 0; i < tab_rows_.size(); ++i) {
        if (tab_rows_[i]->down_button.has_focus()) {
            // Move this tab down
            if (settings_->move_tab_down(tab_rows_[i]->id)) {
                update_tab_list();
            }
            break;
        }
    }
}

void SettingsTab::on_tab_enabled_toggled(const std::string& tab_id) {
    // Find the row for this tab
    for (const auto& row : tab_rows_) {
        if (row->id == tab_id) {
            // Update the setting
            settings_->set_tab_enabled(tab_id, row->enabled_check.get_active());
            break;
        }
    }
}

void SettingsTab::on_save_clicked() {
    // Save the settings
    settings_->save();
    
    // Notify that settings have changed
    if (settings_changed_callback_) {
        settings_changed_callback_();
    }
}

void SettingsTab::set_settings_changed_callback(SettingsChangedCallback callback) {
    settings_changed_callback_ = callback;
}

std::shared_ptr<TabSettings> SettingsTab::get_tab_settings() const {
    return settings_;
}

} // namespace Settings
