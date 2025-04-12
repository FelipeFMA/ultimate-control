/**
 * @file PowerTab.cpp
 * @brief Implementation of the power management tab
 *
 * This file implements the PowerTab class which provides a user interface
 * for system power operations like shutdown, reboot, suspend, and hibernate,
 * as well as power profile management.
 */

#include "PowerTab.hpp"
#include <iostream>

namespace Power
{

    /**
     * @brief Constructor for the power tab
     *
     * Initializes the power manager and creates the UI components.
     */
    PowerTab::PowerTab()
        : manager_(std::make_shared<PowerManager>()), // Initialize power manager
          main_box_(Gtk::ORIENTATION_VERTICAL, 15)    // Main container with 15px spacing
    {
        // Set up the main container orientation
        set_orientation(Gtk::ORIENTATION_VERTICAL);

        // Create and setup accelerator group for shortcuts
        accel_group_ = Gtk::AccelGroup::create();

        // We'll add the accel group to the parent toplevel window when realized (widget shown)
        signal_realize().connect([this]()
                                 {
        Gtk::Window* parent = dynamic_cast<Gtk::Window*>(get_toplevel());
        if (parent) {
            parent->add_accel_group(accel_group_);
        } });

        // Add a scrolled window to contain all sections
        scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

        // Set up the main box inside the scrolled window with margins
        main_box_.set_margin_start(20);
        main_box_.set_margin_end(20);
        main_box_.set_margin_top(20);
        main_box_.set_margin_bottom(20);
        scrolled_window_.add(main_box_);

        // Create the three main sections of the power tab
        create_system_section();
        create_session_section();
        create_power_profiles_section();

        // Add all section frames to the main box
        main_box_.pack_start(system_frame_, Gtk::PACK_SHRINK);
        main_box_.pack_start(session_frame_, Gtk::PACK_SHRINK);
        main_box_.pack_start(profiles_frame_, Gtk::PACK_SHRINK);

        show_all_children();
        std::cout << "Power tab loaded!" << std::endl;
    }

    /**
     * @brief Destructor for the power tab
     */
    PowerTab::~PowerTab() = default;

    /**
     * @brief Create the system power section
     *
     * Creates the UI components for system power operations
     * (shutdown and reboot).
     */
    void PowerTab::create_system_section()
    {
        // Configure the frame and container for the system power section
        system_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        system_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        system_box_.set_spacing(10);
        system_box_.set_margin_start(15);
        system_box_.set_margin_end(15);
        system_box_.set_margin_top(15);
        system_box_.set_margin_bottom(15);

        // Configure the header for the system power section
        system_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        system_header_box_.set_spacing(10);

        system_icon_.set_from_icon_name("system-shutdown-symbolic", Gtk::ICON_SIZE_DIALOG);
        system_label_.set_markup("<span size='large' weight='bold'>System Power</span>");
        system_label_.set_halign(Gtk::ALIGN_START);
        system_label_.set_valign(Gtk::ALIGN_CENTER);

        system_header_box_.pack_start(system_icon_, Gtk::PACK_SHRINK);
        system_header_box_.pack_start(system_label_, Gtk::PACK_EXPAND_WIDGET);

        // Add settings button to the header for configuring power commands
        add_settings_button_to_header(system_header_box_);

        // Configure the container for system power buttons
        system_buttons_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        system_buttons_box_.set_spacing(15);
        system_buttons_box_.set_homogeneous(true);

        // Configure the shutdown button with icon and click handler, add shortcut hint and accelerator
        shutdown_button_.set_label("Shutdown [S]");
        shutdown_button_.set_image_from_icon_name("system-shutdown-symbolic", Gtk::ICON_SIZE_BUTTON);
        shutdown_button_.set_always_show_image(true);
        shutdown_button_.set_tooltip_text("Power off the system");
        shutdown_button_.signal_clicked().connect([this]()
                                                  { manager_->shutdown(); });
        // Accelerator: S
        shutdown_button_.add_accelerator("clicked", accel_group_, GDK_KEY_s, Gdk::ModifierType(0), Gtk::ACCEL_VISIBLE);

        // Configure the reboot button with icon and click handler, add shortcut hint and accelerator
        reboot_button_.set_label("Reboot [R]");
        reboot_button_.set_image_from_icon_name("system-reboot-symbolic", Gtk::ICON_SIZE_BUTTON);
        reboot_button_.set_always_show_image(true);
        reboot_button_.set_tooltip_text("Restart the system");
        reboot_button_.signal_clicked().connect([this]()
                                                { manager_->reboot(); });
        // Accelerator: R
        reboot_button_.add_accelerator("clicked", accel_group_, GDK_KEY_r, Gdk::ModifierType(0), Gtk::ACCEL_VISIBLE);

        // Add both buttons to the buttons container
        system_buttons_box_.pack_start(shutdown_button_, Gtk::PACK_EXPAND_WIDGET);
        system_buttons_box_.pack_start(reboot_button_, Gtk::PACK_EXPAND_WIDGET);

        // Assemble the system section components
        system_box_.pack_start(system_header_box_, Gtk::PACK_SHRINK);
        system_box_.pack_start(system_buttons_box_, Gtk::PACK_SHRINK);

        // Add the assembled system box to the frame
        system_frame_.add(system_box_);
    }

