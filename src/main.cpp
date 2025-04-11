/**
 * @file main.cpp
 * @brief Main application entry point for Ultimate Control
 *
 * This file contains the MainWindow class which manages the tab-based UI
 * and implements lazy loading of tab content for better performance.
 */

#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <map>
#include <cstdlib>
#include <string>
#include <mutex>
#include <glibmm/optioncontext.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optionentry.h>
#include <glibmm/dispatcher.h>
#include "volume/VolumeTab.hpp"
#include "wifi/WifiTab.hpp"
#include "display/DisplayTab.hpp"
#include "power/PowerTab.hpp"
#include "settings/SettingsWindow.hpp"
#include "settings/TabSettings.hpp"
#include "core/Settings.hpp"

/**
 * @class MainWindow
 * @brief Main application window that manages tabs and lazy loading
 *
 * The MainWindow class handles the creation and management of tabs,
 * implements lazy loading for better performance, and provides
 * functionality for switching between tabs.
 */
class MainWindow : public Gtk::Window
{
public:
    /**
     * @brief Constructor for MainWindow
     * @param initial_tab Tab ID to select on startup (empty for default)
     * @param minimal_mode Whether to hide the tab bar
better_control.py     * @param floating_mode Whether to make the window float on tiling window managers
     */
    MainWindow(const std::string &initial_tab = "", bool minimal_mode = false, bool floating_mode = false)
    {
        initial_tab_ = initial_tab;
        minimal_mode_ = minimal_mode;
        prevent_auto_loading_ = !initial_tab_.empty();
        set_title("Ultimate Control");
        set_default_size(800, 600);

        // Set window type hint based on floating mode
        // DIALOG hint makes the window float on tiling window managers
        if (floating_mode)
        {
            set_type_hint(Gdk::WINDOW_TYPE_HINT_DIALOG);
        }
        else
        {
            set_type_hint(Gdk::WINDOW_TYPE_HINT_NORMAL);
        }

        // Check if running under Hyprland
        const char *hyprland_signature = getenv("HYPRLAND_INSTANCE_SIGNATURE");
        if (hyprland_signature != nullptr)
        {
            std::string cmd;
            if (floating_mode)
            {
                // Add a rule to make the window float
                cmd = "hyprctl --batch 'keyword windowrule float,class:^(ultimate-control)$'";
            }
            else
            {
                // Remove any existing floating rule
                cmd = "hyprctl --batch 'keyword windowrulev2 unset,class:^(ultimate-control)$'";
            }
            std::system(cmd.c_str());
        }

        vbox_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        add(vbox_);

        notebook_.set_scrollable(true);
        notebook_.set_show_tabs(!minimal_mode_);
        vbox_.pack_start(notebook_, Gtk::PACK_EXPAND_WIDGET);

        // Load tab configuration from settings
        tab_settings_ = std::make_shared<Settings::TabSettings>();

        // Connect to tab switch signal for lazy loading
        notebook_.signal_switch_page().connect(sigc::mem_fun(*this, &MainWindow::on_tab_switch));

        // Create tab placeholders
        create_tabs();

        // Create settings button on the right side of the notebook
        create_settings_button();

        // Handle window close event with quick exit to avoid hanging
        signal_delete_event().connect([](GdkEventAny *event) -> bool
                                      {
                                          std::quick_exit(0); // Force immediate exit without cleanup
                                          return true;        // Prevent the default handler from running
                                      });

        // Handle keybinds to close window
        signal_key_press_event().connect([](GdkEventKey *event) -> bool
                                         {
            if (event->keyval == 'q' || event->keyval == 'Q') {
                if (event->state & GDK_SHIFT_MASK) {
                    std::cout << "Application closed" << std::endl;
                    std::quick_exit(0);
                } else {
                    std::cout << "Application closed" << std::endl;
                    std::quick_exit(0);
                }
            }
            return false; });

        // Show all children
        show_all_children();

        // Switch to the initial tab if specified
        if (!initial_tab_.empty())
        {
            switch_to_tab(initial_tab_);
        }
    }

