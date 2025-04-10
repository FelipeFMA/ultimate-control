/**
 * @file WifiNetworkWidget.hpp
 * @brief Widget for displaying and interacting with a WiFi network
 *
 * This file defines the WifiNetworkWidget class which displays a single
 * WiFi network in the UI with signal strength, security status, and connection
 * controls. It provides functionality for connecting to networks, forgetting
 * saved networks, and sharing networks via QR codes.
 */

#pragma once

#include <gtkmm.h>
#include "WifiManager.hpp"
#include <memory>
#include "../utils/QRCode.hpp"

namespace Wifi {

/**
 * @class WifiNetworkWidget
 * @brief Widget for displaying and interacting with a WiFi network
 *
 * Displays information about a WiFi network including its name (SSID),
 * signal strength, security status, and connection status. Provides
 * buttons for connecting to the network, forgetting saved credentials,
 * and sharing the network via QR code.
 */
class WifiNetworkWidget : public Gtk::Box {
public:
    /**
     * @brief Constructor for the WiFi network widget
     * @param network The network to display
     * @param manager Shared pointer to the WiFi manager
     *
     * Creates a widget that displays information about a WiFi network and
     * provides controls for connecting, forgetting, and sharing the network.
     */
    WifiNetworkWidget(const Network& network, std::shared_ptr<WifiManager> manager);

    /**
     * @brief Virtual destructor
     */
    virtual ~WifiNetworkWidget();

private:
    /**
     * @brief Handler for connect/disconnect button clicks
     *
     * If already connected, disconnects from the network.
     * If not connected, attempts to connect using saved credentials first.
     * If that fails and the network is secured, prompts for a password.
     */
    void on_connect_clicked();

    /**
     * @brief Handler for forget button clicks
     *
     * Shows a confirmation dialog and if confirmed, removes all saved
     * credentials for the network using the WiFi manager.
     */
    void on_forget_clicked();

    /**
     * @brief Handler for share button clicks
     *
     * Creates a QR code for the WiFi network that can be scanned by phones
     * to connect. For secured networks, prompts for the password if not connected.
     */
    void on_share_clicked();

    /**
     * @brief Convert signal strength to a human-readable quality string
     * @param signal_strength Signal strength as a percentage (0-100)
     * @return String representation of the signal strength
     */
    std::string convert_signal_to_quality(int signal_strength);

    /**
     * @brief Update the signal strength icon based on signal level
     * @param signal_strength Signal strength as a percentage (0-100)
     */
    void update_signal_icon(int signal_strength);

    /**
     * @brief Update the security icon based on network encryption
     * @param secured Whether the network uses encryption
     */
    void update_security_icon(bool secured);

    /**
     * @brief Update the connection status icon
     * @param connected Whether the device is currently connected to this network
     */
    void update_connection_status(bool connected);

    std::shared_ptr<WifiManager> manager_;  ///< Pointer to the WiFi manager for network operations
    Network network_;                      ///< The network this widget represents

    // Main layout containers
    Gtk::Box network_info_box_;  ///< Container for network information (SSID, icons)
    Gtk::Box controls_box_;      ///< Container for buttons (connect, forget, share)

    // Network information widgets
    Gtk::Image signal_icon_;    ///< Icon showing signal strength
    Gtk::Image security_icon_;  ///< Icon showing security status (locked/unlocked)
    Gtk::Image status_icon_;    ///< Icon showing connection status
    Gtk::Label ssid_label_;     ///< Label showing the network name
    Gtk::Label signal_label_;   ///< Label showing signal strength percentage
    Gtk::Label status_label_;   ///< Label showing connection status text

    // Control widgets
    Gtk::Button connect_button_;  ///< Button to connect/disconnect from the network
    Gtk::Button forget_button_;   ///< Button to forget saved credentials
    Gtk::Button share_button_;    ///< Button to share network via QR code
};

} // namespace Wifi