    /**
     * @brief Create the session actions section
     *
     * Creates the UI components for session actions
     * (suspend, hibernate, and lock screen).
     */
    void PowerTab::create_session_section()
    {
        // Configure the frame and container for the session section
        session_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        session_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        session_box_.set_spacing(10);
        session_box_.set_margin_start(15);
        session_box_.set_margin_end(15);
        session_box_.set_margin_top(15);
        session_box_.set_margin_bottom(15);

        // Configure the header for the session section
        session_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        session_header_box_.set_spacing(10);

        session_icon_.set_from_icon_name("system-lock-screen-symbolic", Gtk::ICON_SIZE_DIALOG);
        session_label_.set_markup("<span size='large' weight='bold'>Session Actions</span>");
        session_label_.set_halign(Gtk::ALIGN_START);
        session_label_.set_valign(Gtk::ALIGN_CENTER);

        session_header_box_.pack_start(session_icon_, Gtk::PACK_SHRINK);
        session_header_box_.pack_start(session_label_, Gtk::PACK_EXPAND_WIDGET);

        // Configure the container for session action buttons
        session_buttons_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        session_buttons_box_.set_spacing(15);
        session_buttons_box_.set_homogeneous(true);

        // Configure the suspend button with icon and click handler, add shortcut hint and accelerator
        suspend_button_.set_label("Suspend [U]");
        suspend_button_.set_image_from_icon_name("system-suspend-symbolic", Gtk::ICON_SIZE_BUTTON);
        suspend_button_.set_always_show_image(true);
        suspend_button_.set_tooltip_text("Put the system to sleep");
        suspend_button_.signal_clicked().connect([this]()
                                                 { manager_->suspend(); });
        // Accelerator: U
        suspend_button_.add_accelerator("clicked", accel_group_, GDK_KEY_u, Gdk::ModifierType(0), Gtk::ACCEL_VISIBLE);

        // Configure the hibernate button with icon and click handler, add shortcut hint and accelerator
        hibernate_button_.set_label("Hibernate [H]");
        hibernate_button_.set_image_from_icon_name("system-hibernate-symbolic", Gtk::ICON_SIZE_BUTTON);
        hibernate_button_.set_always_show_image(true);
        hibernate_button_.set_tooltip_text("Hibernate the system");
        hibernate_button_.signal_clicked().connect([this]()
                                                   { manager_->hibernate(); });
        // Accelerator: H
        hibernate_button_.add_accelerator("clicked", accel_group_, GDK_KEY_h, Gdk::ModifierType(0), Gtk::ACCEL_VISIBLE);

        // Configure the lock screen button with icon and click handler, add shortcut hint and accelerator
        lock_button_.set_label("Lock [L]");
        lock_button_.set_image_from_icon_name("system-lock-screen-symbolic", Gtk::ICON_SIZE_BUTTON);
        lock_button_.set_always_show_image(true);
        lock_button_.set_tooltip_text("Lock the screen");
        lock_button_.signal_clicked().connect([this]()
                                              { std::system(manager_->get_settings()->get_command("lock").c_str()); });
        // Accelerator: L
        lock_button_.add_accelerator("clicked", accel_group_, GDK_KEY_l, Gdk::ModifierType(0), Gtk::ACCEL_VISIBLE);

        // Add all three buttons to the buttons container
        session_buttons_box_.pack_start(suspend_button_, Gtk::PACK_EXPAND_WIDGET);
        session_buttons_box_.pack_start(hibernate_button_, Gtk::PACK_EXPAND_WIDGET);
        session_buttons_box_.pack_start(lock_button_, Gtk::PACK_EXPAND_WIDGET);

        // Assemble the session section components
        session_box_.pack_start(session_header_box_, Gtk::PACK_SHRINK);
        session_box_.pack_start(session_buttons_box_, Gtk::PACK_SHRINK);

        // Add the assembled session box to the frame
        session_frame_.add(session_box_);
    }

