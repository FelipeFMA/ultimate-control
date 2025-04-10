/**
 * @file QRCode.cpp
 * @brief Implementation of the QR code generation functionality
 *
 * This file implements the QRCode class which provides an interface for
 * generating QR codes, drawing them to Cairo contexts, and creating
 * special formats like WiFi network information QR codes.
 */

#include "QRCode.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace Utils {

/**
 * @brief Constructor for the QR code generator
 * @param version QR code version (size)
 * @param ecLevel Error correction level
 */
QRCode::QRCode(Version version, ErrorCorrection ecLevel)
    : version_(version), ecLevel_(ecLevel) {
    initialize();
}

/**
 * @brief Initialize the QR code object
 *
 * Prepares the QR code object for encoding. Called by the constructor.
 */
void QRCode::initialize() {
    // Nothing to initialize until we encode data
    initialized_ = false;
}

/**
 * @brief Encode data into a QR code
 * @param data The string data to encode
 * @return true if encoding was successful, false otherwise
 */
bool QRCode::encode(const std::string& data) {
    try {
        // Convert our enum to the library's error correction level format
        qrcodegen::QrCode::Ecc ecLevel;
        switch (ecLevel_) {
            case ErrorCorrection::L: ecLevel = qrcodegen::QrCode::Ecc::LOW; break;
            case ErrorCorrection::M: ecLevel = qrcodegen::QrCode::Ecc::MEDIUM; break;
            case ErrorCorrection::Q: ecLevel = qrcodegen::QrCode::Ecc::QUARTILE; break;
            case ErrorCorrection::H: ecLevel = qrcodegen::QrCode::Ecc::HIGH; break;
            default: ecLevel = qrcodegen::QrCode::Ecc::MEDIUM;
        }

        // Create the QR code using the underlying library
        qrCode_ = std::make_unique<qrcodegen::QrCode>(qrcodegen::QrCode::encodeText(data.c_str(), ecLevel));
        initialized_ = true;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error encoding QR code: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Get the size of the QR code
 * @return Number of modules per side (e.g., 29 for version 3)
 */
int QRCode::getSize() const {
    if (!initialized_) {
        return 0;
    }
    return qrCode_->getSize();
}

/**
 * @brief Get the state of a specific module
 * @param x X-coordinate of the module (0-based)
 * @param y Y-coordinate of the module (0-based)
 * @return true if the module is black, false if white or out of bounds
 */
bool QRCode::getModule(int x, int y) const {
    if (!initialized_ || x < 0 || y < 0 || x >= getSize() || y >= getSize()) {
        return false;
    }
    return qrCode_->getModule(x, y);
}

/**
 * @brief Draw the QR code to a Cairo context
 * @param cr Cairo context to draw on
 * @param x X-coordinate of the top-left corner
 * @param y Y-coordinate of the top-left corner
 * @param size Size of the QR code in pixels
 */
void QRCode::draw(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double size) const {
    if (!initialized_) {
        return;
    }

    int qrSize = getSize();
    double moduleSize = size / qrSize;

    // Save the current state of the Cairo context
    cr->save();

    // Move to the starting position for drawing
    cr->translate(x, y);

    // Draw each module (black squares for true values)
    for (int row = 0; row < qrSize; row++) {
        for (int col = 0; col < qrSize; col++) {
            if (getModule(col, row)) {
                cr->rectangle(col * moduleSize, row * moduleSize, moduleSize, moduleSize);
                cr->fill();
            }
        }
    }

    // Restore the original state of the Cairo context
    cr->restore();
}

/**
 * @brief Create a Pixbuf from the QR code
 * @param size Size of the QR code in pixels
 * @param foreground Color for the QR code modules (default: black)
 * @param background Color for the background (default: white)
 * @return A Glib::RefPtr to a Gdk::Pixbuf containing the rendered QR code
 */
Glib::RefPtr<Gdk::Pixbuf> QRCode::toPixbuf(int size, const Gdk::RGBA& foreground,
                                          const Gdk::RGBA& background) const {
    if (!initialized_) {
        return Glib::RefPtr<Gdk::Pixbuf>();
    }

    int qrSize = getSize();
    int moduleSize = size / qrSize;
    int actualSize = moduleSize * qrSize;

    // Create a Cairo surface to draw the QR code on
    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, actualSize, actualSize);
    auto cr = Cairo::Context::create(surface);

    // Fill the background with the specified color
    cr->set_source_rgba(background.get_red(), background.get_green(),
                       background.get_blue(), background.get_alpha());
    cr->paint();

    // Set the foreground color for the QR code modules
    cr->set_source_rgba(foreground.get_red(), foreground.get_green(),
                       foreground.get_blue(), foreground.get_alpha());

    // Draw each module of the QR code as a rectangle
    for (int row = 0; row < qrSize; row++) {
        for (int col = 0; col < qrSize; col++) {
            if (getModule(col, row)) {
                cr->rectangle(col * moduleSize, row * moduleSize, moduleSize, moduleSize);
                cr->fill();
            }
        }
    }

    // Convert the Cairo surface to a Gdk::Pixbuf for use with GTK widgets
    return Gdk::Pixbuf::create(surface, 0, 0, actualSize, actualSize);
}

/**
 * @brief Format WiFi network information for QR code
 * @param ssid Network name
 * @param password Network password (empty for open networks)
 * @param isHidden Whether the network is hidden (not broadcasting SSID)
 * @param authType Authentication type (WPA, WEP, etc.)
 * @return A formatted string ready for QR code encoding
 */
std::string QRCode::formatWifiNetwork(const std::string& ssid, const std::string& password,
                                     bool isHidden, const std::string& authType) {
    std::stringstream ss;
    ss << "WIFI:";
    ss << "T:" << escapeString(authType) << ";";
    ss << "S:" << escapeString(ssid) << ";";

    if (!password.empty()) {
        ss << "P:" << escapeString(password) << ";";
    }

    if (isHidden) {
        ss << "H:true;";
    }

    ss << ";";
    return ss.str();
}

/**
 * @brief Escape special characters in a string
 * @param input The string to escape
 * @return The escaped string
 */
std::string QRCode::escapeString(const std::string& input) {
    std::stringstream ss;
    for (char c : input) {
        if (c == '\\' || c == ';' || c == ',' || c == ':' || c == '"') {
            ss << '\\' << c;
        } else {
            ss << c;
        }
    }
    return ss.str();
}

} // namespace Utils
