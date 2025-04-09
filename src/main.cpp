#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <map>
#include <cstdlib>
#include <string>
#include <glibmm/optioncontext.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optionentry.h>
#include "volume/VolumeTab.hpp"
#include "wifi/WifiTab.hpp"
#include "display/DisplayTab.hpp"
#include "power/PowerTab.hpp"
#include "settings/SettingsTab.hpp"
#include "settings/TabSettings.hpp"
#include "core/Settings.hpp"

class MainWindow : public Gtk::Window {
public:
    MainWindow(const std::string& initial_tab = "") {
        initial_tab_ = initial_tab;
        prevent_auto_loading_ = !initial_tab_.empty();
        set_title("Ultimate Control");
        set_default_size(800, 600);

        vbox_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(vbox_);

        notebook_.set_scrollable(true);
        vbox_.pack_start(notebook_, Gtk::PACK_EXPAND_WIDGET);

        // Initialize tab settings
        tab_settings_ = std::make_shared<Settings::TabSettings>();

        // Connect to tab switch signal for lazy loading
        notebook_.signal_switch_page().connect(sigc::mem_fun(*this, &MainWindow::on_tab_switch));

        // Create tab placeholders
        create_tabs();

        // Create a placeholder for the settings tab
        auto settings_placeholder = Gtk::make_managed<Gtk::Box>();
        settings_placeholder->set_size_request(100, 100);

        // Add settings tab placeholder
        add_tab("settings", settings_placeholder, "preferences-system-symbolic", "Settings");

        // Connect to the delete event signal
        signal_delete_event().connect([](GdkEventAny* event) -> bool {
            // Force exit the application
            std::quick_exit(0);
            return true; // Prevent the default handler from running
        });

        // Show all children
        show_all_children();

        // Switch to the initial tab if specified
        if (!initial_tab_.empty()) {
            switch_to_tab(initial_tab_);
        }
    }

    // Switch to a specific tab by ID
    void switch_to_tab(const std::string& tab_id) {
        // Find the tab
        auto it = tab_widgets_.find(tab_id);
        if (it != tab_widgets_.end()) {
            // Load the tab content if not already loaded
            if (!it->second.loaded) {
                load_tab_content(tab_id, it->second.page_num);
            }

            // Set the current page
            notebook_.set_current_page(it->second.page_num);
        }
    }

private:
    void create_tabs() {
        // Clear existing tabs
        while (notebook_.get_n_pages() > 0) {
            notebook_.remove_page(-1);
        }
        tab_widgets_.clear();

        // Get tab order from settings
        auto tab_order = tab_settings_->get_tab_order();

        // If we have an initial tab specified, make sure it's enabled
        if (!initial_tab_.empty()) {
            tab_settings_->set_tab_enabled(initial_tab_, true);
        }

        // Create tabs in the specified order
        for (const auto& tab_id : tab_order) {
            if (tab_id == "settings") {
                continue; // Settings tab is added separately
            }

            if (!tab_settings_->is_tab_enabled(tab_id)) {
                continue; // Skip disabled tabs
            }

            // Create a placeholder for lazy loading
            auto placeholder = Gtk::make_managed<Gtk::Box>();
            placeholder->set_size_request(100, 100);

            // Add the tab with appropriate icon and label
            if (tab_id == "volume") {
                add_tab(tab_id, placeholder, "audio-volume-high-symbolic", "Volume");
            } else if (tab_id == "wifi") {
                add_tab(tab_id, placeholder, "network-wireless-symbolic", "WiFi");
            } else if (tab_id == "display") {
                add_tab(tab_id, placeholder, "video-display-symbolic", "Display");
            } else if (tab_id == "power") {
                add_tab(tab_id, placeholder, "system-shutdown-symbolic", "Power");
            }
        }
    }

    Gtk::Box* create_tab_label(const std::string& icon_name, const std::string& label_text) {
        // Create tab label with icon
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
        auto icon = Gtk::make_managed<Gtk::Image>();
        icon->set_from_icon_name(icon_name, Gtk::ICON_SIZE_SMALL_TOOLBAR);
        auto label = Gtk::make_managed<Gtk::Label>(label_text);
        box->pack_start(*icon, Gtk::PACK_SHRINK);
        box->pack_start(*label, Gtk::PACK_SHRINK);
        box->show_all();
        return box;
    }

