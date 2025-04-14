/**
 * @file BluetoothTab.hpp
 * @brief Bluetooth tab for the Ultimate Control application
 *
 * This file defines the BluetoothTab class which provides a user interface
 * for scanning, viewing, and connecting to Bluetooth devices. It displays
 * a list of available devices and allows the user to enable/disable Bluetooth.
 */

#pragma once

#include <gtkmm.h>
#include "BluetoothManager.hpp"
#include "BluetoothDeviceWidget.hpp"
#include <memory>
#include <vector>

namespace Bluetooth
{

    /**
     * @class BluetoothTab
     * @brief Tab for Bluetooth device management
     *
     * Provides a user interface for scanning, viewing, and connecting to Bluetooth devices.
     * Displays a list of available devices with signal strength and connection status.
     * Allows enabling/disabling Bluetooth and scanning for devices.
     */
    class BluetoothTab : public Gtk::ScrolledWindow
    {
    public:
        /**
         * @brief Constructor for the Bluetooth tab
         *
         * Initializes the Bluetooth manager and creates the UI components.
         * Does not perform an initial device scan to improve loading time.
         */
        BluetoothTab();

        /**
         * @brief Virtual destructor
         */
        virtual ~BluetoothTab();

    private:
        /**
         * @brief Update the list of displayed Bluetooth devices
         * @param devices Vector of Device objects to display
         *
         * Clears the current list of device widgets and creates new ones
         * for each device in the provided vector.
         */
        void update_device_list(const std::vector<Device> &devices);

        /**
         * @brief Update the UI based on Bluetooth state
         * @param enabled Whether Bluetooth is enabled
         *
         * Updates the Bluetooth icon, status label, and scan button sensitivity
         * based on the current Bluetooth state.
         */
        void update_bluetooth_state(bool enabled);

        /**
         * @brief Handler for Bluetooth switch toggle events
         *
         * Enables or disables Bluetooth based on the switch state.
         */
        void on_bluetooth_switch_toggled();

        /**
         * @brief Perform a delayed device scan
         *
         * Called after the tab is fully loaded and visible to the user.
         * Delays the scan by a short time to ensure UI responsiveness.
         */
        void perform_delayed_scan();

        std::shared_ptr<BluetoothManager> manager_;                   ///< Bluetooth manager for device operations
        Gtk::Box container_;                                          ///< Container for device widgets
        Gtk::Button scan_button_;                                     ///< Button to trigger device scanning
        Gtk::Switch bluetooth_switch_;                                ///< Switch to enable/disable Bluetooth
        Gtk::Label bluetooth_status_label_;                           ///< Label showing Bluetooth status (Enabled/Disabled)
        Gtk::Image bluetooth_status_icon_;                            ///< Icon showing Bluetooth status
        std::vector<std::unique_ptr<BluetoothDeviceWidget>> widgets_; ///< List of device widgets
        bool initial_scan_performed_ = false;                         ///< Flag to track if initial scan has been done
        Gtk::Label *loading_label_ = nullptr;                         ///< Loading message shown before devices are loaded
    };

} // namespace Bluetooth
