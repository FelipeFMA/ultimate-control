#include "WifiNetworkWidget.hpp"
#include <gtkmm/messagedialog.h>
#include <iostream>

namespace Wifi {

WifiNetworkWidget::WifiNetworkWidget(const Network& network, std::shared_ptr<WifiManager> manager)
: Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5),
  manager_(std::move(manager)),
  network_(network),
  network_info_box_(Gtk::ORIENTATION_HORIZONTAL, 8),
  controls_box_(Gtk::ORIENTATION_HORIZONTAL, 8),
  ssid_label_(network.ssid),
  signal_label_(std::to_string(network.signal_strength) + "%"),
  status_label_(network.connected ? "Connected" : (network.secured ? "Secured" : "Open")),
  connect_button_(),
  forget_button_()
{
  // Debug: Print the signal strength in the constructor
  std::cout << "WifiNetworkWidget constructor for " << network.ssid << ", signal strength: " << network.signal_strength << "%" << std::endl;
    // Set up the main container
    set_margin_start(10);
    set_margin_end(10);
    set_margin_top(8);
    set_margin_bottom(8);

    // Add a frame around the widget for better visual separation
    Gtk::Frame* frame = Gtk::manage(new Gtk::Frame());
    frame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);

    // Create a box inside the frame
    Gtk::Box* inner_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
    inner_box->set_margin_start(10);
    inner_box->set_margin_end(10);
    inner_box->set_margin_top(10);
    inner_box->set_margin_bottom(10);
    frame->add(*inner_box);

    // Set up the network info box
    update_signal_icon(network.signal_strength);
    update_security_icon(network.secured);
    update_connection_status(network.connected);

    // Make the SSID label bold and larger
    Pango::AttrList attrs;
    auto font_desc = Pango::FontDescription("Bold");
    auto attr = Pango::Attribute::create_attr_font_desc(font_desc);
    attrs.insert(attr);
    ssid_label_.set_attributes(attrs);

    // Add network info widgets to the info box
    network_info_box_.pack_start(signal_icon_, Gtk::PACK_SHRINK);
    network_info_box_.pack_start(ssid_label_, Gtk::PACK_SHRINK);
    network_info_box_.pack_start(security_icon_, Gtk::PACK_SHRINK);
    network_info_box_.pack_start(status_icon_, Gtk::PACK_SHRINK);

    // Add signal strength label
    Gtk::Box* signal_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 4));
    Gtk::Label* signal_prefix = Gtk::manage(new Gtk::Label("Signal Strength:"));
    signal_box->pack_start(*signal_prefix, Gtk::PACK_SHRINK);

    // Set the signal label text directly
    signal_label_.set_text(std::to_string(network_.signal_strength) + "%");

    signal_box->pack_start(signal_label_, Gtk::PACK_SHRINK);

    // Set up the connect button with an icon
    if (network.connected) {
        connect_button_.set_image_from_icon_name("network-wireless-connected-symbolic", Gtk::ICON_SIZE_BUTTON);
        connect_button_.set_label("Disconnect");
    } else {
        connect_button_.set_image_from_icon_name("network-wireless-signal-excellent-symbolic", Gtk::ICON_SIZE_BUTTON);
        connect_button_.set_label("Connect");
    }

    // Set up the forget button with an icon
    forget_button_.set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);
    forget_button_.set_label("Forget");
    forget_button_.set_tooltip_text("Forget this network");

    // Add controls to the controls box
    controls_box_.pack_end(connect_button_, Gtk::PACK_SHRINK);
    controls_box_.pack_start(forget_button_, Gtk::PACK_SHRINK);

    // Add all components to the inner box
    inner_box->pack_start(network_info_box_, Gtk::PACK_SHRINK);
    inner_box->pack_start(*signal_box, Gtk::PACK_SHRINK);
    inner_box->pack_start(controls_box_, Gtk::PACK_SHRINK);

    // Connect signals
    connect_button_.signal_clicked().connect(sigc::mem_fun(*this, &WifiNetworkWidget::on_connect_clicked));
    forget_button_.signal_clicked().connect(sigc::mem_fun(*this, &WifiNetworkWidget::on_forget_clicked));

    show_all_children();
}

WifiNetworkWidget::~WifiNetworkWidget() = default;

std::string WifiNetworkWidget::convert_signal_to_quality(int signal_strength) {
    // Simply return the signal strength as a percentage
    // This is the raw value from nmcli which is already a percentage
    std::cout << "Signal strength for " << network_.ssid << ": " << signal_strength << "%" << std::endl;
    return std::to_string(signal_strength) + "%";
}

