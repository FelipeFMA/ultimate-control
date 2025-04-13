/**
 * @file WifiManager.cpp
 * @brief Implementation of the WiFi management functionality
 *
 * This file implements the WifiManager class which provides an interface
 * for scanning, connecting to, and managing WiFi networks using NetworkManager.
 * It uses the PIMPL idiom to hide implementation details.
 */

#include "WifiManager.hpp"
#include "utils/QRCode.hpp"
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <algorithm>
#include <ctime>

namespace Wifi
{

    /**
     * @class WifiManager::Impl
     * @brief Private implementation of the WifiManager class
     *
     * This class implements the actual WiFi management functionality using
     * NetworkManager's command-line interface (nmcli).
     */
    class WifiManager::Impl
    {
    public:
        /**
         * @brief Constructor for the implementation class
         *
         * Initializes the WiFi state by checking if WiFi is currently enabled
         */
        Impl() : wifi_enabled_(check_wifi_enabled()), scan_thread_(nullptr), connect_thread_(nullptr)
        {
            // Initialize the dispatcher for thread-safe UI updates
            scan_dispatcher_.connect([this]()
                                     {
            // This runs in the main thread
            if (update_callback_) {
                update_callback_(networks_);
            } });

            // Initialize the dispatcher for connection results
            connect_dispatcher_.connect([this]()
                                        {
            // This runs in the main thread
            if (connect_callback_) {
                connect_callback_(connect_success_, connect_ssid_);
                // Clear the callback after it's been called
                connect_callback_ = nullptr;
            } });
        }
        ~Impl()
        {
            // Clean up any running scan thread
            stop_scan_thread();

            // Clean up any running connect thread
            stop_connect_thread();
        }

        /**
         * @brief Scan for available WiFi networks
         *
         * Uses nmcli to scan for available networks and populates the networks_ vector.
         * If WiFi is disabled, clears the networks list instead.
         * Calls the update callback when finished if one is registered.
         * This method blocks until the scan is complete.
         */
        void scan_networks()
        {
            if (!wifi_enabled_)
            {
                std::lock_guard<std::mutex> lock(networks_mutex_);
                networks_.clear();
                if (update_callback_)
                {
                    update_callback_(networks_);
                }
                return;
            }

            // Perform the actual scan
            perform_scan();

            // Call the update callback directly since we're in the main thread
            if (update_callback_)
            {
                std::lock_guard<std::mutex> lock(networks_mutex_);
                update_callback_(networks_);
            }
        }

        /**
         * @brief Scan for available WiFi networks asynchronously
         *
         * Starts a new thread to scan for networks without blocking the UI.
         * When the scan is complete, the update callback will be called
         * on the main thread using the dispatcher.
         */
        void scan_networks_async()
        {
            if (!wifi_enabled_)
            {
                std::lock_guard<std::mutex> lock(networks_mutex_);
                networks_.clear();
                if (update_callback_)
                {
                    update_callback_(networks_);
                }
                return;
            }

            // Stop any existing scan thread
            stop_scan_thread();

            // Start a new scan thread
            scan_thread_ = std::make_unique<std::thread>([this]()
                                                         {
            // Perform the scan in the background thread
            perform_scan();

            // Notify the main thread that the scan is complete
            scan_dispatcher_.emit(); });

            // Detach the thread so it can continue running after this function returns
            scan_thread_->detach();
        }

