/**
 * @file WifiTab.hpp
 * @brief WiFi tab for the Ultimate Control application
 *
 * This file defines the WifiTab class which provides a user interface
 * for scanning, viewing, and connecting to WiFi networks. It displays
 * a list of available networks and allows the user to enable/disable WiFi.
 */

#ifndef ULTIMATE_CONTROL_WIFI_TAB_HPP
#define ULTIMATE_CONTROL_WIFI_TAB_HPP

#include <gtkmm.h>
#include "WifiManager.hpp"
#include "WifiNetworkWidget.hpp"
#include <memory>
#include <vector>

namespace Wifi {

/**
 * @class WifiTab
 * @brief Tab for WiFi network management
 *
 * Provides a user interface for scanning, viewing, and connecting to WiFi networks.
 * Displays a list of available networks with signal strength and security information.
 * Allows enabling/disabling WiFi and scanning for networks.
 */
class WifiTab : public Gtk::ScrolledWindow {
public:
    /**
     * @brief Constructor for the WiFi tab
     *
     * Initializes the WiFi manager and creates the UI components.
     * Does not perform an initial network scan to improve loading time.
     */
    WifiTab();

    /**
     * @brief Virtual destructor
     */
    virtual ~WifiTab();

private:
    /**
     * @brief Update the list of displayed WiFi networks
     * @param networks Vector of Network objects to display
     *
     * Clears the current list of network widgets and creates new ones
     * for each network in the provided vector.
     */
    void update_network_list(const std::vector<Network>& networks);

    /**
     * @brief Update the UI based on WiFi state
     * @param enabled Whether WiFi is enabled
     *
     * Updates the WiFi icon, status label, and scan button sensitivity
     * based on the current WiFi state.
     */
    void update_wifi_state(bool enabled);

    /**
     * @brief Handler for WiFi switch toggle events
     *
     * Enables or disables WiFi based on the switch state.
     */
    void on_wifi_switch_toggled();

    /**
     * @brief Perform a delayed network scan
     *
     * Called after the tab is fully loaded and visible to the user.
     * Delays the scan by a short time to ensure UI responsiveness.
     */
    void perform_delayed_scan();

    std::shared_ptr<WifiManager> manager_;  ///< WiFi manager for network operations
    Gtk::Box container_;                    ///< Container for network widgets
    Gtk::Button scan_button_;               ///< Button to trigger network scanning
    Gtk::Switch wifi_switch_;               ///< Switch to enable/disable WiFi
    Gtk::Label wifi_status_label_;          ///< Label showing WiFi status (Enabled/Disabled)
    Gtk::Image wifi_status_icon_;           ///< Icon showing WiFi status
    std::vector<std::unique_ptr<WifiNetworkWidget>> widgets_;  ///< List of network widgets
    bool initial_scan_performed_ = false;   ///< Flag to track if initial scan has been done
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_TAB_HPP
