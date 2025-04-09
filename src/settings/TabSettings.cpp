#include "TabSettings.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

namespace Settings {

TabSettings::TabSettings() {
    // Set the config path
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        config_path_ = std::string(home_dir) + "/.config/ultimate-control/settings.json";
    } else {
        config_path_ = "/tmp/ultimate-control-settings.json";
    }

    // Config path is set

    // Initialize default tab information
    tab_info_["volume"] = {"volume", "Volume", "audio-volume-high-symbolic", true};
    tab_info_["wifi"] = {"wifi", "WiFi", "network-wireless-symbolic", true};
    tab_info_["display"] = {"display", "Display", "video-display-symbolic", true};
    tab_info_["power"] = {"power", "Power", "system-shutdown-symbolic", true};
    tab_info_["settings"] = {"settings", "Settings", "preferences-system-symbolic", true};

    // Default tab order
    tab_order_ = {"volume", "wifi", "display", "power", "settings"};

    // Default all tabs to enabled
    for (const auto& tab : tab_info_) {
        tab_enabled_[tab.first] = true;
    }

    // Load settings from file
    load();
}

TabSettings::~TabSettings() {
    save();
}

void TabSettings::ensure_config_dir() const {
    std::string dir_path = config_path_.substr(0, config_path_.find_last_of('/'));

    // Create directory structure
    std::string current_path;
    std::istringstream path_stream(dir_path);
    std::string path_part;

    while (std::getline(path_stream, path_part, '/')) {
        if (path_part.empty()) {
            current_path = "/";
            continue;
        }

        current_path += path_part + "/";

        struct stat info;
        if (stat(current_path.c_str(), &info) != 0) {
            // Directory doesn't exist, create it
            if (mkdir(current_path.c_str(), 0755) != 0) {
                std::cerr << "Failed to create directory: " << current_path << std::endl;
                return;
            }
        }
    }
}

void TabSettings::load() {
    std::ifstream infile(config_path_);
    if (!infile.is_open()) {
        // File doesn't exist, use defaults
        return;
    }
    std::string line;
    while (std::getline(infile, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t delimiter_pos = line.find('=');
        if (delimiter_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, delimiter_pos);
        std::string value = line.substr(delimiter_pos + 1);

        if (key == "tab_order") {
            // Parse tab order
            tab_order_.clear();
            std::istringstream order_stream(value);
            std::string tab_id;

            while (std::getline(order_stream, tab_id, ',')) {
                if (!tab_id.empty()) {
                    tab_order_.push_back(tab_id);
                }
            }
        } else if (key.substr(0, 4) == "tab_") {
            // Parse tab enabled state
            std::string tab_id = key.substr(4);
            bool enabled = (value == "1" || value == "true");
            tab_enabled_[tab_id] = enabled;
        }
    }

    // Ensure all tabs are in the order list
    for (const auto& tab : tab_info_) {
        if (std::find(tab_order_.begin(), tab_order_.end(), tab.first) == tab_order_.end()) {
            tab_order_.push_back(tab.first);
        }
    }
}

void TabSettings::save() const {
    ensure_config_dir();

    std::ofstream outfile(config_path_);
    if (!outfile.is_open()) {
        std::cerr << "Failed to save tab settings to " << config_path_ << std::endl;
        return;
    }

    // Write header
    outfile << "# Ultimate Control Tab Settings\n";

    // Write tab order
    outfile << "tab_order=";
    for (size_t i = 0; i < tab_order_.size(); ++i) {
        outfile << tab_order_[i];
        if (i < tab_order_.size() - 1) {
            outfile << ",";
        }
    }
    outfile << "\n";

    // Write tab enabled states
    for (const auto& tab : tab_enabled_) {
        outfile << "tab_" << tab.first << "=" << (tab.second ? "1" : "0") << "\n";
    }
}

std::vector<std::string> TabSettings::get_tab_order() const {
    return tab_order_;
}

void TabSettings::set_tab_order(const std::vector<std::string>& order) {
    tab_order_ = order;
}

bool TabSettings::is_tab_enabled(const std::string& tab_id) const {
    auto it = tab_enabled_.find(tab_id);
    if (it != tab_enabled_.end()) {
        return it->second;
    }
    return true; // Default to enabled
}

void TabSettings::set_tab_enabled(const std::string& tab_id, bool enabled) {
    tab_enabled_[tab_id] = enabled;
}

std::vector<TabInfo> TabSettings::get_all_tabs() const {
    std::vector<TabInfo> result;

    for (const auto& tab_id : tab_order_) {
        auto it = tab_info_.find(tab_id);
        if (it != tab_info_.end()) {
            TabInfo info = it->second;
            info.enabled = is_tab_enabled(tab_id);
            result.push_back(info);
        }
    }

    return result;
}

bool TabSettings::move_tab_up(const std::string& tab_id) {
    auto it = std::find(tab_order_.begin(), tab_order_.end(), tab_id);
    if (it == tab_order_.end() || it == tab_order_.begin()) {
        return false;
    }

    std::iter_swap(it, it - 1);
    return true;
}

bool TabSettings::move_tab_down(const std::string& tab_id) {
    auto it = std::find(tab_order_.begin(), tab_order_.end(), tab_id);
    if (it == tab_order_.end() || it == tab_order_.end() - 1) {
        return false;
    }

    std::iter_swap(it, it + 1);
    return true;
}

} // namespace Settings
