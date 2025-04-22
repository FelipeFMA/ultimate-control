/**
 * @file BluetoothManager.cpp
 * @brief BlueZ D-Bus implementation of BluetoothManager for Ultimate Control
 *
 * This file provides real implementations for BluetoothManager methods,
 * discovering devices via the BlueZ D-Bus API.
 */

#include "BluetoothManager.hpp"
#include <giomm.h>
#include <glibmm.h>
#include <iostream>
#include <map>
#include <mutex>

namespace Bluetooth
{

    // Helper: Map RSSI (dBm) to 0-100% (simple linear mapping, clamp to [0,100])
    static int rssi_to_percent(int rssi)
    {
        // Typical RSSI range: -100 (weak) to -40 (strong)
        if (rssi <= -100)
            return 0;
        if (rssi >= -40)
            return 100;
        return (rssi + 100) * 100 / 60;
    }

    // Helper: Estimate signal strength based on other properties when RSSI is not available
    static int estimate_signal_strength(const std::map<std::string, Glib::VariantBase> &props)
    {
        // Check if the device is connected - connected devices likely have good signal
        auto connected_it = props.find("Connected");
        bool is_connected = (connected_it != props.end()) ? Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(connected_it->second).get() : false;

        if (is_connected)
        {
            // Connected devices are assumed to have at least moderate signal strength
            return 75; // 75% signal for connected devices
        }

        // Check if the device is paired - paired devices were in range recently
        auto paired_it = props.find("Paired");
        bool is_paired = (paired_it != props.end()) ? Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(paired_it->second).get() : false;

        if (is_paired)
        {
            return 60; // 60% signal for paired but not connected devices
        }

        // For other detected devices, assume a moderate signal strength
        // since they must be in range to be detected at all
        return 50;
    }

    // PIMPL idiom: real implementation
    class BluetoothManager::Impl
    {
    public:
        Impl()
        {
            try
            {
                connection = Gio::DBus::Connection::get_sync(Gio::DBus::BUS_TYPE_SYSTEM);
            }
            catch (const Glib::Error &ex)
            {
                std::cerr << "Failed to connect to system D-Bus: " << ex.what() << std::endl;
                connection.reset();
            }
        }

        Glib::RefPtr<Gio::DBus::Connection> connection;
        std::mutex mutex;
        DeviceList last_devices;
    };

    BluetoothManager::BluetoothManager() : impl_(new Impl), enabled_(true) {}
    BluetoothManager::~BluetoothManager() = default;

    void BluetoothManager::scan_devices()
    {
        // Synchronous scan (not recommended for UI)
        if (!enabled_ || !impl_->connection)
            return;

        DeviceList devices = get_devices_from_bluez();
        {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->last_devices = devices;
        }
        if (update_callback_)
            update_callback_(devices);
    }

    void BluetoothManager::scan_devices_async()
    {
        if (!enabled_ || !impl_->connection || !update_callback_)
            return;

        // Fallback: Run synchronous device scan in a background thread, then post result to main thread
        std::thread([this]()
                    {
            DeviceList devices = get_devices_from_bluez();
            {
                std::lock_guard<std::mutex> lock(impl_->mutex);
                impl_->last_devices = devices;
            }
            if (update_callback_) {
                Glib::signal_idle().connect_once([this, devices]() {
                    update_callback_(devices);
                });
            } })
            .detach();
    }

