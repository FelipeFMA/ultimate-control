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

    std::shared_ptr<WifiManager> manager_;
    Gtk::Box container_;
    std::vector<std::unique_ptr<WifiNetworkWidget>> widgets_;
};

} // namespace Wifi

#endif // ULTIMATE_CONTROL_WIFI_TAB_HPP
