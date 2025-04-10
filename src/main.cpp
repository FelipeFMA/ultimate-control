#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <map>
#include <cstdlib>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <glibmm/optioncontext.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optionentry.h>
#include <glibmm/dispatcher.h>
#include "volume/VolumeTab.hpp"
#include "wifi/WifiTab.hpp"
#include "display/DisplayTab.hpp"
#include "power/PowerTab.hpp"
#include "settings/SettingsTab.hpp"
#include "settings/TabSettings.hpp"
#include "core/Settings.hpp"

class MainWindow : public Gtk::Window {
public:
    MainWindow(const std::string& initial_tab = "", bool minimal_mode = false) {
        initial_tab_ = initial_tab;
        minimal_mode_ = minimal_mode;
        prevent_auto_loading_ = !initial_tab_.empty();
        set_title("Ultimate Control");
        set_default_size(800, 600);

        vbox_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(vbox_);

        notebook_.set_scrollable(true);
        notebook_.set_show_tabs(!minimal_mode_);
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
            // Load the tab content if not already loaded or loading
            if (!it->second.loaded && !it->second.loading) {
                // Show loading indicator and start async loading
                show_loading_indicator(tab_id, it->second.page_num);
                load_tab_content_async(tab_id, it->second.page_num);
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
        tab_widgets_[id] = TabInfo{widget, page_num, false, false};

        // Create a dispatcher for this tab
        tab_loaded_dispatchers_[id].connect([this, id]() {
            on_tab_loaded(id);
        });
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
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            for (const auto& [id, info] : tab_widgets_) {
                if (info.page_num == static_cast<int>(page_num) && !info.loaded && !info.loading) {
                    tab_id_to_load = id;
                    break;
                }
            }
        }

        // If we found a tab to load, load it
        if (!tab_id_to_load.empty()) {
            // Set loading flag to prevent recursive calls
            loading = true;

            // Use a single-shot timer to delay loading slightly
            // This helps avoid GTK+ issues during tab switching
            Glib::signal_timeout().connect_once([this, tab_id_to_load, page_num]() {
                // Show loading indicator and start async loading
                show_loading_indicator(tab_id_to_load, page_num);
                load_tab_content_async(tab_id_to_load, page_num);
            }, 50);

            // Reset loading flag after a short delay
            Glib::signal_timeout().connect_once([]() {
                // Use a new lambda to avoid capturing the static variable
                loading = false;
            }, 100);
        }
    }

    // Create a loading indicator widget
    Gtk::Widget* create_loading_indicator() {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
        box->set_halign(Gtk::ALIGN_CENTER);
        box->set_valign(Gtk::ALIGN_CENTER);

        // Add a spinner
        auto spinner = Gtk::make_managed<Gtk::Spinner>();
        spinner->set_size_request(32, 32);
        spinner->start();
        box->pack_start(*spinner, Gtk::PACK_SHRINK);

        // Add a loading label
        auto label = Gtk::make_managed<Gtk::Label>("Loading...");
        box->pack_start(*label, Gtk::PACK_SHRINK);

        box->show_all();
        return box;
    }

    // Show loading indicator for a tab
    void show_loading_indicator(const std::string& id, int page_num) {
        std::lock_guard<std::mutex> lock(tab_mutex_);

        // Check if already loaded or loading
        if (tab_widgets_[id].loaded || tab_widgets_[id].loading) {
            return;
        }

        // Make sure the tab exists in the notebook
        if (tab_widgets_.find(id) == tab_widgets_.end()) {
            return;
        }

        // Make sure the page number is valid
        if (page_num < 0 || page_num >= notebook_.get_n_pages()) {
            return;
        }

        // Mark as loading
        tab_widgets_[id].loading = true;

        // Create a loading indicator
        auto loading_indicator = create_loading_indicator();

        // Get the icon name and label text for the tab
        std::string icon_name;
        std::string label_text;

        if (id == "volume") {
            icon_name = "audio-volume-high-symbolic";
            label_text = "Volume";
        } else if (id == "wifi") {
            icon_name = "network-wireless-symbolic";
            label_text = "WiFi";
        } else if (id == "display") {
            icon_name = "video-display-symbolic";
            label_text = "Display";
        } else if (id == "power") {
            icon_name = "system-shutdown-symbolic";
            label_text = "Power";
        } else if (id == "settings") {
            icon_name = "preferences-system-symbolic";
            label_text = "Settings";
        } else {
            // Unknown tab
            tab_widgets_[id].loading = false;
            return;
        }

        try {
            // Create a new tab label with icon
            auto box = create_tab_label(icon_name, label_text);

            // Replace the placeholder with the loading indicator
            notebook_.remove_page(page_num);

            // Insert the loading indicator
            int new_page_num = notebook_.insert_page(*loading_indicator, *box, page_num);

            // Show the loading indicator
            loading_indicator->show_all();

            // Update the tab info
            tab_widgets_[id].widget = loading_indicator;
            tab_widgets_[id].page_num = new_page_num;

            // Set the current page
            notebook_.set_current_page(new_page_num);
        } catch (const std::exception& e) {
            std::cerr << "Error showing loading indicator for tab " << id << ": " << e.what() << std::endl;
            tab_widgets_[id].loading = false;
        } catch (...) {
            std::cerr << "Unknown error showing loading indicator for tab " << id << std::endl;
            tab_widgets_[id].loading = false;
        }
    }