    /**
     * @brief Switch to a specific tab by ID
     * @param tab_id The ID of the tab to switch to
     *
     * If the tab content isn't loaded yet, this will trigger the loading process
     */
    void switch_to_tab(const std::string &tab_id)
    {
        // Find the tab in our map of tab widgets
        auto it = tab_widgets_.find(tab_id);
        if (it != tab_widgets_.end())
        {
            // Load the tab content if not already loaded or loading
            if (!it->second.loaded && !it->second.loading)
            {
                // Show loading indicator and start async loading
                show_loading_indicator(tab_id, it->second.page_num);
                load_tab_content_async(tab_id, it->second.page_num);
            }

            // Switch to the requested tab
            notebook_.set_current_page(it->second.page_num);
        }
    }

    /**
     * @brief Create a settings button on the right side of the notebook
     *
     * Creates a button that opens the settings window when clicked.
     */
    /**
     * @brief Simple class for a rotating settings icon using CSS animations
     */
    class RotatingSettingsIcon : public Gtk::Image
    {
    public:
        /**
         * @brief Constructor
         */
        RotatingSettingsIcon() : animating_(false)
        {
            // Set the icon and prepare CSS animation
            set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_MENU);
            set_name("settings-icon");

            // Create and load CSS for the rotation animation
            css_provider_ = Gtk::CssProvider::create();

            // Simple CSS for rotation animation
            const std::string css = ""
                                    "#settings-icon {"
                                    "    transition: all 200ms ease;"
                                    "}"
                                    "#settings-icon.rotate-active {"
                                    "    -gtk-icon-transform: rotate(360deg);"
                                    "    transition: all 600ms ease;"
                                    "}";

            try
            {
                css_provider_->load_from_data(css);
                get_style_context()->add_provider(css_provider_, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
            catch (const Glib::Error &ex)
            {
                std::cerr << "Error loading CSS: " << ex.what() << std::endl;
            }
        }

        /**
         * @brief Start the rotation animation
         */
        void start_animation()
        {
            if (animating_)
                return;
            animating_ = true;
            get_style_context()->add_class("rotate-active");
        }

        /**
         * @brief Reset the rotation animation
         */
        void reset_animation()
        {
            get_style_context()->remove_class("rotate-active");
            animating_ = false;
        }

    private:
        bool animating_;
        Glib::RefPtr<Gtk::CssProvider> css_provider_;
    };

    void create_settings_button()
    {
        // Create a settings button with our custom rotating icon
        auto settings_button = Gtk::make_managed<Gtk::Button>();
        settings_button->set_tooltip_text("Settings");
        settings_button->set_relief(Gtk::RELIEF_NONE);

        auto rotating_icon = Gtk::make_managed<RotatingSettingsIcon>();
        settings_button->add(*rotating_icon);

        // Add the button to the notebook
        auto button_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        button_box->pack_start(*settings_button, Gtk::PACK_SHRINK);
        button_box->set_margin_end(5);
        notebook_.set_action_widget(button_box, Gtk::PACK_END);
        button_box->show_all();

        // Connect click handler to open settings and animate icon
        settings_button->signal_clicked().connect([this, rotating_icon]()
                                                  {
            rotating_icon->start_animation();

            if (!settings_window_) {
                settings_window_ = std::make_unique<Settings::SettingsWindow>(*this);

                // Reset animation when settings window closes
                settings_window_->signal_hide().connect([rotating_icon]() {
                    rotating_icon->reset_animation();
                });

                settings_window_->set_settings_changed_callback([this]() {
                    std::cout << "Settings changed, restart required" << std::endl;
                    std::quick_exit(42);
                });
            }

            settings_window_->present(); });
    }

private:
    /**
     * @brief Create all tabs according to settings
     *
     * Creates tab placeholders in the order specified by settings.
     * Actual tab content is loaded lazily when the tab is selected.
     */
    void create_tabs()
    {
        // Clear existing tabs
        while (notebook_.get_n_pages() > 0)
        {
            notebook_.remove_page(-1);
        }
        tab_widgets_.clear();

        // Get tab order from settings
        auto tab_order = tab_settings_->get_tab_order();

        // If we have an initial tab specified, make sure it's enabled
        if (!initial_tab_.empty())
        {
            tab_settings_->set_tab_enabled(initial_tab_, true);
        }
        for (const auto &tab_id : tab_order)
        {
            if (!tab_settings_->is_tab_enabled(tab_id))
            {
                continue; // Skip disabled tabs
            }

            // Create a placeholder for lazy loading
            auto placeholder = Gtk::make_managed<Gtk::Box>();
            placeholder->set_size_request(100, 100);

            // Add the tab with appropriate icon and label
            if (tab_id == "volume")
            {
                add_tab(tab_id, placeholder, "audio-volume-high-symbolic", "Volume");
            }
            else if (tab_id == "wifi")
            {
                add_tab(tab_id, placeholder, "network-wireless-symbolic", "WiFi");
            }
            else if (tab_id == "display")
            {
                add_tab(tab_id, placeholder, "video-display-symbolic", "Display");
            }
            else if (tab_id == "power")
            {
                add_tab(tab_id, placeholder, "system-shutdown-symbolic", "Power");
            }
        }
    }

