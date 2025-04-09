#include "WifiTab.hpp"

namespace Wifi {

WifiTab::WifiTab()
: manager_(std::make_shared<WifiManager>()),
  container_(Gtk::ORIENTATION_VERTICAL, 10)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    // Create a main box to hold everything
    Gtk::Box* main_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
    main_box->set_margin_start(10);
    main_box->set_margin_end(10);
    main_box->set_margin_top(10);
    main_box->set_margin_bottom(10);
    add(*main_box);

    // Create a header box with title and controls
    Gtk::Box* header_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));

    // Add WiFi icon
    wifi_status_icon_.set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_DIALOG);
    header_box->pack_start(wifi_status_icon_, Gtk::PACK_SHRINK);

    // Add title
    Gtk::Label* title = Gtk::manage(new Gtk::Label());
    title->set_markup("<span size='large' weight='bold'>Available Networks</span>");
    title->set_halign(Gtk::ALIGN_START);
    title->set_valign(Gtk::ALIGN_CENTER);
    header_box->pack_start(*title, Gtk::PACK_EXPAND_WIDGET);

    // Create a box for controls on the right side
    Gtk::Box* controls_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5));

    // Create WiFi toggle box
    Gtk::Box* toggle_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));

    // Set up the WiFi toggle switch with label
    Gtk::Label* toggle_label = Gtk::manage(new Gtk::Label("WiFi:"));
    wifi_status_label_.set_text("Enabled");
    wifi_switch_.set_active(manager_->is_wifi_enabled());
    wifi_switch_.set_tooltip_text("Enable/Disable WiFi");

    toggle_box->pack_start(*toggle_label, Gtk::PACK_SHRINK);
    toggle_box->pack_start(wifi_switch_, Gtk::PACK_SHRINK);
    toggle_box->pack_start(wifi_status_label_, Gtk::PACK_SHRINK);

    // Add scan button with icon
    scan_button_.set_image_from_icon_name("view-refresh-symbolic", Gtk::ICON_SIZE_BUTTON);
    scan_button_.set_label("Scan");
    scan_button_.set_always_show_image(true);
    scan_button_.set_sensitive(manager_->is_wifi_enabled());

    // Add toggle and scan button to controls box
    controls_box->pack_start(*toggle_box, Gtk::PACK_SHRINK);
    controls_box->pack_start(scan_button_, Gtk::PACK_SHRINK);

    // Add controls to header
    header_box->pack_end(*controls_box, Gtk::PACK_SHRINK);

    // Add a separator
    Gtk::Separator* separator = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));

    // Add header and separator to main box
    main_box->pack_start(*header_box, Gtk::PACK_SHRINK);
    main_box->pack_start(*separator, Gtk::PACK_SHRINK);

    // Add a scrolled window for the networks
    Gtk::ScrolledWindow* networks_scroll = Gtk::manage(new Gtk::ScrolledWindow());
    networks_scroll->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    networks_scroll->add(container_);
    main_box->pack_start(*networks_scroll, Gtk::PACK_EXPAND_WIDGET);

    // Connect scan button signal
    scan_button_.signal_clicked().connect([this]() {
        scan_button_.set_sensitive(false);
        scan_button_.set_label("Scanning...");
        manager_->scan_networks();
        // Re-enable the button after a short delay
        Glib::signal_timeout().connect_once([this]() {
            scan_button_.set_sensitive(true);
            scan_button_.set_label("Scan");
        }, 2000);
    });

    // Connect WiFi switch signal
    wifi_switch_.property_active().signal_changed().connect(sigc::mem_fun(*this, &WifiTab::on_wifi_switch_toggled));

    // Set up the callback for network updates
    manager_->set_update_callback([this](const std::vector<Network>& networks) {
        update_network_list(networks);
    });

    // Set up the callback for WiFi state updates
    manager_->set_state_callback([this](bool enabled) {
        update_wifi_state(enabled);
    });

    // Update initial WiFi state
    update_wifi_state(manager_->is_wifi_enabled());

    // Initial scan
    manager_->scan_networks();

    show_all_children();
}

WifiTab::~WifiTab() = default;

void WifiTab::update_wifi_state(bool enabled) {
    // Update the UI based on WiFi state
    wifi_switch_.set_active(enabled);
    wifi_status_label_.set_text(enabled ? "Enabled" : "Disabled");
    scan_button_.set_sensitive(enabled);

    // Update the WiFi icon
    if (enabled) {
        wifi_status_icon_.set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_DIALOG);
    } else {
        wifi_status_icon_.set_from_icon_name("network-wireless-disabled-symbolic", Gtk::ICON_SIZE_DIALOG);
    }
}

void WifiTab::on_wifi_switch_toggled() {
    bool enabled = wifi_switch_.get_active();

    // Prevent recursive calls from the state callback
    wifi_switch_.set_sensitive(false);

    if (enabled) {
        manager_->enable_wifi();
    } else {
        manager_->disable_wifi();
    }

    // Re-enable the switch after a short delay
    Glib::signal_timeout().connect_once([this]() {
        wifi_switch_.set_sensitive(true);
    }, 1000);
}

void WifiTab::update_network_list(const std::vector<Network>& networks) {
    for (auto& widget : widgets_) {
        container_.remove(*widget);
    }
    widgets_.clear();

    if (networks.empty() && manager_->is_wifi_enabled()) {
        // Add a message when no networks are found but WiFi is enabled
        Gtk::Label* no_networks = Gtk::manage(new Gtk::Label("No wireless networks found"));
        no_networks->set_margin_top(20);
        no_networks->set_margin_bottom(20);
        container_.pack_start(*no_networks, Gtk::PACK_SHRINK);
    } else {
        for (const auto& net : networks) {
            auto widget = std::make_unique<WifiNetworkWidget>(net, manager_);
            container_.pack_start(*widget, Gtk::PACK_SHRINK);
            widgets_.push_back(std::move(widget));
        }
    }

    show_all_children();
}

} // namespace Wifi
