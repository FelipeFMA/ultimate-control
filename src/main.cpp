#include <gtkmm.h>
#include <iostream>
#include "volume/VolumeTab.hpp"
#include "wifi/WifiTab.hpp"
#include "display/DisplayTab.hpp"
#include "power/PowerTab.hpp"
#include "battery/BatteryTab.hpp"
#include "core/Settings.hpp"

class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        set_title("Ultimate Control");
        set_default_size(800, 600);

        vbox_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(vbox_);

        header_.set_show_close_button(false);
        header_.set_title("Ultimate Control");
        set_titlebar(header_);

        settings_button_.set_label("Settings");
        settings_button_.signal_clicked().connect([this]() {
            Core::SettingsWindow settings_dialog(*this);
            settings_dialog.run();
        });
        header_.pack_end(settings_button_);

        notebook_.set_scrollable(true);
        vbox_.pack_start(notebook_, Gtk::PACK_EXPAND_WIDGET);

        auto volume_tab = Gtk::make_managed<Volume::VolumeTab>();
        notebook_.append_page(*volume_tab, "Volume");

        auto wifi_tab = Gtk::make_managed<Wifi::WifiTab>();
        notebook_.append_page(*wifi_tab, "WiFi");

        auto display_tab = Gtk::make_managed<Display::DisplayTab>();
        notebook_.append_page(*display_tab, "Display");

        auto power_tab = Gtk::make_managed<Power::PowerTab>();
        notebook_.append_page(*power_tab, "Power");

        auto battery_tab = Gtk::make_managed<Battery::BatteryTab>();
        notebook_.append_page(*battery_tab, "Battery");

        show_all_children();
    }

private:
    Gtk::Box vbox_;
    Gtk::HeaderBar header_;
    Gtk::Button settings_button_;
    Gtk::Notebook notebook_;

};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "com.example.ultimatecontrol");
    MainWindow window;
    return app->run(window);
}