    void BluetoothManager::on_get_managed_objects_finished(const Glib::RefPtr<Gio::AsyncResult> &result)
    {
        DeviceList devices;
        try
        {
            Glib::VariantContainerBase v = impl_->connection->call_finish(result);
            // v is a{oa{sa{sv}}}
            auto managed = Glib::VariantBase::cast_dynamic<Glib::Variant<std::map<Glib::DBusObjectPathString, std::map<std::string, std::map<std::string, Glib::VariantBase>>>>>(v);
            auto objects = managed.get();
            for (const auto &obj : objects)
            {
                const auto &object_path = obj.first;
                const auto &interfaces = obj.second;
                auto it = interfaces.find("org.bluez.Device1");
                if (it != interfaces.end())
                {
                    const auto &props = it->second;
                    Device dev;
                    auto addr_it = props.find("Address");
                    if (addr_it != props.end())
                        dev.address = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(addr_it->second).get();
                    auto name_it = props.find("Name");
                    if (name_it != props.end())
                        dev.name = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(name_it->second).get();
                    auto paired_it = props.find("Paired");
                    dev.paired = (paired_it != props.end()) ? Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(paired_it->second).get() : false;
                    auto connected_it = props.find("Connected");
                    dev.connected = (connected_it != props.end()) ? Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(connected_it->second).get() : false;
                    auto rssi_it = props.find("RSSI");
                    if (rssi_it != props.end())
                    {
                        int rssi = Glib::VariantBase::cast_dynamic<Glib::Variant<int16_t>>(rssi_it->second).get();
                        dev.signal_strength = rssi_to_percent(rssi);
                    }
                    else
                    {
                        // RSSI not available, use fallback estimation
                        dev.signal_strength = estimate_signal_strength(props);
                    }
                    devices.push_back(dev);
                }
            }
        }
        catch (const Glib::Error &ex)
        {
            std::cerr << "D-Bus error: " << ex.what() << std::endl;
        }
        {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->last_devices = devices;
        }
        if (update_callback_)
            update_callback_(devices);
    }

