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
    void on_forget_clicked();
    void update_signal_icon(int signal_strength);
    void update_security_icon(bool secured);
    void update_connection_status(bool connected);

    std::shared_ptr<WifiManager> manager_;
    Network network_;

    // Main layout containers
    Gtk::Box network_info_box_;
    Gtk::Box controls_box_;

    // Network information widgets
    Gtk::Image signal_icon_;
    Gtk::Image security_icon_;
    Gtk::Image status_icon_;
    Gtk::Label ssid_label_;
    Gtk::Label signal_label_;
    Gtk::Label status_label_;

    // Control widgets
    Gtk::Button connect_button_;
    Gtk::Button forget_button_;
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_NETWORK_WIDGET_HPP