void WifiNetworkWidget::update_signal_icon(int signal_strength) {
    std::string icon_name;

    // Use signal_strength directly to determine the icon
    if (signal_strength < 20) {
        icon_name = "network-wireless-signal-none-symbolic";
    } else if (signal_strength < 40) {
        icon_name = "network-wireless-signal-weak-symbolic";
    } else if (signal_strength < 60) {
        icon_name = "network-wireless-signal-ok-symbolic";
    } else if (signal_strength < 80) {
        icon_name = "network-wireless-signal-good-symbolic";
    } else {
        icon_name = "network-wireless-signal-excellent-symbolic";
    }

    signal_icon_.set_from_icon_name(icon_name, Gtk::ICON_SIZE_LARGE_TOOLBAR);
}

void WifiNetworkWidget::update_security_icon(bool secured) {
    if (secured) {
        security_icon_.set_from_icon_name("channel-secure-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
        security_icon_.set_tooltip_text("Secured Network");
    } else {
        security_icon_.set_from_icon_name("channel-insecure-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
        security_icon_.set_tooltip_text("Open Network");
    }
}

void WifiNetworkWidget::update_connection_status(bool connected) {
    if (connected) {
        status_icon_.set_from_icon_name("network-wireless-connected-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
        status_icon_.set_tooltip_text("Connected");
    } else {
        status_icon_.clear();
    }
}

void WifiNetworkWidget::on_connect_clicked() {
    if (network_.connected) {
        manager_->disconnect();
    } else {
        std::string password;
        if (network_.secured) {
            // Create a styled dialog for password entry
            Gtk::Dialog dialog("Enter WiFi Password", true);
            dialog.set_default_size(300, -1);
            dialog.set_border_width(10);

            // Create a box with an icon for the dialog
            Gtk::Box* content_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
            Gtk::Image* lock_icon = Gtk::manage(new Gtk::Image());
            lock_icon->set_from_icon_name("channel-secure-symbolic", Gtk::ICON_SIZE_DIALOG);
            content_box->pack_start(*lock_icon, Gtk::PACK_SHRINK);

            // Create a vertical box for the network name and password entry
            Gtk::Box* entry_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));

            // Add network name label
            Gtk::Label* network_label = Gtk::manage(new Gtk::Label());
            network_label->set_markup("<b>" + network_.ssid + "</b>");
            network_label->set_halign(Gtk::ALIGN_START);
            entry_box->pack_start(*network_label, Gtk::PACK_SHRINK);

            // Add password entry field with label
            Gtk::Label* password_label = Gtk::manage(new Gtk::Label("Password:"));
            password_label->set_halign(Gtk::ALIGN_START);
            entry_box->pack_start(*password_label, Gtk::PACK_SHRINK);

            Gtk::Entry* entry = Gtk::manage(new Gtk::Entry());
            entry->set_visibility(false);
            entry->set_invisible_char('*');
            entry->set_activates_default(true);
            entry_box->pack_start(*entry, Gtk::PACK_SHRINK);

            content_box->pack_start(*entry_box, Gtk::PACK_EXPAND_WIDGET);

            dialog.get_content_area()->pack_start(*content_box, Gtk::PACK_EXPAND_WIDGET);
            dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
            dialog.add_button("Connect", Gtk::RESPONSE_OK);
            dialog.set_default_response(Gtk::RESPONSE_OK);

            dialog.show_all_children();

            int result = dialog.run();
            if (result == Gtk::RESPONSE_OK) {
                password = entry->get_text();
            } else {
                return;
            }
        }
        manager_->connect(network_.ssid, password);
    }
}

void WifiNetworkWidget::on_forget_clicked() {
    // Create a confirmation dialog
    Gtk::MessageDialog dialog(*dynamic_cast<Gtk::Window*>(get_toplevel()),
                             "Are you sure you want to forget this network?",
                             false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);

    // Add network name to the dialog
    Gtk::Box* content_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
    Gtk::Image* wifi_icon = Gtk::manage(new Gtk::Image());
    wifi_icon->set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_DIALOG);
    content_box->pack_start(*wifi_icon, Gtk::PACK_SHRINK);

    Gtk::Label* network_label = Gtk::manage(new Gtk::Label());
    network_label->set_markup("<b>" + network_.ssid + "</b>");
    network_label->set_halign(Gtk::ALIGN_START);
    content_box->pack_start(*network_label, Gtk::PACK_SHRINK);

    dialog.get_content_area()->pack_start(*content_box, Gtk::PACK_SHRINK);
    dialog.show_all_children();

    int result = dialog.run();
    if (result == Gtk::RESPONSE_YES) {
        manager_->forget_network(network_.ssid);
    }
}

} // namespace Wifi
