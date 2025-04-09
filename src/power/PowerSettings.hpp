#ifndef ULTIMATE_CONTROL_POWER_SETTINGS_HPP
#define ULTIMATE_CONTROL_POWER_SETTINGS_HPP

#include <string>
#include <map>

namespace Power {

class PowerSettings {
public:
    PowerSettings();
    ~PowerSettings();

    // Get command for a specific action
    std::string get_command(const std::string& action) const;

    // Set command for a specific action
    void set_command(const std::string& action, const std::string& command);

    // Load settings from file
    void load();

    // Save settings to file
    void save() const;

private:
    // Create config directory if it doesn't exist
    void ensure_config_dir() const;

    std::string config_path_;
    std::map<std::string, std::string> commands_;
};

} // namespace Power

#endif // ULTIMATE_CONTROL_POWER_SETTINGS_HPP