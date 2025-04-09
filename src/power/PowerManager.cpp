#include "PowerManager.hpp"
#include <cstdlib>
#include <cstdio>
#include <array>
#include <string>
#include <vector>
#include <iostream>

namespace Power {

PowerManager::PowerManager() = default;
PowerManager::~PowerManager() = default;

void PowerManager::shutdown() {
    std::system("systemctl poweroff");
    notify();
}

void PowerManager::reboot() {
    std::system("systemctl reboot");
    notify();
}

void PowerManager::suspend() {
    std::system("systemctl suspend");
    notify();
}

void PowerManager::hibernate() {
    std::system("systemctl hibernate");
    notify();
}

std::vector<std::string> PowerManager::list_power_profiles() {
    std::vector<std::string> profiles;
    std::string cmd = "powerprofilesctl list";
    std::array<char, 2048> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return profiles;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    size_t pos = 0;
    while ((pos = result.find('\n')) != std::string::npos) {
        std::string line = result.substr(0, pos);
        result.erase(0, pos + 1);

        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;

        // If line starts with '*', remove it
        if (line[0] == '*') {
            line = line.substr(1);
            line.erase(0, line.find_first_not_of(" \t"));
        }

        // If line ends with ':', it's a profile name
        if (!line.empty() && line.back() == ':') {
            line.pop_back();
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (!line.empty()) {
                profiles.push_back(line);
            }
        }
    }
    return profiles;
}

void PowerManager::set_power_profile(const std::string& profile) {
    std::string cmd = "powerprofilesctl set " + profile;
    std::system(cmd.c_str());
    notify();
}

std::string PowerManager::get_current_power_profile() {
    std::string cmd = "powerprofilesctl get";
    std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

void PowerManager::set_update_callback(Callback cb) {
    callback_ = cb;
}

void PowerManager::notify() {
    if (callback_) {
        callback_();
    }
}

} // namespace Power
