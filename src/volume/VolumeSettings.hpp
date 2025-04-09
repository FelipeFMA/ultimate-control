#ifndef ULTIMATE_CONTROL_VOLUME_SETTINGS_HPP
#define ULTIMATE_CONTROL_VOLUME_SETTINGS_HPP

#include <string>
#include <map>

namespace Volume {

class VolumeSettings {
public:
    VolumeSettings();
    ~VolumeSettings();

    void load();
    void save() const;

    int get_default_volume() const;
    void set_default_volume(int volume);

private:
    std::map<std::string, int> settings_;
    std::string config_path_;
};

} // namespace Volume

#endif // ULTIMATE_CONTROL_VOLUME_SETTINGS_HPP