    /**
     * @brief Create a tab label with an icon and text
     * @param icon_name Name of the icon to use
     * @param label_text Text to display in the tab
     * @return Pointer to the created Gtk::Box containing the icon and label
     */
    Gtk::Box *create_tab_label(const std::string &icon_name, const std::string &label_text)
    {
        // Create horizontal box to hold icon and label
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
        auto icon = Gtk::make_managed<Gtk::Image>();
        icon->set_from_icon_name(icon_name, Gtk::ICON_SIZE_SMALL_TOOLBAR);
        auto label = Gtk::make_managed<Gtk::Label>(label_text);
        box->pack_start(*icon, Gtk::PACK_SHRINK);
        box->pack_start(*label, Gtk::PACK_SHRINK);
        box->show_all();
        return box;
    }

    /**
     * @brief Add a tab to the notebook
     * @param id Unique identifier for the tab
     * @param widget Widget to display in the tab
     * @param icon_name Name of the icon to use in the tab label
     * @param label_text Text to display in the tab label
     */
    void add_tab(const std::string &id, Gtk::Widget *widget, const std::string &icon_name, const std::string &label_text)
    {
        // Create tab label with icon and text
        auto box = create_tab_label(icon_name, label_text);

        // Add the tab
        int page_num = notebook_.append_page(*widget, *box);

        // Store the widget and page number
        tab_widgets_[id] = TabInfo{widget, page_num, false, false};

        // Create a dispatcher for this tab
        tab_loaded_dispatchers_[id].connect([this, id]()
                                            { on_tab_loaded(id); });
    }

    /**
     * @brief Handler for tab switch events
     * @param page Widget of the page being switched to
     * @param page_num Index of the page being switched to
     *
     * Implements lazy loading by detecting when a tab is selected
     * and loading its content if it hasn't been loaded yet.
     * Also prevents switching to the separator tab.
     */
    void on_tab_switch(Gtk::Widget *page, guint page_num)
    {
        // Guard against recursive calls that can happen during tab loading
        static bool loading = false;
        if (loading)
        {
            return;
        }

        // If we're preventing auto-loading (e.g., when starting with a specific tab),
        // don't load other tabs automatically
        if (prevent_auto_loading_)
        {
            // Only allow loading the initial tab
            std::string current_tab;
            for (const auto &[id, info] : tab_widgets_)
            {
                if (info.page_num == static_cast<int>(page_num))
                {
                    current_tab = id;
                    break;
                }
            }

            if (current_tab != initial_tab_)
            {
                return;
            }
        }

        // Find which tab was selected
        std::string tab_id_to_load;
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            for (const auto &[id, info] : tab_widgets_)
            {
                if (info.page_num == static_cast<int>(page_num) && !info.loaded && !info.loading)
                {
                    tab_id_to_load = id;
                    break;
                }
            }
        }

        // If we found a tab to load, load it
        if (!tab_id_to_load.empty())
        {
            // Set loading flag to prevent recursive calls
            loading = true;

            // Use a single-shot timer to delay loading slightly
            // This prevents UI freezes and GTK+ rendering issues during tab switching
            Glib::signal_timeout().connect_once([this, tab_id_to_load, page_num]()
                                                {
                // Show loading indicator and start async loading
                show_loading_indicator(tab_id_to_load, page_num);
                load_tab_content_async(tab_id_to_load, page_num); }, 50);

            // Reset loading flag after a short delay
            Glib::signal_timeout().connect_once([]()
                                                {
                // Use a new lambda to avoid capturing the static variable
                loading = false; }, 100);
        }
    }

