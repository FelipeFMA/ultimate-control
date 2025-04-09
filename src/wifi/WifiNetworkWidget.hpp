#ifndef ULTIMATE_CONTROL_WIFI_NETWORK_WIDGET_HPP
#define ULTIMATE_CONTROL_WIFI_NETWORK_WIDGET_HPP

#include <gtkmm.h>
#include "WifiManager.hpp"
#include <memory>

namespace Wifi {

class WifiNetworkWidget : public Gtk::Box {
public:
    WifiNetworkWidget(const Network& network, std::shared_ptr<WifiManager> manager);
    virtual ~WifiNetworkWidget();

private:
    void on_connect_clicked();

    std::shared_ptr<WifiManager> manager_;
    Network network_;

    Gtk::Label ssid_label_;
    Gtk::Label signal_label_;
    Gtk::Label status_label_;
    Gtk::Button connect_button_;
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_NETWORK_WIDGET_HPP