        /**
         * @brief Perform the actual network scan
         *
         * Common implementation used by both synchronous and asynchronous scanning.
         * Populates the networks_ vector with the scan results.
         */
        void perform_scan()
        {
            // Clear the networks list before scanning
            {
                std::lock_guard<std::mutex> lock(networks_mutex_);
                networks_.clear();
            }

            std::string cmd = "nmcli -t -f IN-USE,SSID,SIGNAL,SECURITY device wifi list";
            std::array<char, 4096> buffer;
            std::string result;

            FILE *pipe = popen(cmd.c_str(), "r");
            if (!pipe)
            {
                std::cerr << "Failed to run nmcli\n";
                return;
            }
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            {
                result += buffer.data();
            }
            pclose(pipe);

            // Parse the scan results
            std::vector<Network> new_networks;
            size_t pos = 0;
            while ((pos = result.find('\n')) != std::string::npos)
            {
                std::string line = result.substr(0, pos);
                result.erase(0, pos + 1);

                auto tokens = split(line, ':');
                if (tokens.size() >= 4)
                {
                    Network net;
                    net.connected = (tokens[0] == "*");
                    net.ssid = tokens[1];
                    net.bssid = ""; // We don't have BSSID in this command
                    try
                    {
                        // Parse the signal strength percentage from nmcli output
                        int signal = std::stoi(tokens[2]);

                        // Store the signal strength in the network object
                        net.signal_strength = signal;
                    }
                    catch (...)
                    {
                        net.signal_strength = 0;
                    }
                    net.secured = (tokens[3] != "--" && !tokens[3].empty());
                    new_networks.push_back(net);
                }
            }

            // Update the networks list with the scan results
            {
                std::lock_guard<std::mutex> lock(networks_mutex_);
                networks_ = std::move(new_networks);
            }
        }

        /**
         * @brief Disconnect from the current WiFi network
         *
         * Disconnects from the currently connected WiFi network using nmcli.
         * Rescans networks after disconnecting.
         */
        void disconnect()
        {
            // Get the WiFi interface to disconnect
            std::string wifi_interface = get_wifi_interface();
            if (wifi_interface.empty())
            {
                std::cerr << "Error: No WiFi interface found" << std::endl;
                return;
            }

            // Disconnect from the current WiFi network
            std::string cmd = "nmcli device disconnect " + wifi_interface;
            std::cout << "Disconnecting from WiFi..." << std::endl;
            std::system(cmd.c_str());

            // Update network list after disconnecting (asynchronously)
            scan_networks_async();
        }

        /**
         * @brief Remove saved credentials for a WiFi network
         * @param ssid The SSID of the network to forget
         *
         * Finds and deletes all connection profiles associated with the specified SSID.
         * Also cleans up any temporary connections that might have been created.
         * Rescans networks after forgetting.
         */
        void forget_network(const std::string &ssid)
        {
            std::cout << "Forgetting network: " << ssid << std::endl;

            // Get a list of all connection profiles from NetworkManager
            std::string cmd = "nmcli -t -f NAME,UUID,TYPE connection show";
            std::array<char, 4096> buffer;
            std::string result;

            FILE *pipe = popen(cmd.c_str(), "r");
            if (!pipe)
            {
                std::cerr << "Failed to run nmcli to find connections" << std::endl;
                return;
            }

            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            {
                result += buffer.data();
            }
            pclose(pipe);

            // Parse the output to find all WiFi connection profiles
            size_t pos = 0;
            std::vector<std::pair<std::string, std::string>> wifi_connections; // name, uuid pairs

            while ((pos = result.find('\n')) != std::string::npos)
            {
                std::string line = result.substr(0, pos);
                result.erase(0, pos + 1);

                auto tokens = split(line, ':');
                if (tokens.size() >= 3 && tokens[2] == "802-11-wireless")
                {
                    wifi_connections.push_back({tokens[0], tokens[1]});
                }
            }

            // Check each WiFi connection profile to see if it matches our target SSID
            bool deleted_any = false;

            for (const auto &conn : wifi_connections)
            {
                // Get the SSID associated with this connection profile
                std::string check_cmd = "nmcli -g 802-11-wireless.ssid connection show " + conn.second + " 2>/dev/null";
                FILE *check_pipe = popen(check_cmd.c_str(), "r");
                if (!check_pipe)
                    continue;

                std::string conn_ssid;
                if (fgets(buffer.data(), buffer.size(), check_pipe) != nullptr)
                {
                    conn_ssid = buffer.data();
                    // Trim whitespace from the SSID
                    conn_ssid.erase(0, conn_ssid.find_first_not_of(" \t\n\r"));
                    conn_ssid.erase(conn_ssid.find_last_not_of(" \t\n\r") + 1);
                }
                pclose(check_pipe);

                // If this connection profile matches our target SSID, delete it
                if (conn_ssid == ssid)
                {
                    std::string delete_cmd = "nmcli connection delete " + conn.second;
                    std::cout << "Deleting connection '" << conn.first << "' (UUID: " << conn.second << ") for SSID: " << ssid << std::endl;
                    std::system(delete_cmd.c_str());
                    deleted_any = true;
                }
            }

            // If no matching profiles were found, try deleting by connection name as a fallback
            if (!deleted_any)
            {
                std::string delete_cmd = "nmcli connection delete \"" + ssid + "\" 2>/dev/null || true";
                std::system(delete_cmd.c_str());
            }

            // Clean up any temporary connections that might have been created by nmcli
            std::string cleanup_cmd = "nmcli -t -f NAME connection show | grep \"temp-conn-\" | xargs -r -n1 nmcli connection delete 2>/dev/null || true";
            std::system(cleanup_cmd.c_str());

            std::cout << "Network forgotten: " << ssid << std::endl;
            scan_networks_async();
        }

