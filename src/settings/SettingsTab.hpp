#ifndef ULTIMATE_CONTROL_SETTINGS_TAB_HPP
#define ULTIMATE_CONTROL_SETTINGS_TAB_HPP

#include <gtkmm.h>
#include "TabSettings.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace Settings {

class SettingsTab : public Gtk::Box {
public:
    SettingsTab();
    virtual ~SettingsTab();
    
    // Set callback for when settings are changed
    using SettingsChangedCallback = std::function<void()>;
    void set_settings_changed_callback(SettingsChangedCallback callback);
    
    // Get the tab settings
    std::shared_ptr<TabSettings> get_tab_settings() const;

private:
    void create_tab_order_section();
    void update_tab_list();
    void on_move_up_clicked();
    void on_move_down_clicked();
    void on_tab_enabled_toggled(const std::string& tab_id);
    void on_save_clicked();
    
    std::shared_ptr<TabSettings> settings_;
    SettingsChangedCallback settings_changed_callback_;
    
    // Main containers
    Gtk::ScrolledWindow scrolled_window_;
    Gtk::Box main_box_;
    
    // Tab order section
    Gtk::Frame tab_order_frame_;
    Gtk::Box tab_order_box_;
    Gtk::Box tab_order_header_box_;
    Gtk::Image tab_order_icon_;
    Gtk::Label tab_order_label_;
    
    // Tab list
    Gtk::Box tab_list_box_;
    struct TabRow {
        std::string id;
        Gtk::Box row_box;
        Gtk::CheckButton enabled_check;
        Gtk::Label name_label;
        Gtk::Button up_button;
        Gtk::Button down_button;
    };
    std::vector<std::unique_ptr<TabRow>> tab_rows_;
    
    // Buttons
    Gtk::Box buttons_box_;
    Gtk::Button save_button_;
};

} // namespace Settings

#endif // ULTIMATE_CONTROL_SETTINGS_TAB_HPP