    BluetoothManager::DeviceList BluetoothManager::get_devices_from_bluez() const
    {
        BluetoothManager::DeviceList devices;
        if (!impl_->connection)
        {
            std::cerr << "No D-Bus connection available" << std::endl;
            return devices;
        }

        std::cerr << "Scanning for Bluetooth devices..." << std::endl;

        try
        {
            // Check if BlueZ is available on the system bus
            bool bluez_found = false;
            try
            {
                auto reply = impl_->connection->call_sync(
                    "/org/freedesktop/DBus",
                    "org.freedesktop.DBus",
                    "ListNames",
                    Glib::VariantContainerBase(),
                    "org.freedesktop.DBus");
                auto names_variant = Glib::VariantBase::cast_dynamic<Glib::Variant<std::vector<Glib::ustring>>>(reply.get_child(0));
                auto names = names_variant.get();
                for (const auto &name : names)
                {
                    if (name == "org.bluez")
                    {
                        bluez_found = true;
                        break;
                    }
                }
            }
            catch (const Glib::Error &ex)
            {
                std::cerr << "Failed to query D-Bus for available names: " << ex.what() << std::endl;
            }
            if (!bluez_found)
            {
                std::cerr << "BlueZ service (org.bluez) not found on system D-Bus. Is bluetoothd running?" << std::endl;
                return devices;
            }

            // Fallback: enumerate all objects under /org/bluez and look for org.bluez.Device1
            auto reply = impl_->connection->call_sync(
                "/org/bluez",
                "org.freedesktop.DBus.Introspectable",
                "Introspect",
                Glib::VariantContainerBase(),
                "org.bluez");
            std::string xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(reply.get_child(0)).get();

            // Parse the XML to find all adapter and device object paths
            std::vector<std::string> device_paths;
            // Simple XML parsing: look for <node name="hciX"> and <node name="dev_...">
            size_t pos = 0;

            std::cerr << "BlueZ XML: " << xml.substr(0, 100) << "..." << std::endl;

            while ((pos = xml.find("<node name=\"", pos)) != std::string::npos)
            {
                pos += 12;
                size_t end = xml.find("\"", pos);
                if (end == std::string::npos)
                    break;
                std::string node = xml.substr(pos, end - pos);
                std::cerr << "Found node: " << node << std::endl;

                if (node.find("hci") == 0)
                {
                    // Introspect this adapter
                    std::string adapter_path = std::string("/org/bluez/") + node;
                    std::cerr << "Introspecting adapter: " << adapter_path << std::endl;

                    auto adapter_reply = impl_->connection->call_sync(
                        adapter_path,
                        "org.freedesktop.DBus.Introspectable",
                        "Introspect",
                        Glib::VariantContainerBase(),
                        "org.bluez");
                    std::string adapter_xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(adapter_reply.get_child(0)).get();

                    std::cerr << "Adapter XML: " << adapter_xml.substr(0, 100) << "..." << std::endl;

                    size_t pos2 = 0;
                    while ((pos2 = adapter_xml.find("<node name=\"", pos2)) != std::string::npos)
                    {
                        pos2 += 12;
                        size_t end2 = adapter_xml.find("\"", pos2);
                        if (end2 == std::string::npos)
                            break;
                        std::string dev_node = adapter_xml.substr(pos2, end2 - pos2);
                        std::cerr << "Found device node: " << dev_node << std::endl;

                        if (dev_node.find("dev_") == 0)
                        {
                            std::string dev_path = adapter_path + "/" + dev_node;
                            std::cerr << "Adding device path: " << dev_path << std::endl;
                            device_paths.push_back(dev_path);
                        }
                    }
                }
            }

            std::cerr << "Found " << device_paths.size() << " device paths" << std::endl;

            // For each device path, get properties from org.bluez.Device1
            for (const auto &dev_path : device_paths)
            {
                try
                {
                    std::cerr << "Getting properties for device: " << dev_path << std::endl;

                    auto props_reply = impl_->connection->call_sync(
                        dev_path,
                        "org.freedesktop.DBus.Properties",
                        "GetAll",
                        Glib::VariantContainerBase::create_tuple({Glib::Variant<Glib::ustring>::create("org.bluez.Device1")}),
                        "org.bluez");
                    auto props_variant = Glib::VariantBase::cast_dynamic<Glib::Variant<std::map<std::string, Glib::VariantBase>>>(props_reply.get_child(0));
                    const auto &props = props_variant.get();

                    std::cerr << "Got properties for device: " << dev_path << std::endl;

                    Device dev;
                    auto addr_it = props.find("Address");
                    if (addr_it != props.end())
                    {
                        dev.address = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(addr_it->second).get();
                        std::cerr << "  Address: " << dev.address << std::endl;
                    }

                    auto name_it = props.find("Name");
                    if (name_it != props.end())
                    {
                        dev.name = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(name_it->second).get();
                        std::cerr << "  Name: " << dev.name << std::endl;
                    }
                    else
                    {
                        std::cerr << "  No name found" << std::endl;
                    }

                    auto paired_it = props.find("Paired");
                    dev.paired = (paired_it != props.end()) ? Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(paired_it->second).get() : false;
                    std::cerr << "  Paired: " << (dev.paired ? "yes" : "no") << std::endl;

                    auto connected_it = props.find("Connected");
                    dev.connected = (connected_it != props.end()) ? Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(connected_it->second).get() : false;
                    std::cerr << "  Connected: " << (dev.connected ? "yes" : "no") << std::endl;

                    auto rssi_it = props.find("RSSI");
                    if (rssi_it != props.end())
                    {
                        int rssi = Glib::VariantBase::cast_dynamic<Glib::Variant<int16_t>>(rssi_it->second).get();
                        dev.signal_strength = rssi_to_percent(rssi);
                        std::cerr << "  RSSI: " << rssi << " (" << dev.signal_strength << "%)" << std::endl;
                    }
                    else
                    {
                        // RSSI not available, use fallback estimation
                        dev.signal_strength = estimate_signal_strength(props);
                        std::cerr << "  No RSSI information, using estimated signal strength: " << dev.signal_strength << "%" << std::endl;
                    }

                    devices.push_back(dev);
                    std::cerr << "Added device to list: " << dev.name << " (" << dev.address << ")" << std::endl;
                }
                catch (const Glib::Error &ex)
                {
                    std::cerr << "Failed to get properties for " << dev_path << ": " << ex.what() << std::endl;
                }
            }

            std::cerr << "Total devices found: " << devices.size() << std::endl;
        }
        catch (const Glib::Error &ex)
        {
            std::cerr << "D-Bus error: " << ex.what() << std::endl;
        }
        return devices;
    }

