#include <gtkmm.h>
#include <iostream>
#include "volume/VolumeTab.hpp"
#include "wifi/WifiTab.hpp"
#include "display/DisplayTab.hpp"
#include "power/PowerTab.hpp"
#include "core/Settings.hpp"

class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        set_title("Ultimate Control");
        set_default_size(800, 600);

        vbox_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(vbox_);

        notebook_.set_scrollable(true);
        vbox_.pack_start(notebook_, Gtk::PACK_EXPAND_WIDGET);

        auto volume_tab = Gtk::make_managed<Volume::VolumeTab>();
        {
            auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
            auto icon = Gtk::make_managed<Gtk::Image>();
            icon->set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            auto label = Gtk::make_managed<Gtk::Label>("Volume");
            box->pack_start(*icon, Gtk::PACK_SHRINK);
            box->pack_start(*label, Gtk::PACK_SHRINK);
            box->show_all();
            notebook_.append_page(*volume_tab, *box);
        }

        auto wifi_tab = Gtk::make_managed<Wifi::WifiTab>();
        {
            auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
            auto icon = Gtk::make_managed<Gtk::Image>();
            icon->set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            auto label = Gtk::make_managed<Gtk::Label>("WiFi");
            box->pack_start(*icon, Gtk::PACK_SHRINK);
            box->pack_start(*label, Gtk::PACK_SHRINK);
            box->show_all();
            notebook_.append_page(*wifi_tab, *box);
        }

        auto display_tab = Gtk::make_managed<Display::DisplayTab>();
        {
            auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
            auto icon = Gtk::make_managed<Gtk::Image>();
            icon->set_from_icon_name("video-display-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            auto label = Gtk::make_managed<Gtk::Label>("Display");
            box->pack_start(*icon, Gtk::PACK_SHRINK);
            box->pack_start(*label, Gtk::PACK_SHRINK);
            box->show_all();
            notebook_.append_page(*display_tab, *box);
        }

        auto power_tab = Gtk::make_managed<Power::PowerTab>();
        {
            auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
            auto icon = Gtk::make_managed<Gtk::Image>();
            icon->set_from_icon_name("system-shutdown-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            auto label = Gtk::make_managed<Gtk::Label>("Power");
            box->pack_start(*icon, Gtk::PACK_SHRINK);
            box->pack_start(*label, Gtk::PACK_SHRINK);
            box->show_all();
            notebook_.append_page(*power_tab, *box);
        }



        show_all_children();
    }

private:
    Gtk::Box vbox_;
    Gtk::Notebook notebook_;

};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "com.example.ultimatecontrol");
    MainWindow window;
    return app->run(window);
}
