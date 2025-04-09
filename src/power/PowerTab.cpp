#include "PowerTab.hpp"
#include <iostream>

namespace Power {

PowerTab::PowerTab()
: manager_(std::make_shared<PowerManager>()),
  main_box_(Gtk::ORIENTATION_VERTICAL, 15)
{
    // Set up the main container
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    // Add a scrolled window
    scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the main box inside the scrolled window
    main_box_.set_margin_start(20);
    main_box_.set_margin_end(20);
    main_box_.set_margin_top(20);
    main_box_.set_margin_bottom(20);
    scrolled_window_.add(main_box_);

    // Create the different sections
    create_system_section();
    create_session_section();
    create_power_profiles_section();

    // Add frames to the main box
    main_box_.pack_start(system_frame_, Gtk::PACK_SHRINK);
    main_box_.pack_start(session_frame_, Gtk::PACK_SHRINK);
    main_box_.pack_start(profiles_frame_, Gtk::PACK_SHRINK);

    show_all_children();
    std::cout << "Power tab loaded!" << std::endl;
}

PowerTab::~PowerTab() = default;

void PowerTab::create_system_section() {
    // Set up the system section frame
    system_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    system_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
    system_box_.set_spacing(10);
    system_box_.set_margin_start(15);
    system_box_.set_margin_end(15);
    system_box_.set_margin_top(15);
    system_box_.set_margin_bottom(15);

    // Set up the system header
    system_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    system_header_box_.set_spacing(10);

    system_icon_.set_from_icon_name("system-shutdown-symbolic", Gtk::ICON_SIZE_DIALOG);
    system_label_.set_markup("<span size='large' weight='bold'>System Power</span>");
    system_label_.set_halign(Gtk::ALIGN_START);
    system_label_.set_valign(Gtk::ALIGN_CENTER);

    system_header_box_.pack_start(system_icon_, Gtk::PACK_SHRINK);
    system_header_box_.pack_start(system_label_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the system buttons
    system_buttons_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    system_buttons_box_.set_spacing(15);
    system_buttons_box_.set_homogeneous(true);

    // Configure shutdown button
    shutdown_button_.set_label("Shutdown");
    shutdown_button_.set_image_from_icon_name("system-shutdown-symbolic", Gtk::ICON_SIZE_BUTTON);
    shutdown_button_.set_always_show_image(true);
    shutdown_button_.set_tooltip_text("Power off the system");
    shutdown_button_.signal_clicked().connect([this]() {
        manager_->shutdown();
    });

    // Configure reboot button
    reboot_button_.set_label("Reboot");
    reboot_button_.set_image_from_icon_name("system-reboot-symbolic", Gtk::ICON_SIZE_BUTTON);
    reboot_button_.set_always_show_image(true);
    reboot_button_.set_tooltip_text("Restart the system");
    reboot_button_.signal_clicked().connect([this]() {
        manager_->reboot();
    });

    // Add buttons to the box
    system_buttons_box_.pack_start(shutdown_button_, Gtk::PACK_EXPAND_WIDGET);
    system_buttons_box_.pack_start(reboot_button_, Gtk::PACK_EXPAND_WIDGET);

    // Add components to the system box
    system_box_.pack_start(system_header_box_, Gtk::PACK_SHRINK);
    system_box_.pack_start(system_buttons_box_, Gtk::PACK_SHRINK);

    // Add the system box to the frame
    system_frame_.add(system_box_);
}

void PowerTab::create_session_section() {
    // Set up the session section frame
    session_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    session_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
    session_box_.set_spacing(10);
    session_box_.set_margin_start(15);
    session_box_.set_margin_end(15);
    session_box_.set_margin_top(15);
    session_box_.set_margin_bottom(15);

    // Set up the session header
    session_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    session_header_box_.set_spacing(10);

    session_icon_.set_from_icon_name("system-lock-screen-symbolic", Gtk::ICON_SIZE_DIALOG);
    session_label_.set_markup("<span size='large' weight='bold'>Session Actions</span>");
    session_label_.set_halign(Gtk::ALIGN_START);
    session_label_.set_valign(Gtk::ALIGN_CENTER);

    session_header_box_.pack_start(session_icon_, Gtk::PACK_SHRINK);
    session_header_box_.pack_start(session_label_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the session buttons
    session_buttons_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    session_buttons_box_.set_spacing(15);
    session_buttons_box_.set_homogeneous(true);

    // Configure suspend button
    suspend_button_.set_label("Suspend");
    suspend_button_.set_image_from_icon_name("system-suspend-symbolic", Gtk::ICON_SIZE_BUTTON);
    suspend_button_.set_always_show_image(true);
    suspend_button_.set_tooltip_text("Put the system to sleep");
    suspend_button_.signal_clicked().connect([this]() {
        manager_->suspend();
    });

    // Configure hibernate button
    hibernate_button_.set_label("Hibernate");
    hibernate_button_.set_image_from_icon_name("system-hibernate-symbolic", Gtk::ICON_SIZE_BUTTON);
    hibernate_button_.set_always_show_image(true);
    hibernate_button_.set_tooltip_text("Hibernate the system");
    hibernate_button_.signal_clicked().connect([this]() {
        manager_->hibernate();
    });

    // Configure lock button
    lock_button_.set_label("Lock Screen");
    lock_button_.set_image_from_icon_name("system-lock-screen-symbolic", Gtk::ICON_SIZE_BUTTON);
    lock_button_.set_always_show_image(true);
    lock_button_.set_tooltip_text("Lock the screen");
    lock_button_.signal_clicked().connect([]() {
        std::system("loginctl lock-session");
    });

    // Add buttons to the box
    session_buttons_box_.pack_start(suspend_button_, Gtk::PACK_EXPAND_WIDGET);
    session_buttons_box_.pack_start(hibernate_button_, Gtk::PACK_EXPAND_WIDGET);
    session_buttons_box_.pack_start(lock_button_, Gtk::PACK_EXPAND_WIDGET);

    // Add components to the session box
    session_box_.pack_start(session_header_box_, Gtk::PACK_SHRINK);
    session_box_.pack_start(session_buttons_box_, Gtk::PACK_SHRINK);

    // Add the session box to the frame
    session_frame_.add(session_box_);
}

void PowerTab::create_power_profiles_section() {
    // Set up the profiles section frame
    profiles_frame_.set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    profiles_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
    profiles_box_.set_spacing(10);
    profiles_box_.set_margin_start(15);
    profiles_box_.set_margin_end(15);
    profiles_box_.set_margin_top(15);
    profiles_box_.set_margin_bottom(15);

    // Set up the profiles header
    profiles_header_box_.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    profiles_header_box_.set_spacing(10);

    profiles_icon_.set_from_icon_name("power-profile-balanced-symbolic", Gtk::ICON_SIZE_DIALOG);
    profiles_label_.set_markup("<span size='large' weight='bold'>Power Profiles</span>");
    profiles_label_.set_halign(Gtk::ALIGN_START);
    profiles_label_.set_valign(Gtk::ALIGN_CENTER);

    profiles_header_box_.pack_start(profiles_icon_, Gtk::PACK_SHRINK);
    profiles_header_box_.pack_start(profiles_label_, Gtk::PACK_EXPAND_WIDGET);

    // Set up the profiles content
    profiles_content_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);
    profiles_content_box_.set_spacing(10);

    // Add description label
    Gtk::Label* description = Gtk::manage(new Gtk::Label());
    description->set_markup("Select a power profile to optimize battery life and performance:");
    description->set_halign(Gtk::ALIGN_START);
    profiles_content_box_.pack_start(*description, Gtk::PACK_SHRINK);

    // Set up the profile combo box
    profile_combo_.set_hexpand(true);

    // Get available power profiles
    auto profiles = manager_->list_power_profiles();
    profile_combo_.remove_all();
    for (const auto& profile : profiles) {
        profile_combo_.append(profile);
    }

    // Set the active profile
    if (!profiles.empty()) {
        profile_combo_.set_sensitive(true);

        auto current = manager_->get_current_power_profile();
        bool found = false;
        for (const auto& profile : profiles) {
            if (profile == current) {
                profile_combo_.set_active_text(profile);
                found = true;
                break;
            }
        }
        if (!found) {
            profile_combo_.set_active(0);
        }
    } else {
        profile_combo_.set_sensitive(false);
    }

    // Connect the signal
    profile_combo_.signal_changed().connect([this]() {
        auto selected = profile_combo_.get_active_text();
        if (!selected.empty()) {
            manager_->set_power_profile(selected);
        }
    });

    // Add the combo box to the content box
    profiles_content_box_.pack_start(profile_combo_, Gtk::PACK_SHRINK);

    // Add components to the profiles box
    profiles_box_.pack_start(profiles_header_box_, Gtk::PACK_SHRINK);
    profiles_box_.pack_start(profiles_content_box_, Gtk::PACK_SHRINK);

    // Add the profiles box to the frame
    profiles_frame_.add(profiles_box_);
}

} // namespace Power