    void BluetoothManager::connect_async(const std::string &address, ConnectionCallback callback)
    {
        if (!enabled_ || !impl_->connection)
        {
            if (callback)
                callback(false, address);
            return;
        }

        std::cout << "Attempting to connect to device: " << address << std::endl;

        // Run in a background thread to avoid blocking the UI
        std::thread([this, address, callback]()
                    {
            bool success = false;
            try {
                // Find the device path from the address
                std::string device_path;
                auto devices = get_devices_from_bluez();
                for (const auto& dev : devices) {
                    if (dev.address == address) {
                        // Found the device, now find its path
                        auto reply = impl_->connection->call_sync(
                            "/org/bluez",
                            "org.freedesktop.DBus.Introspectable",
                            "Introspect",
                            Glib::VariantContainerBase(),
                            "org.bluez");
                        std::string xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(reply.get_child(0)).get();

                        // Parse the XML to find the adapter
                        size_t pos = 0;
                        while ((pos = xml.find("<node name=\"", pos)) != std::string::npos) {
                            pos += 12;
                            size_t end = xml.find("\"", pos);
                            if (end == std::string::npos)
                                break;
                            std::string node = xml.substr(pos, end - pos);
                            if (node.find("hci") == 0) {
                                // Found an adapter, check if it has our device
                                std::string adapter_path = std::string("/org/bluez/") + node;
                                auto adapter_reply = impl_->connection->call_sync(
                                    adapter_path,
                                    "org.freedesktop.DBus.Introspectable",
                                    "Introspect",
                                    Glib::VariantContainerBase(),
                                    "org.bluez");
                                std::string adapter_xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(adapter_reply.get_child(0)).get();

                                // Look for the device in this adapter
                                std::string dev_addr_underscores = address;
                                // Replace colons with underscores in the address
                                for (size_t i = 0; i < dev_addr_underscores.length(); ++i) {
                                    if (dev_addr_underscores[i] == ':') {
                                        dev_addr_underscores[i] = '_';
                                    }
                                }

                                std::string dev_node = "dev_" + dev_addr_underscores;
                                if (adapter_xml.find(dev_node) != std::string::npos) {
                                    device_path = adapter_path + "/" + dev_node;
                                    break;
                                }
                            }
                        }

                        break;
                    }
                }

                if (device_path.empty()) {
                    std::cerr << "Could not find device path for address: " << address << std::endl;
                    success = false;
                } else {
                    std::cout << "Found device path: " << device_path << std::endl;

                    // Call Connect method on the device
                    auto connect_reply = impl_->connection->call_sync(
                        device_path,
                        "org.bluez.Device1",
                        "Connect",
                        Glib::VariantContainerBase(),
                        "org.bluez");

                    std::cout << "Connect call succeeded for device: " << address << std::endl;
                    success = true;
                }
            } catch (const Glib::Error &ex) {
                std::cerr << "Failed to connect to device " << address << ": " << ex.what() << std::endl;
                success = false;
            }

            // Call the callback on the main thread
            if (callback) {
                Glib::signal_idle().connect_once([callback, success, address]() {
                    callback(success, address);
                });
            }

            // Refresh the device list to update the UI
            scan_devices_async(); })
            .detach();
    }

