#ifndef ULTIMATE_CONTROL_BATTERY_TAB_HPP
#define ULTIMATE_CONTROL_BATTERY_TAB_HPP

#include <gtkmm.h>
#include "BatteryManager.hpp"
#include <memory>

namespace Battery {

class BatteryTab : public Gtk::Box {
public:
    BatteryTab();
    virtual ~BatteryTab();

private:
    void update_ui(const BatteryStatus& status);

    std::shared_ptr<BatteryManager> manager_;

    Gtk::Label status_label_;
    Gtk::ProgressBar progress_bar_;

    Gtk::ComboBoxText profile_combo_;
};

} // namespace Battery

#endif // ULTIMATE_CONTROL_BATTERY_TAB_HPP
