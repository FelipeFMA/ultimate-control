/**
 * @file BluetoothDeviceWidget.hpp
 * @brief Widget for displaying and interacting with a Bluetooth device
 *
 * This file defines the BluetoothDeviceWidget class which displays a single
 * Bluetooth device in the UI with signal strength, connection, and pairing
 * controls. It provides functionality for connecting to devices and forgetting
 * paired devices.
 */

#pragma once

#include <gtkmm.h>
#include "BluetoothManager.hpp"
#include <memory>

namespace Bluetooth
{

    /**
     * @class BluetoothDeviceWidget
     * @brief Widget for displaying and interacting with a Bluetooth device
     *
     * Displays information about a Bluetooth device including its name,
     * signal strength, connection status, and pairing status. Provides
     * buttons for connecting/disconnecting and forgetting the device.
     */
    class BluetoothDeviceWidget : public Gtk::Box
    {
    public:
        /**
         * @brief Constructor for the Bluetooth device widget
         * @param device The device to display
         * @param manager Shared pointer to the Bluetooth manager
         *
         * Creates a widget that displays information about a Bluetooth device and
         * provides controls for connecting, disconnecting, and forgetting the device.
         */
        BluetoothDeviceWidget(const Device &device, std::shared_ptr<BluetoothManager> manager);

        /**
         * @brief Virtual destructor
         */
        virtual ~BluetoothDeviceWidget();

    private:
        void on_connect_clicked();
        void on_forget_clicked();

        std::shared_ptr<BluetoothManager> manager_; ///< Pointer to the Bluetooth manager for device operations
        Device device_;                             ///< The device this widget represents

        // Main layout containers
        Gtk::Box device_info_box_; ///< Container for device information (name, icons)
        Gtk::Box controls_box_;    ///< Container for buttons (connect, forget)

        // Device information widgets
        Gtk::Image signal_icon_;  ///< Icon showing signal strength
        Gtk::Image status_icon_;  ///< Icon showing connection/paired status
        Gtk::Image check_icon_;   ///< Check mark icon for connected devices
        Gtk::Label name_label_;   ///< Label showing the device name
        Gtk::Label signal_label_; ///< Label showing signal strength percentage
        Gtk::Label status_label_; ///< Label showing connection/paired status text

        // Control widgets
        Gtk::Button connect_button_; ///< Button to connect/disconnect from the device
        Gtk::Button forget_button_;  ///< Button to forget the device
    };

} // namespace Bluetooth