    void BluetoothManager::disconnect(const std::string &address)
    {
        if (!enabled_ || !impl_->connection)
            return;

        std::cout << "Attempting to disconnect from device: " << address << std::endl;

        // Run in a background thread to avoid blocking the UI
        std::thread([this, address]()
                    {
            try {
                // Find the device path from the address
                std::string device_path;
                auto devices = get_devices_from_bluez();
                for (const auto& dev : devices) {
                    if (dev.address == address) {
                        // Found the device, now find its path
                        auto reply = impl_->connection->call_sync(
                            "/org/bluez",
                            "org.freedesktop.DBus.Introspectable",
                            "Introspect",
                            Glib::VariantContainerBase(),
                            "org.bluez");
                        std::string xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(reply.get_child(0)).get();

                        // Parse the XML to find the adapter
                        size_t pos = 0;
                        while ((pos = xml.find("<node name=\"", pos)) != std::string::npos) {
                            pos += 12;
                            size_t end = xml.find("\"", pos);
                            if (end == std::string::npos)
                                break;
                            std::string node = xml.substr(pos, end - pos);
                            if (node.find("hci") == 0) {
                                // Found an adapter, check if it has our device
                                std::string adapter_path = std::string("/org/bluez/") + node;
                                auto adapter_reply = impl_->connection->call_sync(
                                    adapter_path,
                                    "org.freedesktop.DBus.Introspectable",
                                    "Introspect",
                                    Glib::VariantContainerBase(),
                                    "org.bluez");
                                std::string adapter_xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(adapter_reply.get_child(0)).get();

                                // Look for the device in this adapter
                                std::string dev_addr_underscores = address;
                                // Replace colons with underscores in the address
                                for (size_t i = 0; i < dev_addr_underscores.length(); ++i) {
                                    if (dev_addr_underscores[i] == ':') {
                                        dev_addr_underscores[i] = '_';
                                    }
                                }

                                std::string dev_node = "dev_" + dev_addr_underscores;
                                if (adapter_xml.find(dev_node) != std::string::npos) {
                                    device_path = adapter_path + "/" + dev_node;
                                    break;
                                }
                            }
                        }

                        break;
                    }
                }

                if (device_path.empty()) {
                    std::cerr << "Could not find device path for address: " << address << std::endl;
                } else {
                    std::cout << "Found device path: " << device_path << std::endl;

                    // Call Disconnect method on the device
                    auto disconnect_reply = impl_->connection->call_sync(
                        device_path,
                        "org.bluez.Device1",
                        "Disconnect",
                        Glib::VariantContainerBase(),
                        "org.bluez");

                    std::cout << "Disconnect call succeeded for device: " << address << std::endl;
                }
            } catch (const Glib::Error &ex) {
                std::cerr << "Failed to disconnect from device " << address << ": " << ex.what() << std::endl;
            }

            // Refresh the device list to update the UI
            scan_devices_async(); })
            .detach();
    }