    /**
     * @brief Create the power profiles section
     *
     * Creates the UI components for power profile management.
     */
    void PowerTab::create_power_profiles_section()
    {
        // Configure the frame and container for the power profiles section
        profiles_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        profiles_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        profiles_box_.set_spacing(10);
        profiles_box_.set_margin_start(15);
        profiles_box_.set_margin_end(15);
        profiles_box_.set_margin_top(15);
        profiles_box_.set_margin_bottom(15);

        // Configure the header for the power profiles section
        profiles_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        profiles_header_box_.set_spacing(10);

        profiles_icon_.set_from_icon_name("power-profile-balanced-symbolic", Gtk::ICON_SIZE_DIALOG);
        profiles_label_.set_markup("<span size='large' weight='bold'>Power Profiles</span>");
        profiles_label_.set_halign(Gtk::ALIGN_START);
        profiles_label_.set_valign(Gtk::ALIGN_CENTER);

        profiles_header_box_.pack_start(profiles_icon_, Gtk::PACK_SHRINK);
        profiles_header_box_.pack_start(profiles_label_, Gtk::PACK_EXPAND_WIDGET);

        // Configure the container for power profiles content
        profiles_content_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
        profiles_content_box_.set_spacing(10);

        // Add descriptive text explaining power profiles
        Gtk::Label *description = Gtk::manage(new Gtk::Label());
        description->set_markup("Select a power profile to optimize battery life and performance:");
        description->set_halign(Gtk::ALIGN_START);
        profiles_content_box_.pack_start(*description, Gtk::PACK_SHRINK);

        // Configure the dropdown for selecting power profiles
        profile_combo_.set_hexpand(true);

        // Populate the dropdown with available power profiles
        auto profiles = manager_->list_power_profiles();
        profile_combo_.remove_all();
        for (const auto &profile : profiles)
        {
            profile_combo_.append(profile);
        }

        // Set the currently active profile in the dropdown
        if (!profiles.empty())
        {
            profile_combo_.set_sensitive(true);

            auto current = manager_->get_current_power_profile();
            bool found = false;
            for (const auto &profile : profiles)
            {
                if (profile == current)
                {
                    profile_combo_.set_active_text(profile);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                profile_combo_.set_active(0);
            }
        }
        else
        {
            profile_combo_.set_sensitive(false);
        }

        // Connect the change signal to update the power profile
        profile_combo_.signal_changed().connect([this]()
                                                {
        auto selected = profile_combo_.get_active_text();
        if (!selected.empty()) {
            manager_->set_power_profile(selected);
        } });

        // Add the dropdown to the content container
        profiles_content_box_.pack_start(profile_combo_, Gtk::PACK_SHRINK);

        // Assemble the power profiles section components
        profiles_box_.pack_start(profiles_header_box_, Gtk::PACK_SHRINK);
        profiles_box_.pack_start(profiles_content_box_, Gtk::PACK_SHRINK);

        // Add the assembled profiles box to the frame
        profiles_frame_.add(profiles_box_);
    }

    /**
     * @brief Add a settings button to a section header
     * @param header_box The header box to add the button to
     *
     * Adds a settings button with a cog icon to a section header.
     * When clicked, the button opens the power settings dialog.
     */
    void PowerTab::add_settings_button_to_header(Gtk::Box &header_box)
    {
        // Create a button with no relief (flat appearance)
        auto settings_button = Gtk::make_managed<Gtk::Button>();
        settings_button->set_relief(Gtk::RELIEF_NONE);
        settings_button->set_tooltip_text("Configure power commands");

        // Add a settings/cog icon to the button
        auto settings_icon = Gtk::make_managed<Gtk::Image>();
        settings_icon->set_from_icon_name("emblem-system-symbolic", Gtk::ICON_SIZE_BUTTON);
        settings_button->set_image(*settings_icon);

        // Connect the button click to open settings dialog
        settings_button->signal_clicked().connect(sigc::mem_fun(*this, &PowerTab::on_settings_clicked));

        // Add the button to the right side of the header
        header_box.pack_end(*settings_button, Gtk::PACK_SHRINK);
    }

    /**
     * @brief Handler for settings button clicks
     *
     * Opens the power settings dialog to configure power commands.
     */
    void PowerTab::on_settings_clicked()
    {
        // Get the parent window for the dialog
        Gtk::Window *parent = dynamic_cast<Gtk::Window *>(get_toplevel());
        if (!parent)
            return;

        // Create and show the power settings dialog
        PowerSettingsDialog dialog(*parent, manager_->get_settings());
        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK)
        {
            // Save the settings if OK was clicked
            dialog.save_settings();
        }
    }

} // namespace Power
