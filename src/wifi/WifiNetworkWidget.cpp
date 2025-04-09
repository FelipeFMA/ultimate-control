#include "WifiNetworkWidget.hpp"
#include <gtkmm/messagedialog.h>

namespace Wifi {

WifiNetworkWidget::WifiNetworkWidget(const Network& network, std::shared_ptr<WifiManager> manager)
: Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10),
  manager_(std::move(manager)),
  network_(network),
  ssid_label_(network.ssid),
  signal_label_("Signal: " + std::to_string(network.signal_strength) + "%"),
  status_label_(network.connected ? "Connected" : (network.secured ? "Secured" : "Open")),
  connect_button_(network.connected ? "Disconnect" : "Connect")
{
    set_margin_start(5);
    set_margin_end(5);
    set_margin_top(5);
    set_margin_bottom(5);

    pack_start(ssid_label_, Gtk::PACK_SHRINK);
    pack_start(signal_label_, Gtk::PACK_SHRINK);
    pack_start(status_label_, Gtk::PACK_SHRINK);
    pack_end(connect_button_, Gtk::PACK_SHRINK);

    connect_button_.signal_clicked().connect(sigc::mem_fun(*this, &WifiNetworkWidget::on_connect_clicked));

    show_all_children();
}

WifiNetworkWidget::~WifiNetworkWidget() = default;

void WifiNetworkWidget::on_connect_clicked() {
    if (network_.connected) {
        manager_->disconnect();
    } else {
        std::string password;
        if (network_.secured) {
            Gtk::Dialog dialog("Enter WiFi Password", true);
            Gtk::Entry entry;
            entry.set_visibility(false);
            entry.set_invisible_char('*');
            dialog.get_content_area()->pack_start(entry);
            entry.show();

            dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
            dialog.add_button("Connect", Gtk::RESPONSE_OK);

            int result = dialog.run();
            if (result == Gtk::RESPONSE_OK) {
                password = entry.get_text();
            } else {
                return;
            }
        }
        manager_->connect(network_.ssid, password);
    }
}

} // namespace Wifi
