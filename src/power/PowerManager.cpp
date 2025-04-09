#include "PowerManager.hpp"
#include <cstdlib>
#include <cstdio>
#include <array>
#include <string>
#include <vector>
#include <iostream>

namespace Power {

PowerManager::PowerManager()
: settings_(std::make_shared<PowerSettings>())
{
}

PowerManager::~PowerManager() = default;

void PowerManager::shutdown() {
    std::system(settings_->get_command("shutdown").c_str());
    notify();
}

void PowerManager::reboot() {
    std::system(settings_->get_command("reboot").c_str());
    notify();
}

void PowerManager::suspend() {
    std::system(settings_->get_command("suspend").c_str());
    notify();
}

void PowerManager::hibernate() {
    std::system(settings_->get_command("hibernate").c_str());
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

std::shared_ptr<PowerSettings> PowerManager::get_settings() const {
    return settings_;
}

} // namespace Power
