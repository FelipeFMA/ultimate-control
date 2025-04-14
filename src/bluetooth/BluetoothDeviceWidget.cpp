/**
 * @file BluetoothDeviceWidget.cpp
 * @brief Implementation of BluetoothDeviceWidget for Ultimate Control
 *
 * This file provides the implementation for BluetoothDeviceWidget, which displays
 * information about a Bluetooth device and provides controls for connecting and
 * forgetting the device.
 */

#include "BluetoothDeviceWidget.hpp"
#include <iostream>

namespace Bluetooth
{

    BluetoothDeviceWidget::BluetoothDeviceWidget(const Device &device, std::shared_ptr<BluetoothManager> manager)
        : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10), manager_(manager), device_(device),
          device_info_box_(Gtk::ORIENTATION_HORIZONTAL, 5), controls_box_(Gtk::ORIENTATION_HORIZONTAL, 5)
    {
        set_margin_top(5);
        set_margin_bottom(5);
        set_margin_start(10);
        set_margin_end(10);

        // Set up the device info box (left side)
        device_info_box_.set_hexpand(true);

        // Signal strength icon
        if (device_.signal_strength > 66)
        {
            signal_icon_.set_from_icon_name("network-wireless-signal-excellent-symbolic", Gtk::ICON_SIZE_MENU);
        }
        else if (device_.signal_strength > 33)
        {
            signal_icon_.set_from_icon_name("network-wireless-signal-good-symbolic", Gtk::ICON_SIZE_MENU);
        }
        else
        {
            signal_icon_.set_from_icon_name("network-wireless-signal-weak-symbolic", Gtk::ICON_SIZE_MENU);
        }

        // Status icon (connected/paired)
        if (device_.connected)
        {
            // For connected devices, use the bluetooth-active-symbolic icon (with dots)
            status_icon_.set_from_icon_name("bluetooth-active-symbolic", Gtk::ICON_SIZE_MENU);
            status_label_.set_text("Connected");
            // Set check mark icon for connected devices
            check_icon_.set_from_icon_name("object-select-symbolic", Gtk::ICON_SIZE_MENU);
        }
        else if (device_.paired)
        {
            // For paired but not connected devices
            status_icon_.set_from_icon_name("bluetooth-symbolic", Gtk::ICON_SIZE_MENU);
            status_label_.set_text("Paired");
            // No check mark for non-connected devices
            check_icon_.clear();
        }
        else
        {
            status_icon_.set_from_icon_name("bluetooth-disabled-symbolic", Gtk::ICON_SIZE_MENU);
            status_label_.set_text("Not paired");
            // No check mark for non-connected devices
            check_icon_.clear();
        }

        // Device name
        name_label_.set_text(device_.name.empty() ? "Unknown Device" : device_.name);
        name_label_.set_halign(Gtk::ALIGN_START);
        name_label_.set_hexpand(true);

        // Signal strength percentage
        signal_label_.set_text(std::to_string(device_.signal_strength) + "%");

        // Add all info widgets to the info box
        device_info_box_.pack_start(status_icon_, Gtk::PACK_SHRINK);
        device_info_box_.pack_start(name_label_, Gtk::PACK_EXPAND_WIDGET);
        // Add check mark icon after the name for connected devices
        device_info_box_.pack_start(check_icon_, Gtk::PACK_SHRINK);
        device_info_box_.pack_end(signal_label_, Gtk::PACK_SHRINK);
        device_info_box_.pack_end(signal_icon_, Gtk::PACK_SHRINK);

        // Set up the controls box (right side)
        // Connect/disconnect button
        if (device_.connected)
        {
            connect_button_.set_label("Disconnect");
        }
        else
        {
            connect_button_.set_label("Connect");
        }
        connect_button_.signal_clicked().connect(sigc::mem_fun(*this, &BluetoothDeviceWidget::on_connect_clicked));

        // Forget button (only for paired devices)
        forget_button_.set_label("Forget");
        forget_button_.set_sensitive(device_.paired);
        forget_button_.signal_clicked().connect(sigc::mem_fun(*this, &BluetoothDeviceWidget::on_forget_clicked));

        // Add buttons to controls box
        controls_box_.pack_start(connect_button_, Gtk::PACK_SHRINK);
        controls_box_.pack_start(forget_button_, Gtk::PACK_SHRINK);

        // Add both boxes to the main widget
        pack_start(device_info_box_, Gtk::PACK_EXPAND_WIDGET);
        pack_end(controls_box_, Gtk::PACK_SHRINK);

        show_all_children();

        std::cout << "Created widget for device: " << device_.name << " (" << device_.address << ")" << std::endl;
    }

    BluetoothDeviceWidget::~BluetoothDeviceWidget() {}

    void BluetoothDeviceWidget::on_connect_clicked()
    {
        if (device_.connected)
        {
            std::cout << "Disconnecting from " << device_.name << std::endl;
            manager_->disconnect(device_.address);
        }
        else
        {
            std::cout << "Connecting to " << device_.name << std::endl;
            manager_->connect_async(device_.address, [this](bool success, const std::string &addr)
                                    {
                if (success) {
                    std::cout << "Successfully connected to " << addr << std::endl;
                } else {
                    std::cout << "Failed to connect to " << addr << std::endl;
                } });
        }
    }

    void BluetoothDeviceWidget::on_forget_clicked()
    {
        std::cout << "Forgetting device " << device_.name << std::endl;
        manager_->forget_device(device_.address);
    }

} // namespace Bluetooth
