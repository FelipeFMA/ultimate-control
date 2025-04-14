/**
 * @file BluetoothManager.hpp
 * @brief Bluetooth management functionality for Ultimate Control
 *
 * This file defines the BluetoothManager class which provides an interface
 * for scanning, connecting to, and managing Bluetooth devices.
 */

#pragma once

#include <giomm.h>
#include <glibmm.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

/**
 * @namespace Bluetooth
 * @brief Contains Bluetooth-related functionality
 */
namespace Bluetooth
{

    /**
     * @struct Device
     * @brief Represents a Bluetooth device detected during scanning
     */
    struct Device
    {
        std::string name;    ///< Device name
        std::string address; ///< MAC address of the device
        int signal_strength; ///< Signal strength as a percentage (0-100)
        bool connected;      ///< Whether the device is currently connected
        bool paired;         ///< Whether the device is paired
    };

    /**
     * @class BluetoothManager
     * @brief Manages Bluetooth connections and device scanning
     *
     * Provides an interface for scanning available devices, connecting to devices,
     * and managing paired devices.
     */
    class BluetoothManager
    {
    public:
        using DeviceList = std::vector<Device>;
        using UpdateCallback = std::function<void(const DeviceList &)>;
        using StateCallback = std::function<void(bool)>;
        using ConnectionCallback = std::function<void(bool, const std::string &)>;

        BluetoothManager();
        ~BluetoothManager();

        void scan_devices();
        void scan_devices_async();

        void connect_async(const std::string &address, ConnectionCallback callback = nullptr);
        void disconnect(const std::string &address);
        void forget_device(const std::string &address);

        void enable_bluetooth();
        void disable_bluetooth();
        bool is_bluetooth_enabled() const;

        void set_update_callback(UpdateCallback cb);
        void set_state_callback(StateCallback cb);

        const DeviceList &get_devices() const;

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
        bool enabled_ = false;
        StateCallback state_callback_;
        UpdateCallback update_callback_;

        // --- Added for BlueZ D-Bus implementation ---
        void on_get_managed_objects_finished(const Glib::RefPtr<Gio::AsyncResult> &result);
        DeviceList get_devices_from_bluez() const;
    };

} // namespace Bluetooth
