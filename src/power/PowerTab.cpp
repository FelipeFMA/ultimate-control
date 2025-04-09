#include "PowerTab.hpp"

namespace Power {

PowerTab::PowerTab()
: manager_(std::make_shared<PowerManager>()),
  shutdown_button_("Shutdown"),
  reboot_button_("Reboot"),
  suspend_button_("Suspend"),
  hibernate_button_("Hibernate"),
  lock_button_("Lock")
{
    set_margin_start(10);
    set_margin_end(10);
    set_margin_top(10);
    set_margin_bottom(10);
    set_spacing(10);
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    pack_start(shutdown_button_, Gtk::PACK_SHRINK);
    pack_start(reboot_button_, Gtk::PACK_SHRINK);
    pack_start(suspend_button_, Gtk::PACK_SHRINK);
    pack_start(hibernate_button_, Gtk::PACK_SHRINK);
    pack_start(lock_button_, Gtk::PACK_SHRINK);

    shutdown_button_.signal_clicked().connect([this]() {
        manager_->shutdown();
    });
    reboot_button_.signal_clicked().connect([this]() {
        manager_->reboot();
    });
    suspend_button_.signal_clicked().connect([this]() {
        manager_->suspend();
    });
    hibernate_button_.signal_clicked().connect([this]() {
        manager_->hibernate();
    });
    lock_button_.signal_clicked().connect([]() {
        std::system("loginctl lock-session");
    });

    show_all_children();
}

PowerTab::~PowerTab() = default;

} // namespace Power
