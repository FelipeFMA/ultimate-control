#ifndef ULTIMATE_CONTROL_WIFI_MANAGER_HPP
#define ULTIMATE_CONTROL_WIFI_MANAGER_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace Wifi {

struct Network {
    std::string ssid;
    std::string bssid;
    int signal_strength; // 0-100
    bool connected;
    bool secured;
};

class WifiManager {
public:
    using NetworkList = std::vector<Network>;
    using UpdateCallback = std::function<void(const NetworkList&)>;
    using StateCallback = std::function<void(bool)>;

    WifiManager();
    ~WifiManager();

    void scan_networks();
    void connect(const std::string& ssid, const std::string& password);
    void disconnect();
    void forget_network(const std::string& ssid);
    void enable_wifi();
    void disable_wifi();
    bool is_wifi_enabled() const;
    void set_update_callback(UpdateCallback cb);
    void set_state_callback(StateCallback cb);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_MANAGER_HPP
