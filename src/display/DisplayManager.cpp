#include "DisplayManager.hpp"
#include <cstdlib>
#include <cstdio>
#include <array>
#include <iostream>
#include <algorithm>

namespace Display {

DisplayManager::DisplayManager() {
    brightness_ = get_brightness();
}

DisplayManager::~DisplayManager() = default;

int DisplayManager::get_brightness() const {
    std::string cmd = "brightnessctl get";
    std::array<char, 128> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return 0;
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    int current = 0;
    try {
        current = std::stoi(result);
    } catch (...) {
        return 0;
    }

    cmd = "brightnessctl max";
    result.clear();
    pipe = popen(cmd.c_str(), "r");
    if (!pipe) return 0;
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result = buffer.data();
    }
    pclose(pipe);

    int max = 1;
    try {
        max = std::stoi(result);
    } catch (...) {
        max = 1;
    }

    int percent = static_cast<int>(100.0 * current / max);
    return std::clamp(percent, 0, 100);
}

void DisplayManager::set_brightness(int value) {
    int clamped = std::clamp(value, 0, 100);
    std::string cmd = "brightnessctl set " + std::to_string(clamped) + "%";
    std::system(cmd.c_str());
    brightness_ = clamped;
    notify();
}

void DisplayManager::set_update_callback(BrightnessCallback cb) {
    callback_ = cb;
    notify();
}

void DisplayManager::notify() {
    if (callback_) {
        callback_(brightness_);
    }
}

} // namespace Display
