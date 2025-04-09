#include "BatteryManager.hpp"
#include <fstream>
#include <iostream>
#include <string>

namespace Battery {

BatteryManager::BatteryManager() {
    refresh();
}

BatteryManager::~BatteryManager() = default;

BatteryStatus BatteryManager::get_status() const {
    return status_;
}

void BatteryManager::refresh() {
    BatteryStatus new_status{};
    new_status.present = false;

    std::ifstream present_file("/sys/class/power_supply/BAT0/present");
    if (present_file) {
        int present = 0;
        present_file >> present;
        new_status.present = (present == 1);
    }

    if (!new_status.present) {
        status_ = new_status;
        notify();
        return;
    }

    std::ifstream capacity_file("/sys/class/power_supply/BAT0/capacity");
    if (capacity_file) {
        capacity_file >> new_status.percentage;
    } else {
        new_status.percentage = 0;
    }

    std::ifstream status_file("/sys/class/power_supply/BAT0/status");
    if (status_file) {
        std::string stat;
        status_file >> stat;
        new_status.charging = (stat == "Charging");
    } else {
        new_status.charging = false;
    }

    status_ = new_status;
    notify();
}

void BatteryManager::set_update_callback(UpdateCallback cb) {
    callback_ = cb;
    notify();
}

void BatteryManager::notify() {
    if (callback_) {
        callback_(status_);
    }
}

} // namespace Battery
