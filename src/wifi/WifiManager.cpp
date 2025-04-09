#include "WifiManager.hpp"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <algorithm>

namespace Wifi {

class WifiManager::Impl {
public:
    Impl() = default;
    ~Impl() = default;

    void scan_networks() {
        networks_.clear();

        std::string cmd = "nmcli -t -f active,ssid,bssid,signal,security dev wifi";
        std::array<char, 4096> buffer;
        std::string result;

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run nmcli\n";
            return;
        }
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);

        size_t pos = 0;
        while ((pos = result.find('\n')) != std::string::npos) {
            std::string line = result.substr(0, pos);
            result.erase(0, pos + 1);

            auto tokens = split(line, ':');
            if (tokens.size() >= 5) {
                Network net;
                net.connected = (tokens[0] == "yes");
                net.ssid = tokens[1];
                net.bssid = tokens[2];
                try {
                    net.signal_strength = std::stoi(tokens[3]);
                } catch (...) {
                    net.signal_strength = 0;
                }
                net.secured = (tokens[4] != "--");
                networks_.push_back(net);
            }
        }

        if (update_callback_) {
            update_callback_(networks_);
        }
    }

    void connect(const std::string& ssid, const std::string& password) {
        std::string cmd = "nmcli dev wifi connect \"" + ssid + "\" password \"" + password + "\"";
        std::system(cmd.c_str());
        scan_networks();
    }

    void disconnect() {
        std::string cmd = "nmcli networking off && nmcli networking on";
        std::system(cmd.c_str());
        scan_networks();
    }

    void set_update_callback(WifiManager::UpdateCallback cb) {
        update_callback_ = cb;
    }

private:
    std::vector<Network> networks_;
    WifiManager::UpdateCallback update_callback_;

    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        for (char c : s) {
            if (c == delimiter) {
                tokens.push_back(token);
                token.clear();
            } else {
                token += c;
            }
        }
        tokens.push_back(token);
        return tokens;
    }
};

WifiManager::WifiManager() : impl_(std::make_unique<Impl>()) {}

WifiManager::~WifiManager() = default;

void WifiManager::scan_networks() {
    impl_->scan_networks();
}

void WifiManager::connect(const std::string& ssid, const std::string& password) {
    impl_->connect(ssid, password);
}

void WifiManager::disconnect() {
    impl_->disconnect();
}

void WifiManager::set_update_callback(UpdateCallback cb) {
    impl_->set_update_callback(cb);
}

} // namespace Wifi
