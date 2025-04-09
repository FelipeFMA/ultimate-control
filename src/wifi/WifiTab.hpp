#ifndef ULTIMATE_CONTROL_WIFI_TAB_HPP
#define ULTIMATE_CONTROL_WIFI_TAB_HPP

#include <gtkmm.h>
#include "WifiManager.hpp"
#include "WifiNetworkWidget.hpp"
#include <memory>
#include <vector>

namespace Wifi {

class WifiTab : public Gtk::ScrolledWindow {
public:
    WifiTab();
    virtual ~WifiTab();

private:
    void update_network_list(const std::vector<Network>& networks);
    void update_wifi_state(bool enabled);
    void on_wifi_switch_toggled();

    std::shared_ptr<WifiManager> manager_;
    Gtk::Box container_;
    Gtk::Button scan_button_;
    Gtk::Switch wifi_switch_;
    Gtk::Label wifi_status_label_;
    Gtk::Image wifi_status_icon_;
    std::vector<std::unique_ptr<WifiNetworkWidget>> widgets_;
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_TAB_HPP
