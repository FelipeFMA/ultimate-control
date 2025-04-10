/**
 * @file WifiManager.hpp
 * @brief WiFi management functionality for Ultimate Control
 *
 * This file defines the WifiManager class which provides an interface
 * for scanning, connecting to, and managing WiFi networks using NetworkManager.
 * It uses the PIMPL idiom to hide implementation details.
 */

#ifndef ULTIMATE_CONTROL_WIFI_MANAGER_HPP
#define ULTIMATE_CONTROL_WIFI_MANAGER_HPP

#include <string>
#include <vector>
#include <functional>
#include <memory>

/**
 * @namespace Wifi
 * @brief Contains WiFi-related functionality
 */
namespace Wifi {

/**
 * @struct Network
 * @brief Represents a WiFi network detected during scanning
 */
struct Network {
    std::string ssid;        ///< Network name (SSID)
    std::string bssid;       ///< MAC address of the access point (may be empty)
    int signal_strength;     ///< Signal strength as a percentage (0-100)
    bool connected;          ///< Whether the device is currently connected to this network
    bool secured;            ///< Whether the network uses encryption (requires password)
};

/**
 * @class WifiManager
 * @brief Manages WiFi connections and network scanning
 *
 * Provides an interface for scanning available networks, connecting to networks,
 * and managing saved connections. Uses NetworkManager via its command-line
 * interface (nmcli) for all operations.
 */
class WifiManager {
public:
    using NetworkList = std::vector<Network>;  ///< Type alias for a list of WiFi networks
    using UpdateCallback = std::function<void(const NetworkList&)>;  ///< Callback type for network list updates
    using StateCallback = std::function<void(bool)>;  ///< Callback type for WiFi enabled/disabled state changes

    /**
     * @brief Constructor
     *
     * Initializes the WiFi manager and checks the current WiFi state.
     */
    WifiManager();

    /**
     * @brief Destructor
     */
    ~WifiManager();

    /**
     * @brief Scan for available WiFi networks
     *
     * Initiates a scan for available WiFi networks. When the scan is complete,
     * the update callback (if set) will be called with the list of networks.
     */
    void scan_networks();

    /**
     * @brief Connect to a WiFi network
     * @param ssid The SSID (name) of the network to connect to
     * @param password The password for the network (empty for open networks)
     * @param security_type The security type (defaults to "wpa-psk" for WPA/WPA2)
     *
     * Attempts to connect to the specified WiFi network. First tries to use
     * saved credentials if available, then creates a new connection if needed.
     */
    void connect(const std::string& ssid, const std::string& password, const std::string& security_type = "wpa-psk");

    /**
     * @brief Disconnect from the current WiFi network
     *
     * Disconnects from the currently connected WiFi network, if any.
     */
    void disconnect();

    /**
     * @brief Remove saved credentials for a WiFi network
     * @param ssid The SSID of the network to forget
     *
     * Deletes all saved connection profiles for the specified network.
     */
    void forget_network(const std::string& ssid);

    /**
     * @brief Enable the WiFi radio
     *
     * Turns on the WiFi radio and initiates a network scan.
     */
    void enable_wifi();

    /**
     * @brief Disable the WiFi radio
     *
     * Turns off the WiFi radio and clears the network list.
     */
    void disable_wifi();

    /**
     * @brief Check if WiFi is currently enabled
     * @return true if WiFi is enabled, false otherwise
     */
    bool is_wifi_enabled() const;

    /**
     * @brief Set the callback for network list updates
     * @param cb The callback function to be called when the network list changes
     *
     * The callback will be called after each successful network scan.
     */
    void set_update_callback(UpdateCallback cb);

    /**
     * @brief Set the callback for WiFi state changes
     * @param cb The callback function to be called when WiFi is enabled or disabled
     *
     * The callback will be called whenever the WiFi radio state changes.
     */
    void set_state_callback(StateCallback cb);

    /**
     * @brief Get the current list of WiFi networks
     * @return A reference to the vector of detected networks
     *
     * Returns the most recent scan results. Call scan_networks() first to update the list.
     */
    const NetworkList& get_networks() const;

private:
    class Impl;  ///< Forward declaration of implementation class
    std::unique_ptr<Impl> impl_;  ///< Pointer to implementation (PIMPL idiom)
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_MANAGER_HPP
