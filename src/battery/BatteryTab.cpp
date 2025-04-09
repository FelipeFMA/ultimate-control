#include "BatteryTab.hpp"
#include "../power/PowerManager.hpp"

namespace Battery {

BatteryTab::BatteryTab()
: manager_(std::make_shared<BatteryManager>())
{
    set_margin_start(10);
    set_margin_end(10);
    set_margin_top(10);
    set_margin_bottom(10);
    set_spacing(10);
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    status_label_.set_text("Battery status");
    progress_bar_.set_show_text(true);

    pack_start(status_label_, Gtk::PACK_SHRINK);
    pack_start(progress_bar_, Gtk::PACK_SHRINK);

    profile_combo_.set_hexpand(true);
    profile_combo_.set_margin_top(10);
    profile_combo_.set_margin_bottom(10);
    profile_combo_.set_margin_start(10);
    profile_combo_.set_margin_end(10);
    pack_start(profile_combo_, Gtk::PACK_SHRINK);

    profile_combo_.set_sensitive(true);
    profile_combo_.set_can_focus(true);

    Power::PowerManager power_manager;
    auto profiles = power_manager.list_power_profiles();
    profile_combo_.remove_all();
    for (const auto& profile : profiles) {
        profile_combo_.append(profile);
    }

    if (!profiles.empty()) {
        profile_combo_.set_sensitive(true);
        profile_combo_.set_active(0);
    } else {
        profile_combo_.set_sensitive(false);
    }

    auto current = power_manager.get_current_power_profile();
    bool found = false;
    for (const auto& profile : profiles) {
        if (profile == current) {
            profile_combo_.set_active_text(profile);
            found = true;
            break;
        }
    }
    if (!found && !profiles.empty()) {
        profile_combo_.set_active(0);
    }

    profile_combo_.signal_changed().connect([this]() {
        auto selected = profile_combo_.get_active_text();
        if (!selected.empty()) {
            Power::PowerManager pm;
            pm.set_power_profile(selected);
        }
    });

    manager_->set_update_callback([this](const BatteryStatus& status) {
        update_ui(status);
    });

    manager_->refresh();

    show_all_children();
}

BatteryTab::~BatteryTab() = default;

void BatteryTab::update_ui(const BatteryStatus& status) {
    if (!status.present) {
        status_label_.set_text("No battery detected");
        progress_bar_.set_fraction(0.0);
        progress_bar_.set_text("");
        return;
    }

    progress_bar_.set_fraction(status.percentage / 100.0);
    progress_bar_.set_text(std::to_string(status.percentage) + "%");

    if (status.charging) {
        status_label_.set_text("Charging");
    } else {
        status_label_.set_text("Discharging");
    }
}

} // namespace Battery
