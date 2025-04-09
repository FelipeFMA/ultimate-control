#ifndef ULTIMATE_CONTROL_POWER_MANAGER_HPP
#define ULTIMATE_CONTROL_POWER_MANAGER_HPP

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "PowerSettings.hpp"

namespace Power {

class PowerManager {
public:
    using Callback = std::function<void()>;

    PowerManager();
    ~PowerManager();

    void shutdown();
    void reboot();
    void suspend();
    void hibernate();

    std::vector<std::string> list_power_profiles();
    void set_power_profile(const std::string& profile);
    std::string get_current_power_profile();

    void set_update_callback(Callback cb);

    // Get the power settings
    std::shared_ptr<PowerSettings> get_settings() const;

private:
    Callback callback_;
    std::shared_ptr<PowerSettings> settings_;
    void notify();
};

} // namespace Power

#endif // ULTIMATE_CONTROL_POWER_MANAGER_HPP