    /**
     * @brief Create a loading indicator with spinner and text
     * @return Widget containing the loading indicator
     */
    Gtk::Widget *create_loading_indicator()
    {
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

    /**
     * @brief Show loading indicator for a tab
     * @param id ID of the tab being loaded
     * @param page_num Page number of the tab
     *
     * Replaces the tab's placeholder with a loading indicator
     */
    void show_loading_indicator(const std::string &id, int page_num)
    {
        std::lock_guard<std::mutex> lock(tab_mutex_);

        // Check if already loaded or loading
        if (tab_widgets_[id].loaded || tab_widgets_[id].loading)
        {
            return;
        }

        // Make sure the tab exists in the notebook
        if (tab_widgets_.find(id) == tab_widgets_.end())
        {
            return;
        }

        // Make sure the page number is valid
        if (page_num < 0 || page_num >= notebook_.get_n_pages())
        {
            return;
        }

        // Mark as loading
        tab_widgets_[id].loading = true;

        // Create a loading indicator
        auto loading_indicator = create_loading_indicator();

        // Get the icon name and label text for the tab
        std::string icon_name;
        std::string label_text;

        if (id == "volume")
        {
            icon_name = "audio-volume-high-symbolic";
            label_text = "Volume";
        }
        else if (id == "wifi")
        {
            icon_name = "network-wireless-symbolic";
            label_text = "WiFi";
        }
        else if (id == "display")
        {
            icon_name = "video-display-symbolic";
            label_text = "Display";
        }
        else if (id == "power")
        {
            icon_name = "system-shutdown-symbolic";
            label_text = "Power";
        }
        else
        {
            // Unknown tab
            tab_widgets_[id].loading = false;
            return;
        }

        try
        {
            // Create a new tab label with icon and text
            auto box = create_tab_label(icon_name, label_text);

            // Replace the tab placeholder with the loading indicator
            notebook_.remove_page(page_num);

            // Insert the loading indicator
            int new_page_num = notebook_.insert_page(*loading_indicator, *box, page_num);

            // Show the loading indicator
            loading_indicator->show_all();

            // Update the tab info
            tab_widgets_[id].widget = loading_indicator;
            tab_widgets_[id].page_num = new_page_num;

            // Display the loading indicator
            notebook_.set_current_page(new_page_num);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error showing loading indicator for tab " << id << ": " << e.what() << std::endl;
            tab_widgets_[id].loading = false;
        }
        catch (...)
        {
            std::cerr << "Unknown error showing loading indicator for tab " << id << std::endl;
            tab_widgets_[id].loading = false;
        }
    }

    /**
     * @brief Start asynchronous loading of tab content
     * @param id ID of the tab to load
     * @param page_num Page number of the tab
     *
     * Uses a timeout to keep the UI responsive during loading
     */
    void load_tab_content_async(const std::string &id, int page_num)
    {
        // Check if already loaded or loading
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            if (tab_widgets_[id].loaded || (tab_widgets_.find(id) == tab_widgets_.end()))
            {
                return;
            }
        }

        // Schedule content creation with a short delay
        // This keeps the UI responsive and allows the loading indicator to appear
        Glib::signal_timeout().connect_once([this, id, page_num]()
                                            {
            // This runs in the main thread after a short delay
            // Create the actual tab content
            create_tab_content(id, page_num); }, 100); // Short delay to allow UI to update
    }

    /**
     * @brief Create the actual content for a tab
     * @param id ID of the tab to create content for
     * @param page_num Page number of the tab
     *
     * Creates the appropriate tab widget based on the tab ID
     * and replaces the loading indicator with it
     */
    void create_tab_content(const std::string &id, int page_num)
    {
        // Check if already loaded
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            if (tab_widgets_[id].loaded || (tab_widgets_.find(id) == tab_widgets_.end()))
            {
                return;
            }
        }

