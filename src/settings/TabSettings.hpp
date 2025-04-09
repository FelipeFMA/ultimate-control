#ifndef ULTIMATE_CONTROL_TAB_SETTINGS_HPP
#define ULTIMATE_CONTROL_TAB_SETTINGS_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace Settings {

struct TabInfo {
    std::string id;
    std::string name;
    std::string icon_name;
    bool enabled;
};

class TabSettings {
public:
    TabSettings();
    ~TabSettings();

    // Load settings from file
    void load();
    
    // Save settings to file
    void save() const;
    
    // Get the order of tabs
    std::vector<std::string> get_tab_order() const;
    
    // Set the order of tabs
    void set_tab_order(const std::vector<std::string>& order);
    
    // Check if a tab is enabled
    bool is_tab_enabled(const std::string& tab_id) const;
    
    // Enable or disable a tab
    void set_tab_enabled(const std::string& tab_id, bool enabled);
    
    // Get all tab information
    std::vector<TabInfo> get_all_tabs() const;
    
    // Move a tab up in the order
    bool move_tab_up(const std::string& tab_id);
    
    // Move a tab down in the order
    bool move_tab_down(const std::string& tab_id);

private:
    // Create config directory if it doesn't exist
    void ensure_config_dir() const;
    
    std::string config_path_;
    std::vector<std::string> tab_order_;
    std::map<std::string, bool> tab_enabled_;
    std::map<std::string, TabInfo> tab_info_;
};

} // namespace Settings

#endif // ULTIMATE_CONTROL_TAB_SETTINGS_HPP