        /**
         * @brief Enable the WiFi radio
         *
         * Turns on the WiFi radio using nmcli.
         * Updates the internal state and calls the state callback if registered.
         * Scans for networks asynchronously after enabling WiFi.
         */
        void enable_wifi()
        {
            std::string cmd = "nmcli radio wifi on";
            int ret = std::system(cmd.c_str());
            if (ret == 0)
            {
                wifi_enabled_ = true;
                if (state_callback_)
                {
                    state_callback_(wifi_enabled_);
                }
                // Update network list after enabling WiFi (asynchronously)
                scan_networks_async();
            }
        }

        /**
         * @brief Disable the WiFi radio
         *
         * Turns off the WiFi radio using nmcli.
         * Updates the internal state and calls the state callback if registered.
         * Clears the network list after disabling WiFi.
         */
        void disable_wifi()
        {
            std::string cmd = "nmcli radio wifi off";
            int ret = std::system(cmd.c_str());
            if (ret == 0)
            {
                wifi_enabled_ = false;
                if (state_callback_)
                {
                    state_callback_(wifi_enabled_);
                }
                // Clear network list since WiFi is now disabled
                networks_.clear();
                if (update_callback_)
                {
                    update_callback_(networks_);
                }
            }
        }

        /**
         * @brief Check if WiFi is currently enabled
         * @return true if WiFi is enabled, false otherwise
         */
        bool is_wifi_enabled() const
        {
            return wifi_enabled_;
        }