        try
        {
            // Create the actual tab content
            Gtk::Widget *content = nullptr;
            std::string icon_name;
            std::string label_text;

            if (id == "volume")
            {
                content = Gtk::make_managed<Volume::VolumeTab>();
                icon_name = "audio-volume-high-symbolic";
                label_text = "Volume";
            }
            else if (id == "wifi")
            {
                content = Gtk::make_managed<Wifi::WifiTab>();
                icon_name = "network-wireless-symbolic";
                label_text = "WiFi";
            }
            else if (id == "display")
            {
                content = Gtk::make_managed<Display::DisplayTab>();
                icon_name = "video-display-symbolic";
                label_text = "Display";
            }
            else if (id == "power")
            {
                content = Gtk::make_managed<Power::PowerTab>();
                icon_name = "system-shutdown-symbolic";
                label_text = "Power";
            }
            else
            {
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

            // Create a new tab label with icon and text
            auto box = create_tab_label(icon_name, label_text);

            // Replace the loading indicator with the actual tab content
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

            // Switch to the newly created tab
            notebook_.set_current_page(new_page_num);

            // Notify that the tab has been loaded
            tab_loaded_dispatchers_[id].emit();
        }
        catch (const std::exception &e)
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            tab_widgets_[id].loading = false;
            tab_load_errors_[id] = e.what();
            std::cerr << "Error creating tab " << id << ": " << e.what() << std::endl;
        }
        catch (...)
        {
            std::lock_guard<std::mutex> lock(tab_mutex_);
            tab_widgets_[id].loading = false;
            tab_load_errors_[id] = "Unknown error";
            std::cerr << "Unknown error creating tab " << id << std::endl;
        }
    }

    /**
     * @brief Called when a tab has finished loading
     * @param id ID of the tab that was loaded
     *
     * Called via the dispatcher when a tab is fully loaded.
     * Can be used for post-loading operations.
     */
    void on_tab_loaded(const std::string &id)
    {
        std::cout << "Tab " << id << " loaded successfully" << std::endl;
    }

private:
    Gtk::Box vbox_;
    Gtk::Notebook notebook_;
    std::shared_ptr<Settings::TabSettings> tab_settings_;
    std::string initial_tab_;
    bool prevent_auto_loading_ = false;
    bool minimal_mode_ = false;

    // Tracks tab widgets and their loading state
    struct TabInfo
    {
        Gtk::Widget *widget;
        int page_num;
        bool loaded;
        bool loading; // Indicates if the tab is currently being loaded
    };
    std::map<std::string, TabInfo> tab_widgets_;

    // Components for asynchronous tab loading
    std::mutex tab_mutex_;
    std::map<std::string, Glib::Dispatcher> tab_loaded_dispatchers_;
    std::map<std::string, std::string> tab_load_errors_;

    // Settings window
    std::unique_ptr<Settings::SettingsWindow> settings_window_;
};

/**
 * @brief Application entry point
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return Application exit code
 */
int main(int argc, char *argv[])
{
    // Set up command-line option parsing
    Glib::OptionContext context;
    Glib::OptionGroup group("options", "Application Options", "Application options");

    // Variables to store command-line option values
    bool volume_opt = false;
    bool wifi_opt = false;
    bool display_opt = false;
    bool power_opt = false;
    bool settings_opt = false;
    bool minimal_opt = false;
    bool floating_opt = false;

    // Define the command-line option entries
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

    Glib::OptionEntry floating_entry;
    floating_entry.set_long_name("float");
    floating_entry.set_short_name('f');
    floating_entry.set_description("Start as a floating window on tiling window managers");
    group.add_entry(floating_entry, floating_opt);

    // Add the option group to the parsing context
    context.set_main_group(group);

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Error &error)
    {
        std::cerr << "Error parsing command line: " << error.what() << std::endl;
        return 1;
    }

    // Determine which tab to show initially based on command-line options
    std::string initial_tab;
    if (volume_opt)
    {
        initial_tab = "volume";
    }
    else if (wifi_opt)
    {
        initial_tab = "wifi";
    }
    else if (display_opt)
    {
        initial_tab = "display";
    }
    else if (power_opt)
    {
        initial_tab = "power";
    }
    else if (settings_opt)
    {
        initial_tab = "settings";
    }

    // Check if floating mode should be enabled from settings
    // Command-line option takes precedence over settings
    if (!floating_opt)
    {
        floating_opt = Core::get_setting("floating", "0") == "1";
    }

    // Initialize GTK application with unique identifier
    auto app = Gtk::Application::create(argc, argv, "com.example.ultimatecontrol");

    // Create the main window with the initial tab, minimal mode, and floating mode settings
    MainWindow window(initial_tab, minimal_opt, floating_opt);

    // Run the application
    return app->run(window);
}
