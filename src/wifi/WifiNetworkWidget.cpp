/**
 * @file WifiNetworkWidget.cpp
 * @brief Implementation of the WiFi network widget UI component
 *
 * This file implements the WifiNetworkWidget class which displays a single
 * WiFi network in the UI with signal strength, security status, and connection
 * controls. It provides functionality for connecting to networks, forgetting
 * saved networks, and sharing networks via QR codes.
 */

#include "WifiNetworkWidget.hpp"
#include <gtkmm/messagedialog.h>
#include <gtkmm/spinner.h>
#include <glibmm/thread.h>
#include <iostream>

namespace Wifi
{

    /**
     * @brief Constructor for the WiFi network widget
     * @param network The network to display
     * @param manager Shared pointer to the WiFi manager
     *
     * Creates a widget that displays information about a WiFi network and
     * provides controls for connecting, forgetting, and sharing the network.
     */
    WifiNetworkWidget::WifiNetworkWidget(const Network &network, std::shared_ptr<WifiManager> manager)
        : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5),
          manager_(std::move(manager)),
          network_(network),
          network_info_box_(Gtk::ORIENTATION_HORIZONTAL, 8),
          controls_box_(Gtk::ORIENTATION_HORIZONTAL, 8),
          ssid_label_(network.ssid),
          signal_label_(std::to_string(network.signal_strength) + "%"),
          status_label_(network.connected ? "Connected" : (network.secured ? "Secured" : "Open")),
          connect_button_(),
          forget_button_(),
          share_button_()
    {
        // Set up the main container with margins for better spacing
        set_margin_start(10);
        set_margin_end(10);
        set_margin_top(8);
        set_margin_bottom(8);

        // Add a frame around the widget for visual separation between networks
        Gtk::Frame *frame = Gtk::manage(new Gtk::Frame());
        frame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
        pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);

        // Create a vertical box inside the frame with padding
        Gtk::Box *inner_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
        inner_box->set_margin_start(10);
        inner_box->set_margin_end(10);
        inner_box->set_margin_top(10);
        inner_box->set_margin_bottom(10);
        frame->add(*inner_box);

        // Initialize the icons based on network properties
        update_signal_icon(network.signal_strength);
        update_security_icon(network.secured);
        update_connection_status(network.connected);

        // Make the SSID label bold to emphasize the network name
        Pango::AttrList attrs;
        auto font_desc = Pango::FontDescription("Bold");
        auto attr = Pango::Attribute::create_attr_font_desc(font_desc);
        attrs.insert(attr);
        ssid_label_.set_attributes(attrs);

        // Add network info widgets (signal icon, SSID, security icon, status) to the info box
        network_info_box_.pack_start(signal_icon_, Gtk::PACK_SHRINK);
        network_info_box_.pack_start(ssid_label_, Gtk::PACK_SHRINK);
        network_info_box_.pack_start(security_icon_, Gtk::PACK_SHRINK);
        network_info_box_.pack_start(status_icon_, Gtk::PACK_SHRINK);

        // Create and add signal strength indicator with label
        Gtk::Box *signal_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 4));
        Gtk::Label *signal_prefix = Gtk::manage(new Gtk::Label("Signal Strength:"));
        signal_box->pack_start(*signal_prefix, Gtk::PACK_SHRINK);

        // Set the signal strength percentage text
        signal_label_.set_text(std::to_string(network_.signal_strength) + "%");

        signal_box->pack_start(signal_label_, Gtk::PACK_SHRINK);

        // Set up the connect/disconnect button with appropriate icon and label
        if (network.connected)
        {
            connect_button_.set_image_from_icon_name("network-wireless-connected-symbolic", Gtk::ICON_SIZE_BUTTON);
            connect_button_.set_label("Disconnect");
        }
        else
        {
            connect_button_.set_image_from_icon_name("network-wireless-signal-excellent-symbolic", Gtk::ICON_SIZE_BUTTON);
            connect_button_.set_label("Connect");
        }

        // Set up the forget button with trash icon
        forget_button_.set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);
        forget_button_.set_label("Forget");
        forget_button_.set_tooltip_text("Forget this network");

        // Set up the share button with share icon for QR code generation
        share_button_.set_image_from_icon_name("emblem-shared-symbolic", Gtk::ICON_SIZE_BUTTON);
        share_button_.set_label("Share");
        share_button_.set_tooltip_text("Share network via QR code");

        // Add buttons to the controls box (connect on right, others on left)
        controls_box_.pack_end(connect_button_, Gtk::PACK_SHRINK);
        controls_box_.pack_start(forget_button_, Gtk::PACK_SHRINK);
        controls_box_.pack_start(share_button_, Gtk::PACK_SHRINK);

        // Add all components to the inner box in vertical order
        inner_box->pack_start(network_info_box_, Gtk::PACK_SHRINK);
        inner_box->pack_start(*signal_box, Gtk::PACK_SHRINK);
        inner_box->pack_start(controls_box_, Gtk::PACK_SHRINK);

        // Connect button click handlers
        connect_button_.signal_clicked().connect(sigc::mem_fun(*this, &WifiNetworkWidget::on_connect_clicked));
        forget_button_.signal_clicked().connect(sigc::mem_fun(*this, &WifiNetworkWidget::on_forget_clicked));
        share_button_.signal_clicked().connect(sigc::mem_fun(*this, &WifiNetworkWidget::on_share_clicked));

        show_all_children();
    }

    /**
     * @brief Destructor for the WiFi network widget
     */
    WifiNetworkWidget::~WifiNetworkWidget() = default;

    /**
     * @brief Convert signal strength to a human-readable quality string
     * @param signal_strength Signal strength as a percentage (0-100)
     * @return String representation of the signal strength
     */
    std::string WifiNetworkWidget::convert_signal_to_quality(int signal_strength)
    {
        // Simply return the signal strength as a percentage string
        // The value from nmcli is already a percentage (0-100)
        return std::to_string(signal_strength) + "%";
    }

    /**
     * @brief Update the signal strength icon based on signal level
     * @param signal_strength Signal strength as a percentage (0-100)
     *
     * Sets the appropriate icon based on signal strength ranges.
     */
    void WifiNetworkWidget::update_signal_icon(int signal_strength)
    {
        std::string icon_name;

        // Select the appropriate icon based on signal strength ranges
        if (signal_strength < 20)
        {
            icon_name = "network-wireless-signal-none-symbolic";
        }
        else if (signal_strength < 40)
        {
            icon_name = "network-wireless-signal-weak-symbolic";
        }
        else if (signal_strength < 60)
        {
            icon_name = "network-wireless-signal-ok-symbolic";
        }
        else if (signal_strength < 80)
        {
            icon_name = "network-wireless-signal-good-symbolic";
        }
        else
        {
            icon_name = "network-wireless-signal-excellent-symbolic";
        }

        signal_icon_.set_from_icon_name(icon_name, Gtk::ICON_SIZE_LARGE_TOOLBAR);
    }

    /**
     * @brief Update the security icon based on network encryption
     * @param secured Whether the network uses encryption
     *
     * Sets a lock icon for secured networks or an unlocked icon for open networks.
     */
    void WifiNetworkWidget::update_security_icon(bool secured)
    {
        if (secured)
        {
            security_icon_.set_from_icon_name("channel-secure-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            security_icon_.set_tooltip_text("Secured Network");
        }
        else
        {
            security_icon_.set_from_icon_name("channel-insecure-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            security_icon_.set_tooltip_text("Open Network");
        }
    }

    /**
     * @brief Update the connection status icon
     * @param connected Whether the device is currently connected to this network
     *
     * Shows a connected icon if the device is connected to this network.
     */
    void WifiNetworkWidget::update_connection_status(bool connected)
    {
        if (connected)
        {
            status_icon_.set_from_icon_name("network-wireless-connected-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR);
            status_icon_.set_tooltip_text("Connected");
        }
        else
        {
            status_icon_.clear();
        }
    }

    /**
     * @brief Handler for connect/disconnect button clicks
     *
     * If already connected, disconnects from the network.
     * If not connected, attempts to connect using saved credentials first.
     * If that fails and the network is secured, prompts for a password.
     */
    void WifiNetworkWidget::on_connect_clicked()
    {
        // Store the target SSID for later reference
        std::string target_ssid = network_.ssid;

        if (network_.connected)
        {
            manager_->disconnect();
        }
        else
        {
            // Determine security type based on network properties
            std::string security_type = network_.secured ? "wpa-psk" : "";

            // Disable the connect button and show a spinner while connecting
            connect_button_.set_sensitive(false);
            connect_button_.set_label("Connecting...");

            // Create a spinner to show while connecting
            Gtk::Spinner *spinner = Gtk::manage(new Gtk::Spinner());
            spinner->start();
            connect_button_.set_image(*spinner);
            connect_button_.show_all_children();

            // First try to connect with empty password - this will use saved credentials if available
            std::cout << "Trying to connect to " << target_ssid << " using saved credentials..." << std::endl;

            // Connect asynchronously to avoid freezing the UI
            manager_->connect_async(target_ssid, "", security_type,
                                    [this, target_ssid, security_type, spinner](bool success, const std::string &ssid)
                                    {
                                        // This callback runs in the main thread

                                        // Restore the connect button
                                        connect_button_.set_sensitive(true);
                                        connect_button_.set_label("Connect");
                                        connect_button_.set_image_from_icon_name("network-wireless-signal-excellent-symbolic", Gtk::ICON_SIZE_BUTTON);

                                        if (success)
                                        {
                                            // Show a success message dialog
                                            Gtk::MessageDialog success_dialog(*dynamic_cast<Gtk::Window *>(get_toplevel()),
                                                                              "Successfully connected to " + target_ssid,
                                                                              false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                                            success_dialog.set_secondary_text("Connected using saved credentials");
                                            success_dialog.run();
                                        }
                                        else if (network_.secured)
                                        {
                                            // If connection with saved credentials failed and it's a secured network, ask for password
                                            // Create a modal dialog for password entry with appropriate styling
                                            Gtk::Dialog dialog("Enter WiFi Password", *dynamic_cast<Gtk::Window *>(get_toplevel()), true);
                                            dialog.set_default_size(300, -1);
                                            dialog.set_border_width(10);

                                            // Create a horizontal box with a lock icon for the dialog
                                            Gtk::Box *content_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
                                            Gtk::Image *lock_icon = Gtk::manage(new Gtk::Image());
                                            lock_icon->set_from_icon_name("channel-secure-symbolic", Gtk::ICON_SIZE_DIALOG);
                                            content_box->pack_start(*lock_icon, Gtk::PACK_SHRINK);

                                            // Create a vertical box for the network name label and password entry field
                                            Gtk::Box *entry_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));

                                            // Add network name label with bold formatting
                                            Gtk::Label *network_label = Gtk::manage(new Gtk::Label());
                                            network_label->set_markup("<b>" + target_ssid + "</b>");
                                            network_label->set_halign(Gtk::ALIGN_START);
                                            entry_box->pack_start(*network_label, Gtk::PACK_SHRINK);

                                            // Add password label and masked entry field
                                            Gtk::Label *password_label = Gtk::manage(new Gtk::Label("Password:"));
                                            password_label->set_halign(Gtk::ALIGN_START);
                                            entry_box->pack_start(*password_label, Gtk::PACK_SHRINK);

                                            Gtk::Entry *entry = Gtk::manage(new Gtk::Entry());
                                            entry->set_visibility(false);
                                            entry->set_invisible_char('*');
                                            entry->set_activates_default(true);
                                            entry_box->pack_start(*entry, Gtk::PACK_SHRINK);

                                            content_box->pack_start(*entry_box, Gtk::PACK_EXPAND_WIDGET);

                                            dialog.get_content_area()->pack_start(*content_box, Gtk::PACK_EXPAND_WIDGET);
                                            dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
                                            dialog.add_button("Connect", Gtk::RESPONSE_OK);
                                            dialog.set_default_response(Gtk::RESPONSE_OK);

                                            dialog.show_all_children();

                                            int result = dialog.run();
                                            if (result == Gtk::RESPONSE_OK)
                                            {
                                                std::string password = entry->get_text();

                                                // Log connection attempt
                                                std::cout << "Connecting to " << target_ssid << " with password..." << std::endl;

                                                // Disable the connect button and show a spinner while connecting
                                                connect_button_.set_sensitive(false);
                                                connect_button_.set_label("Connecting...");

                                                Gtk::Spinner *spinner = Gtk::manage(new Gtk::Spinner());
                                                spinner->start();
                                                connect_button_.set_image(*spinner);
                                                connect_button_.show_all_children();

                                                // Connect asynchronously with the provided password
                                                manager_->connect_async(target_ssid, password, security_type,
                                                                        [this, target_ssid](bool success, const std::string &ssid)
                                                                        {
                                                                            // Restore the connect button
                                                                            connect_button_.set_sensitive(true);
                                                                            connect_button_.set_label("Connect");
                                                                            connect_button_.set_image_from_icon_name("network-wireless-signal-excellent-symbolic", Gtk::ICON_SIZE_BUTTON);

                                                                            if (success)
                                                                            {
                                                                                Gtk::MessageDialog success_dialog(*dynamic_cast<Gtk::Window *>(get_toplevel()),
                                                                                                                  "Successfully connected to " + target_ssid,
                                                                                                                  false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                                                                                success_dialog.run();
                                                                            }
                                                                            else
                                                                            {
                                                                                Gtk::MessageDialog error_dialog(*dynamic_cast<Gtk::Window *>(get_toplevel()),
                                                                                                                "Failed to connect to " + target_ssid,
                                                                                                                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                                                                                error_dialog.set_secondary_text("Please check your password and try again.");
                                                                                error_dialog.run();
                                                                            }
                                                                        });
                                            }
                                        }
                                    });
        }
    }

    /**
     * @brief Handler for forget button clicks
     *
     * Shows a confirmation dialog and if confirmed, removes all saved
     * credentials for the network using the WiFi manager.
     */
    void WifiNetworkWidget::on_forget_clicked()
    {
        // Store the target SSID for later reference
        std::string target_ssid = network_.ssid;

        // Create a confirmation dialog with Yes/No buttons
        Gtk::MessageDialog dialog(*dynamic_cast<Gtk::Window *>(get_toplevel()),
                                  "Are you sure you want to forget this network?",
                                  false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);

        // Add WiFi icon and network name to the dialog content
        Gtk::Box *content_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
        Gtk::Image *wifi_icon = Gtk::manage(new Gtk::Image());
        wifi_icon->set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_DIALOG);
        content_box->pack_start(*wifi_icon, Gtk::PACK_SHRINK);

        Gtk::Label *network_label = Gtk::manage(new Gtk::Label());
        network_label->set_markup("<b>" + target_ssid + "</b>");
        network_label->set_halign(Gtk::ALIGN_START);
        content_box->pack_start(*network_label, Gtk::PACK_SHRINK);

        dialog.get_content_area()->pack_start(*content_box, Gtk::PACK_SHRINK);
        dialog.show_all_children();

        int result = dialog.run();
        if (result == Gtk::RESPONSE_YES)
        {
            manager_->forget_network(target_ssid);

            // Show a success message dialog
            Gtk::MessageDialog success_dialog(*dynamic_cast<Gtk::Window *>(get_toplevel()),
                                              "Network forgotten",
                                              false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            success_dialog.set_secondary_text("Successfully removed all saved connections for " + target_ssid);
            success_dialog.run();
        }
    }

    /**
     * @brief Handler for share button clicks
     *
     * Creates a QR code for the WiFi network that can be scanned by phones
     * to connect. For secured networks, prompts for the password if not connected.
     */
    void WifiNetworkWidget::on_share_clicked()
    {
        // Store the target SSID for later reference
        std::string target_ssid = network_.ssid;

        // Create a dialog window to display the QR code
        Gtk::Dialog dialog("Share WiFi Network", *dynamic_cast<Gtk::Window *>(get_toplevel()), true);
        dialog.set_default_size(350, 400);
        dialog.set_border_width(10);

        // Create a vertical box for the dialog content with padding
        Gtk::Box *content_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
        content_box->set_border_width(10);

        // Add WiFi icon and network name in the header
        Gtk::Box *header_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
        Gtk::Image *wifi_icon = Gtk::manage(new Gtk::Image());
        wifi_icon->set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_DIALOG);
        header_box->pack_start(*wifi_icon, Gtk::PACK_SHRINK);

        Gtk::Label *network_label = Gtk::manage(new Gtk::Label());
        network_label->set_markup("<span size='large'><b>" + target_ssid + "</b></span>");
        network_label->set_halign(Gtk::ALIGN_START);
        header_box->pack_start(*network_label, Gtk::PACK_SHRINK);

        content_box->pack_start(*header_box, Gtk::PACK_SHRINK);

        // Add a horizontal separator line
        Gtk::Separator *separator = Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
        content_box->pack_start(*separator, Gtk::PACK_SHRINK);

        // Create a drawing area widget for rendering the QR code
        Gtk::DrawingArea *drawing_area = Gtk::manage(new Gtk::DrawingArea());
        drawing_area->set_size_request(300, 300);

        // Initialize the QR code generator with medium error correction
        Utils::QRCode qrcode(Utils::QRCode::Version::V3, Utils::QRCode::ErrorCorrection::M);

        // Get password for secured networks if not already connected
        std::string password;
        if (network_.secured && !network_.connected)
        {
            // Create a password entry dialog
            Gtk::Dialog pwd_dialog("Enter WiFi Password", *dynamic_cast<Gtk::Window *>(get_toplevel()), true);
            pwd_dialog.set_default_size(300, -1);
            pwd_dialog.set_border_width(10);

            Gtk::Box *pwd_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
            pwd_box->set_border_width(10);

            Gtk::Label *pwd_label = Gtk::manage(new Gtk::Label("Enter the password to include in the QR code:"));
            pwd_box->pack_start(*pwd_label, Gtk::PACK_SHRINK);

            Gtk::Entry *pwd_entry = Gtk::manage(new Gtk::Entry());
            pwd_entry->set_visibility(false);
            pwd_entry->set_invisible_char('*');
            pwd_box->pack_start(*pwd_entry, Gtk::PACK_SHRINK);

            pwd_dialog.get_content_area()->pack_start(*pwd_box, Gtk::PACK_SHRINK);
            pwd_dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
            pwd_dialog.add_button("OK", Gtk::RESPONSE_OK);
            pwd_dialog.set_default_response(Gtk::RESPONSE_OK);

            pwd_dialog.show_all_children();

            int result = pwd_dialog.run();
            if (result == Gtk::RESPONSE_OK)
            {
                password = pwd_entry->get_text();
            }
            else
            {
                return; // User cancelled
            }
        }

        // Format the WiFi network information for the QR code
        std::string auth_type = network_.secured ? "WPA" : "nopass";
        std::string qr_data = Utils::QRCode::formatWifiNetwork(target_ssid, password, false, auth_type);

        // Encode the WiFi data into the QR code
        qrcode.encode(qr_data);

        // Set up the drawing handler to render the QR code when the area is exposed
        drawing_area->signal_draw().connect([&qrcode, drawing_area](const Cairo::RefPtr<Cairo::Context> &cr) -> bool
                                            {
        Gtk::Allocation allocation = drawing_area->get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();
        const int size = std::min(width, height);

        // Center the QR code in the drawing area
        double x = (width - size) / 2.0;
        double y = (height - size) / 2.0;

        // Fill the background with white
        cr->set_source_rgb(1.0, 1.0, 1.0); // White
        cr->paint();

        // Draw the QR code in black
        cr->set_source_rgb(0.0, 0.0, 0.0); // Black
        qrcode.draw(cr, x, y, size);

        return true; });

        content_box->pack_start(*drawing_area, Gtk::PACK_EXPAND_WIDGET);

        // Add instructions for using the QR code
        Gtk::Label *instructions = Gtk::manage(new Gtk::Label("Scan this QR code with a phone camera\nor WiFi configuration app to connect"));
        instructions->set_line_wrap(true);
        content_box->pack_start(*instructions, Gtk::PACK_SHRINK);

        // Add the complete content box to the dialog
        dialog.get_content_area()->pack_start(*content_box, Gtk::PACK_EXPAND_WIDGET);

        // Add a close button to dismiss the dialog
        dialog.add_button("Close", Gtk::RESPONSE_CLOSE);
        dialog.set_default_response(Gtk::RESPONSE_CLOSE);

        // Show all widgets and run the dialog modally
        dialog.show_all_children();
        dialog.run();
    }

} // namespace Wifi
