/**
 * @file QRCode.hpp
 * @brief QR code generation functionality for Ultimate Control
 *
 * This file defines the QRCode class which provides an interface for
 * generating QR codes, drawing them to Cairo contexts, and creating
 * special formats like WiFi network information QR codes.
 */

#ifndef ULTIMATE_CONTROL_QRCODE_HPP
#define ULTIMATE_CONTROL_QRCODE_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <gtkmm.h>
#include "qrcodegen/qrcodegen.hpp"

/**
 * @namespace Utils
 * @brief Contains utility functions and classes
 */
namespace Utils {

/**
 * @class QRCode
 * @brief A QR code generator and renderer
 *
 * Provides functionality for generating QR codes from text or structured data,
 * rendering them to Cairo contexts, and creating Pixbuf objects for display.
 * Also includes special formatting for WiFi network information.
 */
class QRCode {
public:
    /**
     * @enum Version
     * @brief QR code version determining size and capacity
     *
     * QR code versions from 1 to 5, corresponding to sizes from 21x21 to 37x37 modules.
     * Higher versions can store more data but produce larger codes.
     */
    enum class Version {
        V1 = 1,  // 21x21
        V2 = 2,  // 25x25
        V3 = 3,  // 29x29
        V4 = 4,  // 33x33
        V5 = 5   // 37x37
    };

    /**
     * @enum ErrorCorrection
     * @brief Error correction level for QR codes
     *
     * Determines the amount of redundancy in the QR code for error correction.
     * Higher levels allow the code to be read even if partially damaged or obscured,
     * but reduce the amount of data that can be stored.
     */
    enum class ErrorCorrection {
        L = 0,  // Low (7%)
        M = 1,  // Medium (15%)
        Q = 2,  // Quartile (25%)
        H = 3   // High (30%)
    };

    /**
     * @brief Constructor
     * @param version QR code version (size)
     * @param ecLevel Error correction level
     *
     * Creates a QR code generator with the specified version and error correction level.
     * Default is version 3 (29x29) with medium error correction (15%).
     */
    QRCode(Version version = Version::V3, ErrorCorrection ecLevel = ErrorCorrection::M);

    /**
     * @brief Encode data into a QR code
     * @param data The string data to encode
     * @return true if encoding was successful, false otherwise
     *
     * Encodes the provided string data into a QR code using the configured
     * version and error correction level.
     */
    bool encode(const std::string& data);

    /**
     * @brief Get the size of the QR code
     * @return Number of modules per side (e.g., 29 for version 3)
     *
     * Returns the dimension of the QR code in modules. A module is the
     * smallest square element in a QR code (a black or white dot).
     */
    int getSize() const;

    /**
     * @brief Get the state of a specific module
     * @param x X-coordinate of the module (0-based)
     * @param y Y-coordinate of the module (0-based)
     * @return true if the module is black, false if white or out of bounds
     *
     * Returns the state of the module at the specified coordinates.
     * Coordinates are 0-based and start from the top-left corner.
     */
    bool getModule(int x, int y) const;

    /**
     * @brief Draw the QR code to a Cairo context
     * @param cr Cairo context to draw on
     * @param x X-coordinate of the top-left corner
     * @param y Y-coordinate of the top-left corner
     * @param size Size of the QR code in pixels
     *
     * Draws the QR code to the specified Cairo context at the given position and size.
     * The current source color of the context is used for the modules.
     */
    void draw(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double size) const;

    /**
     * @brief Create a Pixbuf from the QR code
     * @param size Size of the QR code in pixels
     * @param foreground Color for the QR code modules (default: black)
     * @param background Color for the background (default: white)
     * @return A Glib::RefPtr to a Gdk::Pixbuf containing the rendered QR code
     *
     * Creates a Gdk::Pixbuf containing the rendered QR code with the specified
     * foreground and background colors. The Pixbuf can be used with GTK+ widgets.
     */
    Glib::RefPtr<Gdk::Pixbuf> toPixbuf(int size, const Gdk::RGBA& foreground = Gdk::RGBA("black"),
                                       const Gdk::RGBA& background = Gdk::RGBA("white")) const;

    /**
     * @brief Format WiFi network information for QR code
     * @param ssid Network name
     * @param password Network password (empty for open networks)
     * @param isHidden Whether the network is hidden (not broadcasting SSID)
     * @param authType Authentication type (WPA, WEP, etc.)
     * @return A formatted string ready for QR code encoding
     *
     * Creates a properly formatted string for WiFi network information
     * according to the standard format used by QR code scanner apps.
     * When scanned, this allows phones to automatically connect to the network.
     */
    static std::string formatWifiNetwork(const std::string& ssid, const std::string& password,
                                        bool isHidden = false, const std::string& authType = "WPA");

private:
    Version version_;                              ///< QR code version (size)
    ErrorCorrection ecLevel_;                      ///< Error correction level
    std::unique_ptr<qrcodegen::QrCode> qrCode_;    ///< Underlying QR code implementation
    bool initialized_ = false;                     ///< Whether a QR code has been generated

    /**
     * @brief Initialize the QR code object
     *
     * Prepares the QR code object for encoding. Called by the constructor.
     */
    void initialize();

    /**
     * @brief Escape special characters in a string
     * @param input The string to escape
     * @return The escaped string
     *
     * Escapes special characters in the input string according to
     * the WiFi QR code format specification.
     */
    static std::string escapeString(const std::string& input);
};

} // namespace Utils

#endif // ULTIMATE_CONTROL_QRCODE_HPP
