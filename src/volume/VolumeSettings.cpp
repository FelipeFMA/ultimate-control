#include "VolumeSettings.hpp"
#include <fstream>
#include <iostream>

namespace Volume {

VolumeSettings::VolumeSettings() {
    config_path_ = "/home/felipe/.config/ultimate-control/volume.conf";
    load();
}

VolumeSettings::~VolumeSettings() {
    save();
}

void VolumeSettings::load() {
    settings_.clear();
    std::ifstream infile(config_path_);
    if (!infile.is_open()) {
        return;
    }

    std::string key;
    int value;
    while (infile >> key >> value) {
        settings_[key] = value;
    }
}

void VolumeSettings::save() const {
    std::ofstream outfile(config_path_);
    if (!outfile.is_open()) {
        std::cerr << "Failed to save volume settings\n";
        return;
    }

    for (const auto& pair : settings_) {
        outfile << pair.first << " " << pair.second << "\n";
    }
}

int VolumeSettings::get_default_volume() const {
    auto it = settings_.find("default_volume");
    if (it != settings_.end()) {
        return it->second;
    }
    return 50; // default fallback
}

void VolumeSettings::set_default_volume(int volume) {
    settings_["default_volume"] = volume;
}

} // namespace Volume