    // Load tab content asynchronously
    void load_tab_content_async(const std::string& id, int page_num) {
        // Check if already loaded or loading
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            if (tab_widgets_[id].loaded || (tab_widgets_.find(id) == tab_widgets_.end())) {
                return;
            }
        }

        // Use a timeout to simulate asynchronous loading
        // This keeps the UI responsive while the tab is "loading"
        Glib::signal_timeout().connect_once([this, id, page_num]() {
            // This runs in the main thread after a short delay
            // Create the actual tab content
            create_tab_content(id, page_num);
        }, 100); // Short delay to allow UI to update
    }

    // Create tab content in the main thread
    void create_tab_content(const std::string& id, int page_num) {
        // Check if already loaded
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            if (tab_widgets_[id].loaded || (tab_widgets_.find(id) == tab_widgets_.end())) {
                return;
            }
        }

        try {
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
                    // This callback is no longer needed as the app restarts
                    // but we keep it for compatibility
                    std::cout << "Settings changed, restart required" << std::endl;
                });
                content = settings_tab;
                icon_name = "preferences-system-symbolic";
                label_text = "Settings";
            } else {
                // Unknown tab
                std::lock_guard<std::mutex> lock(tab_mutex_);
                tab_widgets_[id].loading = false;
                tab_load_errors_[id] = "Unknown tab type";
                return;
            }

            // Get the current page number for this tab
            int current_page_num;
            {
                std::lock_guard<std::mutex> lock(tab_mutex_);
                current_page_num = tab_widgets_[id].page_num;
            }

            // Create a new tab label with icon
            auto box = create_tab_label(icon_name, label_text);

            // Replace the loading indicator with the actual content
            notebook_.remove_page(current_page_num);

            // Insert the new content
            int new_page_num = notebook_.insert_page(*content, *box, current_page_num);

            // Show the new content
            content->show_all();

            // Update the tab info
            {
                std::lock_guard<std::mutex> lock(tab_mutex_);
                tab_widgets_[id].widget = content;
                tab_widgets_[id].page_num = new_page_num;
                tab_widgets_[id].loaded = true;
                tab_widgets_[id].loading = false;
            }

            // Set the current page
            notebook_.set_current_page(new_page_num);

            // Notify that the tab has been loaded
            tab_loaded_dispatchers_[id].emit();

        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            tab_widgets_[id].loading = false;
            tab_load_errors_[id] = e.what();
            std::cerr << "Error creating tab " << id << ": " << e.what() << std::endl;
        } catch (...) {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            tab_widgets_[id].loading = false;
            tab_load_errors_[id] = "Unknown error";
            std::cerr << "Unknown error creating tab " << id << std::endl;
        }
    }

    // Called when a tab has finished loading
    void on_tab_loaded(const std::string& id) {
        // This method is called via the dispatcher when a tab is fully loaded
        // We can use it for any post-loading operations
        std::cout << "Tab " << id << " loaded successfully" << std::endl;
    }





private:
    Gtk::Box vbox_;
    Gtk::Notebook notebook_;
    std::shared_ptr<Settings::TabSettings> tab_settings_;
    std::string initial_tab_;
    bool prevent_auto_loading_ = false;
    bool minimal_mode_ = false;

    // Structure to track tab widgets and loading state
    struct TabInfo {
        Gtk::Widget* widget;
        int page_num;
        bool loaded;
        bool loading;  // Flag to indicate if the tab is currently being loaded
    };
    std::map<std::string, TabInfo> tab_widgets_;

    // Asynchronous loading components
    std::mutex tab_mutex_;
    std::map<std::string, Glib::Dispatcher> tab_loaded_dispatchers_;
    std::map<std::string, std::string> tab_load_errors_;

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
    bool minimal_opt = false;

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

    Glib::OptionEntry minimal_entry;
    minimal_entry.set_long_name("minimal");
    minimal_entry.set_short_name('m');
    minimal_entry.set_description("Start in minimal mode with notebook tabs hidden");
    group.add_entry(minimal_entry, minimal_opt);

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

    // Create the main window with the initial tab and minimal mode setting
    MainWindow window(initial_tab, minimal_opt);

    // Run the application
    return app->run(window);
}
