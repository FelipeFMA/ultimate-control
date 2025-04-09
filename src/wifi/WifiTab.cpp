#include "WifiTab.hpp"

namespace Wifi {

WifiTab::WifiTab()
: manager_(std::make_shared<WifiManager>()),
  container_(Gtk::ORIENTATION_VERTICAL, 5)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(container_);

    manager_->set_update_callback([this](const std::vector<Network>& networks) {
        update_network_list(networks);
    });

    manager_->scan_networks();

    show_all_children();
}

WifiTab::~WifiTab() = default;

void WifiTab::update_network_list(const std::vector<Network>& networks) {
    for (auto& widget : widgets_) {
        container_.remove(*widget);
    }
    widgets_.clear();

    for (const auto& net : networks) {
        auto widget = std::make_unique<WifiNetworkWidget>(net, manager_);
        container_.pack_start(*widget, Gtk::PACK_SHRINK);
        widgets_.push_back(std::move(widget));
    }

    show_all_children();
}

} // namespace Wifi
