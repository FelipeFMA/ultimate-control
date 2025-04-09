#include "PowerSettings.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

namespace Power {

PowerSettings::PowerSettings() {
    // Set the config path
    const char* home_dir = getenv("HOME");
    if (home_dir) {
        config_path_ = std::string(home_dir) + "/.config/ultimate-control/power.conf";
    } else {
        config_path_ = "/tmp/ultimate-control-power.conf";
    }

    // Initialize default commands
    commands_["shutdown"] = "systemctl poweroff";
    commands_["reboot"] = "systemctl reboot";
    commands_["suspend"] = "systemctl suspend";
    commands_["hibernate"] = "systemctl hibernate";
    commands_["lock"] = "loginctl lock-session";

    // Load settings from file
    load();
}

PowerSettings::~PowerSettings() {
    save();
}

std::string PowerSettings::get_command(const std::string& action) const {
    auto it = commands_.find(action);
    if (it != commands_.end()) {
        return it->second;
    }

    // Return default commands if not found
    if (action == "shutdown") return "systemctl poweroff";
    if (action == "reboot") return "systemctl reboot";
    if (action == "suspend") return "systemctl suspend";
    if (action == "hibernate") return "systemctl hibernate";
    if (action == "lock") return "loginctl lock-session";

    return "";
}

void PowerSettings::set_command(const std::string& action, const std::string& command) {
    commands_[action] = command;
}

void PowerSettings::load() {
    std::ifstream infile(config_path_);
    if (!infile.is_open()) {
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

        // Store the command
        commands_[key] = value;
    }
}

void PowerSettings::save() const {
    ensure_config_dir();

    std::ofstream outfile(config_path_);
    if (!outfile.is_open()) {
        std::cerr << "Failed to save power settings to " << config_path_ << std::endl;
        return;
    }

    // Write header
    outfile << "# Ultimate Control Power Settings\n";

    // Write commands
    for (const auto& pair : commands_) {
        outfile << pair.first << "=" << pair.second << "\n";
    }
}

void PowerSettings::ensure_config_dir() const {
    std::string dir_path;
    size_t last_slash = config_path_.find_last_of('/');
    if (last_slash != std::string::npos) {
        dir_path = config_path_.substr(0, last_slash);
    }

    if (!dir_path.empty()) {
        // Create directory with parent directories
        std::string cmd = "mkdir -p " + dir_path;
        std::system(cmd.c_str());
    }
}

} // namespace Power