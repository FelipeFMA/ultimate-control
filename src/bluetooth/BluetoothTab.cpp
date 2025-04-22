/**
 * @file BluetoothTab.cpp
 * @brief Implementation of the Bluetooth tab for the Ultimate Control application
 *
 * This file implements the BluetoothTab class which provides a user interface
 * for scanning, viewing, and connecting to Bluetooth devices.
 */

#include "BluetoothTab.hpp"
#include <iostream>
#include <algorithm>

namespace Bluetooth
{

    BluetoothTab::BluetoothTab()
        : manager_(std::make_shared<BluetoothManager>()),
          container_(Gtk::ORIENTATION_VERTICAL, 10)
    {
        // Explicitly defined constructor to ensure linker finds the symbol
        set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

        Gtk::Box *main_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
        main_box->set_margin_start(10);
        main_box->set_margin_end(10);
        main_box->set_margin_top(10);
        main_box->set_margin_bottom(10);
        add(*main_box);

        Gtk::Box *header_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));

        // Bluetooth status icon
        bluetooth_status_icon_.set_from_icon_name("bluetooth-active-symbolic", Gtk::ICON_SIZE_DIALOG);
        header_box->pack_start(bluetooth_status_icon_, Gtk::PACK_SHRINK);

        // Title label
        Gtk::Label *title = Gtk::manage(new Gtk::Label());
        title->set_markup("<span size='large' weight='bold'>Available Devices</span>");
        title->set_halign(Gtk::ALIGN_START);
        title->set_valign(Gtk::ALIGN_CENTER);
        header_box->pack_start(*title, Gtk::PACK_EXPAND_WIDGET);

        // Controls (toggle and scan)
        Gtk::Box *controls_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5));
        Gtk::Box *toggle_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));

        Gtk::Label *toggle_label = Gtk::manage(new Gtk::Label("Bluetooth:"));
        bluetooth_status_label_.set_text("Enabled");
        bluetooth_switch_.set_active(manager_->is_bluetooth_enabled());
        bluetooth_switch_.set_tooltip_text("Enable/Disable Bluetooth");
        bluetooth_switch_.set_can_focus(false); // Prevent tab navigation to this switch

        toggle_box->pack_start(*toggle_label, Gtk::PACK_SHRINK);
        toggle_box->pack_start(bluetooth_switch_, Gtk::PACK_SHRINK);
        toggle_box->pack_start(bluetooth_status_label_, Gtk::PACK_SHRINK);

        scan_button_.set_image_from_icon_name("view-refresh-symbolic", Gtk::ICON_SIZE_BUTTON);
        scan_button_.set_label("Scan");
        scan_button_.set_always_show_image(true);
        scan_button_.set_sensitive(manager_->is_bluetooth_enabled());
        scan_button_.set_can_focus(false); // Prevent tab navigation to this button

        controls_box->pack_start(*toggle_box, Gtk::PACK_SHRINK);
        controls_box->pack_start(scan_button_, Gtk::PACK_SHRINK);

        header_box->pack_end(*controls_box, Gtk::PACK_SHRINK);

        Gtk::Separator *separator = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
        main_box->pack_start(*header_box, Gtk::PACK_SHRINK);
        main_box->pack_start(*separator, Gtk::PACK_SHRINK);

        Gtk::ScrolledWindow *devices_scroll = Gtk::manage(new Gtk::ScrolledWindow());
        devices_scroll->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        devices_scroll->add(container_);
        main_box->pack_start(*devices_scroll, Gtk::PACK_EXPAND_WIDGET);

        // Scan button handler
        scan_button_.signal_clicked().connect([this]()
                                              {
            scan_button_.set_sensitive(false);
            scan_button_.set_label("Scanning...");
            manager_->scan_devices_async();
            Glib::signal_timeout().connect_once([this]() {
                scan_button_.set_sensitive(true);
                scan_button_.set_label("Scan");
            }, 2000); });

        // Bluetooth switch handler
        bluetooth_switch_.property_active().signal_changed().connect(sigc::mem_fun(*this, &BluetoothTab::on_bluetooth_switch_toggled));

        // Register callbacks
        manager_->set_update_callback([this](const std::vector<Device> &devices)
                                      { update_device_list(devices); });
        manager_->set_state_callback([this](bool enabled)
                                     { update_bluetooth_state(enabled); });

        update_bluetooth_state(manager_->is_bluetooth_enabled());

        loading_label_ = Gtk::manage(new Gtk::Label("Loading devices..."));
        loading_label_->set_margin_top(20);
        loading_label_->set_margin_bottom(20);
        container_.pack_start(*loading_label_, Gtk::PACK_SHRINK);

        show_all_children();
        std::cout << "Bluetooth tab loaded!" << std::endl;

        Glib::signal_timeout().connect_once(
            sigc::mem_fun(*this, &BluetoothTab::perform_delayed_scan), 100);
    }

    BluetoothTab::~BluetoothTab() {}

    void BluetoothTab::update_bluetooth_state(bool enabled)
    {
        bluetooth_switch_.set_active(enabled);
        bluetooth_status_label_.set_text(enabled ? "Enabled" : "Disabled");
        scan_button_.set_sensitive(enabled);

        if (enabled)
        {
            bluetooth_status_icon_.set_from_icon_name("bluetooth-active-symbolic", Gtk::ICON_SIZE_DIALOG);
        }
        else
        {
            bluetooth_status_icon_.set_from_icon_name("bluetooth-disabled-symbolic", Gtk::ICON_SIZE_DIALOG);

            // When Bluetooth is disabled, update the device list to hide all devices
            // and show the "Bluetooth is turned off" message
            update_device_list(manager_->get_devices());
        }
    }

    void BluetoothTab::on_bluetooth_switch_toggled()
    {
        bool enabled = bluetooth_switch_.get_active();
        bluetooth_switch_.set_sensitive(false);

        if (enabled)
        {
            manager_->enable_bluetooth();
        }
        else
        {
            manager_->disable_bluetooth();
        }

        Glib::signal_timeout().connect_once([this]()
                                            { bluetooth_switch_.set_sensitive(true); }, 1000);
    }

    void BluetoothTab::update_device_list(const std::vector<Device> &devices)
    {
        for (auto &widget : widgets_)
        {
            container_.remove(*widget);
        }
        widgets_.clear();

        if (loading_label_ != nullptr)
        {
            container_.remove(*loading_label_);
            loading_label_ = nullptr;
        }

        // If Bluetooth is disabled, show a message and don't display any devices
        if (!manager_->is_bluetooth_enabled())
        {
            Gtk::Label *bluetooth_off = Gtk::manage(new Gtk::Label("Bluetooth is turned off"));
            bluetooth_off->set_margin_top(20);
            bluetooth_off->set_margin_bottom(20);
            container_.pack_start(*bluetooth_off, Gtk::PACK_SHRINK);
        }
        // If Bluetooth is enabled but no devices found
        else if (devices.empty())
        {
            Gtk::Label *no_devices = Gtk::manage(new Gtk::Label("No Bluetooth devices found"));
            no_devices->set_margin_top(20);
            no_devices->set_margin_bottom(20);
            container_.pack_start(*no_devices, Gtk::PACK_SHRINK);
        }
        else
        {
            // Sort devices to keep connected ones at the top
            std::vector<Device> sorted_devices = devices;
            std::sort(sorted_devices.begin(), sorted_devices.end(), [](const Device &a, const Device &b)
                      {
                if(a.connected != b.connected)
                    return a.connected;
                return a.name < b.name; });

            for (const auto &dev : sorted_devices)
            {
                auto widget = std::make_unique<BluetoothDeviceWidget>(dev, manager_);
                container_.pack_start(*widget, Gtk::PACK_SHRINK);
                widgets_.push_back(std::move(widget));
            }
        }

        show_all_children();
    }

    void BluetoothTab::perform_delayed_scan()
    {
        if (initial_scan_performed_)
        {
            return;
        }

        initial_scan_performed_ = true;

        if (manager_->is_bluetooth_enabled())
        {
            scan_button_.set_sensitive(false);
            scan_button_.set_label("Scanning...");
            manager_->scan_devices_async();
            Glib::signal_timeout().connect_once([this]()
                                                {
                scan_button_.set_sensitive(true);
                scan_button_.set_label("Scan"); }, 2000);
        }
    }

} // namespace Bluetooth
