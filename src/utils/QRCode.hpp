#ifndef ULTIMATE_CONTROL_QRCODE_HPP
#define ULTIMATE_CONTROL_QRCODE_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <gtkmm.h>
#include "qrcodegen/qrcodegen.hpp"

namespace Utils {

// A simple QR code generator class
class QRCode {
public:
    // QR code version (1-5, corresponding to 21x21 through 37x37 modules)
    enum class Version {
        V1 = 1,  // 21x21
        V2 = 2,  // 25x25
        V3 = 3,  // 29x29
        V4 = 4,  // 33x33
        V5 = 5   // 37x37
    };

    // Error correction level
    enum class ErrorCorrection {
        L = 0,  // Low (7%)
        M = 1,  // Medium (15%)
        Q = 2,  // Quartile (25%)
        H = 3   // High (30%)
    };

    // Constructor
    QRCode(Version version = Version::V3, ErrorCorrection ecLevel = ErrorCorrection::M);

    // Encode data into QR code
    bool encode(const std::string& data);

    // Get the size of the QR code (number of modules per side)
    int getSize() const;

    // Get the module at a specific position (true = black, false = white)
    bool getModule(int x, int y) const;

    // Draw the QR code to a Cairo context
    void draw(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double size) const;

    // Create a Gdk::Pixbuf from the QR code
    Glib::RefPtr<Gdk::Pixbuf> toPixbuf(int size, const Gdk::RGBA& foreground = Gdk::RGBA("black"),
                                       const Gdk::RGBA& background = Gdk::RGBA("white")) const;

    // Format WiFi network information for QR code
    static std::string formatWifiNetwork(const std::string& ssid, const std::string& password,
                                        bool isHidden = false, const std::string& authType = "WPA");

private:
    Version version_;
    ErrorCorrection ecLevel_;
    std::unique_ptr<qrcodegen::QrCode> qrCode_;
    bool initialized_ = false;

    // Initialize the QR code matrix
    void initialize();

    // Utility functions
    static std::string escapeString(const std::string& input);
};

} // namespace Utils

#endif // ULTIMATE_CONTROL_QRCODE_HPP