        /**
         * @brief Query the system to determine if WiFi is enabled
         * @return true if WiFi is enabled, false otherwise
         *
         * Uses nmcli to check the current state of the WiFi radio.
         */
        bool check_wifi_enabled()
        {
            std::string cmd = "nmcli radio wifi";
            std::array<char, 128> buffer;
            std::string result;

            FILE *pipe = popen(cmd.c_str(), "r");
            if (!pipe)
                return false;
            if (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            {
                result = buffer.data();
            }
            pclose(pipe);

            // Trim whitespace from the result
            result.erase(0, result.find_first_not_of(" \t\n\r"));
            result.erase(result.find_last_not_of(" \t\n\r") + 1);

            return result == "enabled";
        }

        /**
         * @brief Set the callback for network list updates
         * @param cb The callback function to be called when the network list changes
         */
        void set_update_callback(WifiManager::UpdateCallback cb)
        {
            update_callback_ = cb;
        }

        /**
         * @brief Set the callback for WiFi state changes
         * @param cb The callback function to be called when WiFi is enabled or disabled
         */
        void set_state_callback(WifiManager::StateCallback cb)
        {
            state_callback_ = cb;
        }

        /**
         * @brief Get the current list of WiFi networks
         * @return A reference to the vector of detected networks
         */
        const std::vector<Network> &get_networks() const
        {
            return networks_;
        }

        /**
         * @brief Connect to a WiFi network asynchronously
         * @param ssid The SSID of the network to connect to
         * @param password The password for the network (empty for open networks)
         * @param security_type The security type (e.g., "wpa-psk")
         * @param callback Optional callback function to be called when the connection attempt completes
         *
         * First tries to connect using saved credentials. If that fails,
         * creates a new connection profile with the provided credentials.
         * Rescans networks after attempting to connect.
         * This method returns immediately and the connection runs in a background thread.
         */
        void connect_async(const std::string &ssid, const std::string &password,
                           const std::string &security_type, ConnectionCallback callback);

    private:
        std::vector<Network> networks_;
        std::mutex networks_mutex_;
        WifiManager::UpdateCallback update_callback_;
        WifiManager::StateCallback state_callback_;
        bool wifi_enabled_;
        std::unique_ptr<std::thread> scan_thread_;
        Glib::Dispatcher scan_dispatcher_;

        /**
         * @brief Stop the current scan thread if one is running
         */
        void stop_scan_thread()
        {
            if (scan_thread_ && scan_thread_->joinable())
            {
                scan_thread_->join();
            }
            scan_thread_.reset();
        }

        /**
         * @brief Stop the connect thread if it's running
         *
         * Joins the connect thread if it's joinable and resets the pointer.
         */
        void stop_connect_thread()
        {
            if (connect_thread_ && connect_thread_->joinable())
            {
                connect_thread_->join();
            }
            connect_thread_.reset();
        }

        /**
         * @brief Get the name of the WiFi interface
         * @return The name of the WiFi interface (e.g., wlan0)
         *
         * Uses nmcli to find the WiFi interface, excluding p2p interfaces.
         */
        std::string get_wifi_interface()
        {
            std::string cmd = "nmcli device status | grep wifi | grep -v p2p | awk '{print $1}'";
            std::array<char, 128> buffer;
            std::string result;

            FILE *pipe = popen(cmd.c_str(), "r");
            if (!pipe)
                return "";
            if (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
            {
                result = buffer.data();
            }
            pclose(pipe);

            // Trim whitespace from the interface name
            result.erase(0, result.find_first_not_of(" \t\n\r"));
            result.erase(result.find_last_not_of(" \t\n\r") + 1);

            return result;
        }

        /**
         * @brief Split a string by a delimiter character
         * @param s The string to split
         * @param delimiter The character to split on
         * @return A vector of substrings
         */
        static std::vector<std::string> split(const std::string &s, char delimiter)
        {
            std::vector<std::string> tokens;
            std::string token;
            for (char c : s)
            {
                if (c == delimiter)
                {
                    tokens.push_back(token);
                    token.clear();
                }
                else
                {
                    token += c;
                }
            }
            tokens.push_back(token);
            return tokens;
        }

        // Additional member variables for connection
        std::unique_ptr<std::thread> connect_thread_;   ///< Thread for asynchronous connection
        Glib::Dispatcher connect_dispatcher_;           ///< Dispatcher for connection results
        ConnectionCallback connect_callback_ = nullptr; ///< Callback for connection results
        bool connect_success_ = false;                  ///< Whether the last connection attempt was successful
        std::string connect_ssid_;                      ///< SSID of the network being connected to
    };

    /**
     * @brief Constructor for WifiManager
     *
     * Creates the implementation object using the PIMPL idiom.
     */
    WifiManager::WifiManager() : impl_(std::make_unique<Impl>()) {}

    /**
     * @brief Destructor for WifiManager
     *
     * Default implementation is sufficient since impl_ is a unique_ptr.
     */
    WifiManager::~WifiManager() = default;

    /**
     * @brief Scan for available WiFi networks
     *
     * Delegates to the implementation class.
     * This method blocks until the scan is complete.
     */
    void WifiManager::scan_networks()
    {
        impl_->scan_networks();
    }

    /**
     * @brief Scan for available WiFi networks asynchronously
     *
     * Delegates to the implementation class.
     * This method returns immediately and the scan runs in a background thread.
     */
    void WifiManager::scan_networks_async()
    {
        impl_->scan_networks_async();
    }

    /**
     * @brief Connect to a WiFi network asynchronously
     * @param ssid The SSID of the network to connect to
     * @param password The password for the network (empty for open networks)
     * @param security_type The security type (defaults to "wpa-psk")
     * @param callback Optional callback function to be called when the connection attempt completes
     */
    void WifiManager::connect_async(const std::string &ssid, const std::string &password,
                                    const std::string &security_type, ConnectionCallback callback)
    {
        impl_->connect_async(ssid, password, security_type, callback);
    }

    /**
     * @brief Disconnect from the current WiFi network
     */
    void WifiManager::disconnect()
    {
        impl_->disconnect();
    }

    /**
     * @brief Remove saved credentials for a WiFi network
     * @param ssid The SSID of the network to forget
     */
    void WifiManager::forget_network(const std::string &ssid)
    {
        impl_->forget_network(ssid);
    }

    /**
     * @brief Enable the WiFi radio
     */
    void WifiManager::enable_wifi()
    {
        impl_->enable_wifi();
    }

    /**
     * @brief Disable the WiFi radio
     */
    void WifiManager::disable_wifi()
    {
        impl_->disable_wifi();
    }

    /**
     * @brief Check if WiFi is currently enabled
     * @return true if WiFi is enabled, false otherwise
     */
    bool WifiManager::is_wifi_enabled() const
    {
        return impl_->is_wifi_enabled();
    }

    /**
     * @brief Set the callback for network list updates
     * @param cb The callback function to be called when the network list changes
     */
    void WifiManager::set_update_callback(UpdateCallback cb)
    {
        impl_->set_update_callback(cb);
    }

    /**
     * @brief Set the callback for WiFi state changes
     * @param cb The callback function to be called when WiFi is enabled or disabled
     */
    void WifiManager::set_state_callback(StateCallback cb)
    {
        impl_->set_state_callback(cb);
    }

    /**
     * @brief Get the current list of WiFi networks
     * @return A reference to the vector of detected networks
     */
    const WifiManager::NetworkList &WifiManager::get_networks() const
    {
        return impl_->get_networks();
    }

    /**
     * @brief Implementation of connect_async for WifiManager::Impl
     */
    void WifiManager::Impl::connect_async(const std::string &ssid, const std::string &password,
                                          const std::string &security_type, WifiManager::ConnectionCallback callback)
    {
        // Store the callback for later use
        connect_callback_ = callback;
        connect_ssid_ = ssid;

        // Stop any existing connect thread
        stop_connect_thread();

        // Start a new connect thread
        connect_thread_ = std::make_unique<std::thread>([this, ssid, password, security_type]()
                                                        {
            // First check if we're already connected to this network to avoid unnecessary operations
            bool already_connected = false;
            for (const auto &net : networks_)
            {
                if (net.ssid == ssid && net.connected)
                {
                    already_connected = true;
                    break;
                }
            }

            if (already_connected)
            {
                std::cout << "Already connected to " << ssid << std::endl;
                connect_success_ = true;
                connect_dispatcher_.emit();
                return;
            }

            std::cout << "Connecting to WiFi network: " << ssid << "..." << std::endl;

            // Try to connect using an existing saved connection profile first
            std::string saved_cmd = "nmcli con up \"" + ssid + "\" 2>/dev/null";
            int saved_result = std::system(saved_cmd.c_str());

            if (saved_result == 0)
            {
                std::cout << "Successfully connected to saved network: " << ssid << std::endl;
                connect_success_ = true;
                connect_dispatcher_.emit();
                scan_networks_async();
                return;
            }

            // If no saved connection exists or connection failed, create a new connection profile
            bool success = false;

            // For secured networks, create a detailed connection profile with security settings
            if (!password.empty() && !security_type.empty())
            {
                // Get the WiFi interface name (e.g., wlan0, wlp3s0)
                std::string wifi_interface = get_wifi_interface();
                if (wifi_interface.empty())
                {
                    std::cerr << "Error: No WiFi interface found" << std::endl;
                    connect_success_ = false;
                    connect_dispatcher_.emit();
                    return;
                }

                // Use the SSID as the connection profile name
                std::string conn_name = ssid;

                // Delete any existing connection with the same name to avoid conflicts
                std::string delete_cmd = "nmcli con delete \"" + conn_name + "\" 2>/dev/null || true";
                std::system(delete_cmd.c_str());

                // Create a new connection profile with the correct security settings
                std::string create_cmd = "nmcli con add type wifi con-name \"" + conn_name + "\" ifname " + wifi_interface +
                                         " ssid \"" + ssid + "\" && " +
                                         "nmcli con modify \"" + conn_name + "\" wifi-sec.key-mgmt " + security_type + " && " +
                                         "nmcli con modify \"" + conn_name + "\" wifi-sec.psk \"" + password + "\" && " +
                                         "nmcli con up \"" + conn_name + "\"";

                int result = std::system(create_cmd.c_str());

                if (result == 0)
                {
                    std::cout << "Successfully connected to " << ssid << std::endl;
                    success = true;
                }
                else
                {
                    std::cerr << "Failed to connect to " << ssid << std::endl;
                }
            }
            else
            {
                // For open networks or when security type isn't specified, use the simpler connection method
                std::string cmd = "nmcli dev wifi connect \"" + ssid + "\"";
                if (!password.empty())
                {
                    cmd += " password \"" + password + "\"";
                }

                int result = std::system(cmd.c_str());

                if (result == 0)
                {
                    std::cout << "Successfully connected to " << ssid << std::endl;
                    success = true;
                }
                else
                {
                    std::cerr << "Failed to connect to " << ssid << std::endl;
                }
            }

            connect_success_ = success;
            connect_dispatcher_.emit();
            scan_networks_async(); });

        // Detach the thread so it can continue running after this function returns
        connect_thread_->detach();
    }

    std::string WifiManager::get_password(const std::string &ssid)
    {
        std::string command = "nmcli -s -g 802-11-wireless-security.psk connection show \"" + ssid + "\"";

        FILE *fp = popen(command.c_str(), "r");
        if (!fp)
        {
            std::cerr << "Failed to run command: " << command << std::endl;
            return "";
        }

        char result[1024];
        std::string password = "";
        while (fgets(result, sizeof(result), fp) != NULL)
        {
            password += result;
        }

        fclose(fp);

        if (!password.empty() && password.back() == '\n')
        {
            password.pop_back();
        }

        return password;
    }

    std::string WifiManager::generate_qr_code(const std::string &ssid, const std::string &password, const std::string &security)
    {
        // tmp dir for the image
        std::filesystem::path temp_dir = "/tmp/ultimate-control";
        std::filesystem::create_directories(temp_dir);

        std::filesystem::path qr_code_path = temp_dir / (ssid + ".png");

        try
        {
            if (std::filesystem::exists(qr_code_path))
            {
                std::cout << "Found QR code for " << ssid << " at " << qr_code_path << std::endl;
                return qr_code_path.string();
            }

            std::string security_type = (security == "none" || security == "None") ? "nopass" : "WPA";
            std::string wifi_string = "WIFI:T:" + security_type + ";S:" + ssid + ";P:" + password + ";;";

            Utils::QRCode qr;
            qr.encode(wifi_string);

            int module_count = qr.getSize();
            int scale = 7;
            int image_size = module_count * scale;

            // Create a Pixbuf for saving the image
            GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, image_size, image_size);
            guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
            int rowstride = gdk_pixbuf_get_rowstride(pixbuf);

            // Fill the image with qr code modules
            for (int y = 0; y < module_count; ++y)
            {
                for (int x = 0; x < module_count; ++x)
                {
                    bool dark = qr.getModule(x, y);
                    guchar color = dark ? 0 : 255;
                    for (int dy = 0; dy < scale; ++dy)
                    {
                        for (int dx = 0; dx < scale; ++dx)
                        {
                            int px = x * scale + dx;
                            int py = y * scale + dy;
                            guchar *p = pixels + py * rowstride + px * 3;
                            p[0] = p[1] = p[2] = color;
                        }
                    }
                }
            }

            // Save the image to tmp dir
            gdk_pixbuf_save(pixbuf, qr_code_path.c_str(), "png", nullptr, nullptr);
            g_object_unref(pixbuf);

            std::cout << "Generated QR code for " << ssid << " at " << qr_code_path << std::endl;
            return qr_code_path.string();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to generate QR code for " << ssid << ": " << e.what() << std::endl;

            // Return error image path
            return std::filesystem::absolute("src/css/error.png").string();
        }
    }

} // namespace Wifi