    void add_tab(const std::string& id, Gtk::Widget* widget, const std::string& icon_name, const std::string& label_text) {
        // Create tab label with icon
        auto box = create_tab_label(icon_name, label_text);

        // Add the tab
        int page_num = notebook_.append_page(*widget, *box);

        // Store the widget and page number
        tab_widgets_[id] = TabInfo{widget, page_num, false};
    }

    void on_tab_switch(Gtk::Widget* page, guint page_num) {
        // Prevent recursive calls during tab loading
        static bool loading = false;
        if (loading) {
            return;
        }

        // If we're preventing auto-loading (e.g., when starting with a specific tab),
        // don't load other tabs automatically
        if (prevent_auto_loading_) {
            // Only allow loading the initial tab
            std::string current_tab;
            for (const auto& [id, info] : tab_widgets_) {
                if (info.page_num == static_cast<int>(page_num)) {
                    current_tab = id;
                    break;
                }
            }

            if (current_tab != initial_tab_) {
                return;
            }
        }

        // Find which tab was selected
        std::string tab_id_to_load;
        for (const auto& [id, info] : tab_widgets_) {
            if (info.page_num == static_cast<int>(page_num) && !info.loaded) {
                tab_id_to_load = id;
                break;
            }
        }

        // If we found a tab to load, load it
        if (!tab_id_to_load.empty()) {
            // Set loading flag to prevent recursive calls
            loading = true;

            // Use a single-shot timer to delay loading slightly
            // This helps avoid GTK+ issues during tab switching
            Glib::signal_timeout().connect_once([this, tab_id_to_load, page_num]() {
                load_tab_content(tab_id_to_load, page_num);
            }, 50);

            // Reset loading flag after a short delay
            Glib::signal_timeout().connect_once([]() {
                // Use a new lambda to avoid capturing the static variable
                loading = false;
            }, 100);
        }
    }

    void load_tab_content(const std::string& id, int page_num) {
        // Check if already loaded
        if (tab_widgets_[id].loaded) {
            return;
        }

        std::cout << "Lazy loading " << id << " tab" << std::endl;

        // Mark as loaded before creating to prevent recursive loading
        tab_widgets_[id].loaded = true;

        // Create the actual tab content
        Gtk::Widget* content = nullptr;
        std::string icon_name;
        std::string label_text;

        if (id == "volume") {
            content = Gtk::make_managed<Volume::VolumeTab>();
            icon_name = "audio-volume-high-symbolic";
            label_text = "Volume";
        } else if (id == "wifi") {
            content = Gtk::make_managed<Wifi::WifiTab>();
            icon_name = "network-wireless-symbolic";
            label_text = "WiFi";
        } else if (id == "display") {
            content = Gtk::make_managed<Display::DisplayTab>();
            icon_name = "video-display-symbolic";
            label_text = "Display";
        } else if (id == "power") {
            content = Gtk::make_managed<Power::PowerTab>();
            icon_name = "system-shutdown-symbolic";
            label_text = "Power";
        } else if (id == "settings") {
            auto settings_tab = Gtk::make_managed<Settings::SettingsTab>();
            settings_tab->set_settings_changed_callback([this]() {
                // Reload tabs when settings change
                reload_tabs();
            });
            content = settings_tab;
            icon_name = "preferences-system-symbolic";
            label_text = "Settings";
        } else {
            // Unknown tab, reset loaded flag
            tab_widgets_[id].loaded = false;
            return;
        }

        try {
            // Create a new tab label with icon
            auto box = create_tab_label(icon_name, label_text);

            // Replace the placeholder with the actual content
            notebook_.remove_page(page_num);

            // Insert the new content
            int new_page_num = notebook_.insert_page(*content, *box, page_num);

            // Show the new content
            content->show_all();

            // Update the tab info
            tab_widgets_[id].widget = content;
            tab_widgets_[id].page_num = new_page_num;

            // Set the current page
            notebook_.set_current_page(new_page_num);
        } catch (const std::exception& e) {
            std::cerr << "Error loading tab " << id << ": " << e.what() << std::endl;
            tab_widgets_[id].loaded = false;
        } catch (...) {
            std::cerr << "Unknown error loading tab " << id << std::endl;
            tab_widgets_[id].loaded = false;
        }
    }