    void BluetoothManager::forget_device(const std::string &address)
    {
        if (!enabled_ || !impl_->connection)
            return;

        std::cout << "Attempting to forget device: " << address << std::endl;

        // Run in a background thread to avoid blocking the UI
        std::thread([this, address]()
                    {
            try {
                // Find the device path from the address
                std::string device_path;
                auto devices = get_devices_from_bluez();
                for (const auto& dev : devices) {
                    if (dev.address == address) {
                        // Found the device, now find its path
                        auto reply = impl_->connection->call_sync(
                            "/org/bluez",
                            "org.freedesktop.DBus.Introspectable",
                            "Introspect",
                            Glib::VariantContainerBase(),
                            "org.bluez");
                        std::string xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(reply.get_child(0)).get();

                        // Parse the XML to find the adapter
                        size_t pos = 0;
                        while ((pos = xml.find("<node name=\"", pos)) != std::string::npos) {
                            pos += 12;
                            size_t end = xml.find("\"", pos);
                            if (end == std::string::npos)
                                break;
                            std::string node = xml.substr(pos, end - pos);
                            if (node.find("hci") == 0) {
                                // Found an adapter, check if it has our device
                                std::string adapter_path = std::string("/org/bluez/") + node;
                                auto adapter_reply = impl_->connection->call_sync(
                                    adapter_path,
                                    "org.freedesktop.DBus.Introspectable",
                                    "Introspect",
                                    Glib::VariantContainerBase(),
                                    "org.bluez");
                                std::string adapter_xml = Glib::VariantBase::cast_dynamic<Glib::Variant<std::string>>(adapter_reply.get_child(0)).get();

                                // Look for the device in this adapter
                                std::string dev_addr_underscores = address;
                                // Replace colons with underscores in the address
                                for (size_t i = 0; i < dev_addr_underscores.length(); ++i) {
                                    if (dev_addr_underscores[i] == ':') {
                                        dev_addr_underscores[i] = '_';
                                    }
                                }

                                std::string dev_node = "dev_" + dev_addr_underscores;
                                if (adapter_xml.find(dev_node) != std::string::npos) {
                                    device_path = adapter_path + "/" + dev_node;
                                    break;
                                }
                            }
                        }

                        break;
                    }
                }

                if (device_path.empty()) {
                    std::cerr << "Could not find device path for address: " << address << std::endl;
                } else {
                    std::cout << "Found device path: " << device_path << std::endl;

                    // First make sure the device is disconnected
                    try {
                        auto disconnect_reply = impl_->connection->call_sync(
                            device_path,
                            "org.bluez.Device1",
                            "Disconnect",
                            Glib::VariantContainerBase(),
                            "org.bluez");
                        std::cout << "Disconnected device before forgetting: " << address << std::endl;
                    } catch (const Glib::Error &ex) {
                        // Ignore errors here, device might not be connected
                    }

                    // Call RemoveDevice method on the adapter
                    std::string adapter_path = device_path.substr(0, device_path.find_last_of('/'));
                    auto forget_reply = impl_->connection->call_sync(
                        adapter_path,
                        "org.bluez.Adapter1",
                        "RemoveDevice",
                        Glib::VariantContainerBase::create_tuple({Glib::Variant<Glib::DBusObjectPathString>::create(device_path)}),
                        "org.bluez");

                    std::cout << "Forget call succeeded for device: " << address << std::endl;
                }
            } catch (const Glib::Error &ex) {
                std::cerr << "Failed to forget device " << address << ": " << ex.what() << std::endl;
            }

            // Refresh the device list to update the UI
            scan_devices_async(); })
            .detach();
    }

    void BluetoothManager::enable_bluetooth()
    {
        if (!enabled_)
        {
            enabled_ = true;
            if (state_callback_)
                state_callback_(enabled_);

            // Perform a scan to refresh the device list
            scan_devices_async();
        }
    }
    void BluetoothManager::disable_bluetooth()
    {
        if (enabled_)
        {
            enabled_ = false;
            if (state_callback_)
                state_callback_(enabled_);

            // Clear device list since Bluetooth is now disabled
            {
                std::lock_guard<std::mutex> lock(impl_->mutex);
                impl_->last_devices.clear();
            }

            // Update the UI to show the "Bluetooth is turned off" message
            if (update_callback_)
            {
                DeviceList empty_devices;
                update_callback_(empty_devices);
            }
        }
    }
    bool BluetoothManager::is_bluetooth_enabled() const { return enabled_; }

    void BluetoothManager::set_update_callback(UpdateCallback cb)
    {
        update_callback_ = cb;
        // Immediately provide devices if enabled
        if (enabled_ && update_callback_)
        {
            DeviceList devices;
            {
                std::lock_guard<std::mutex> lock(impl_->mutex);
                devices = impl_->last_devices;
            }
            if (devices.empty())
            {
                devices = get_devices_from_bluez();
                {
                    std::lock_guard<std::mutex> lock(impl_->mutex);
                    impl_->last_devices = devices;
                }
            }
            update_callback_(devices);
        }
    }

    void BluetoothManager::set_state_callback(StateCallback cb)
    {
        state_callback_ = cb;
        if (state_callback_)
            state_callback_(enabled_);
    }

    const BluetoothManager::DeviceList &BluetoothManager::get_devices() const
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        return impl_->last_devices;
    }

} // namespace Bluetooth
