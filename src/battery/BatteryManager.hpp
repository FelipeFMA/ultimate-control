#ifndef ULTIMATE_CONTROL_BATTERY_MANAGER_HPP
#define ULTIMATE_CONTROL_BATTERY_MANAGER_HPP

#include <functional>

namespace Battery {

struct BatteryStatus {
    int percentage; // 0-100
    bool charging;
    bool present;
};

class BatteryManager {
public:
    using UpdateCallback = std::function<void(const BatteryStatus&)>;

    BatteryManager();
    ~BatteryManager();

    BatteryStatus get_status() const;
    void refresh();
    void set_update_callback(UpdateCallback cb);

private:
    void notify();

    BatteryStatus status_;
    UpdateCallback callback_;
};

} // namespace Battery

#endif // ULTIMATE_CONTROL_BATTERY_MANAGER_HPP