    void reload_tabs() {
        // Save the current page or use the initial tab
        int current_page = notebook_.get_current_page();
        std::string current_tab;

        // Find which tab is currently selected
        for (const auto& [id, info] : tab_widgets_) {
            if (info.page_num == current_page) {
                current_tab = id;
                break;
            }
        }

        // If we have an initial tab and this is the first load, use that
        if (!initial_tab_.empty() && current_tab.empty()) {
            current_tab = initial_tab_;
        }

        // Recreate tabs
        create_tabs();

        // Restore the current tab if possible
        if (!current_tab.empty()) {
            switch_to_tab(current_tab);
        }
        // Otherwise restore by page number
        else if (current_page >= 0 && current_page < notebook_.get_n_pages()) {
            notebook_.set_current_page(current_page);
        }
    }

private:
    Gtk::Box vbox_;
    Gtk::Notebook notebook_;
    std::shared_ptr<Settings::TabSettings> tab_settings_;
    std::string initial_tab_;
    bool prevent_auto_loading_ = false;

    // Structure to track tab widgets and loading state
    struct TabInfo {
        Gtk::Widget* widget;
        int page_num;
        bool loaded;
    };
    std::map<std::string, TabInfo> tab_widgets_;

};

int main(int argc, char* argv[]) {
    // Define our command-line options
    Glib::OptionContext context;
    Glib::OptionGroup group("options", "Application Options", "Application options");

    // Variables to store option values
    bool volume_opt = false;
    bool wifi_opt = false;
    bool display_opt = false;
    bool power_opt = false;
    bool settings_opt = false;

    // Define the command-line entries
    Glib::OptionEntry volume_entry;
    volume_entry.set_long_name("volume");
    volume_entry.set_short_name('v');
    volume_entry.set_description("Start with the Volume tab selected");
    group.add_entry(volume_entry, volume_opt);

    Glib::OptionEntry wifi_entry;
    wifi_entry.set_long_name("wifi");
    wifi_entry.set_short_name('w');
    wifi_entry.set_description("Start with the WiFi tab selected");
    group.add_entry(wifi_entry, wifi_opt);

    Glib::OptionEntry display_entry;
    display_entry.set_long_name("display");
    display_entry.set_short_name('d');
    display_entry.set_description("Start with the Display tab selected");
    group.add_entry(display_entry, display_opt);

    Glib::OptionEntry power_entry;
    power_entry.set_long_name("power");
    power_entry.set_short_name('p');
    power_entry.set_description("Start with the Power tab selected");
    group.add_entry(power_entry, power_opt);

    Glib::OptionEntry settings_entry;
    settings_entry.set_long_name("settings");
    settings_entry.set_short_name('s');
    settings_entry.set_description("Start with the Settings tab selected");
    group.add_entry(settings_entry, settings_opt);

    // Add the option group to the context
    context.set_main_group(group);

    try {
        context.parse(argc, argv);
    } catch (const Glib::Error& error) {
        std::cerr << "Error parsing command line: " << error.what() << std::endl;
        return 1;
    }

    // Determine which tab to show initially
    std::string initial_tab;
    if (volume_opt) {
        initial_tab = "volume";
    } else if (wifi_opt) {
        initial_tab = "wifi";
    } else if (display_opt) {
        initial_tab = "display";
    } else if (power_opt) {
        initial_tab = "power";
    } else if (settings_opt) {
        initial_tab = "settings";
    }

    // Initialize GTK application
    auto app = Gtk::Application::create(argc, argv, "com.example.ultimatecontrol");

    // Create the main window with the initial tab
    MainWindow window(initial_tab);

    // Run the application
    return app->run(window);
}
