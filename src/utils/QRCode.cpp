#include "QRCode.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace Utils {

QRCode::QRCode(Version version, ErrorCorrection ecLevel)
    : version_(version), ecLevel_(ecLevel) {
    initialize();
}

void QRCode::initialize() {
    // Nothing to initialize until we encode data
    initialized_ = false;
}

bool QRCode::encode(const std::string& data) {
    try {
        // Convert our error correction level to the library's format
        qrcodegen::QrCode::Ecc ecLevel;
        switch (ecLevel_) {
            case ErrorCorrection::L: ecLevel = qrcodegen::QrCode::Ecc::LOW; break;
            case ErrorCorrection::M: ecLevel = qrcodegen::QrCode::Ecc::MEDIUM; break;
            case ErrorCorrection::Q: ecLevel = qrcodegen::QrCode::Ecc::QUARTILE; break;
            case ErrorCorrection::H: ecLevel = qrcodegen::QrCode::Ecc::HIGH; break;
            default: ecLevel = qrcodegen::QrCode::Ecc::MEDIUM;
        }

        // Create the QR code
        qrCode_ = std::make_unique<qrcodegen::QrCode>(qrcodegen::QrCode::encodeText(data.c_str(), ecLevel));
        initialized_ = true;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error encoding QR code: " << e.what() << std::endl;
        return false;
    }
}

int QRCode::getSize() const {
    if (!initialized_) {
        return 0;
    }
    return qrCode_->getSize();
}

bool QRCode::getModule(int x, int y) const {
    if (!initialized_ || x < 0 || y < 0 || x >= getSize() || y >= getSize()) {
        return false;
    }
    return qrCode_->getModule(x, y);
}

void QRCode::draw(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double size) const {
    if (!initialized_) {
        return;
    }

    int qrSize = getSize();
    double moduleSize = size / qrSize;

    // Save the current state
    cr->save();

    // Move to the starting position
    cr->translate(x, y);

    // Draw each module
    for (int row = 0; row < qrSize; row++) {
        for (int col = 0; col < qrSize; col++) {
            if (getModule(col, row)) {
                cr->rectangle(col * moduleSize, row * moduleSize, moduleSize, moduleSize);
                cr->fill();
            }
        }
    }

    // Restore the state
    cr->restore();
}

Glib::RefPtr<Gdk::Pixbuf> QRCode::toPixbuf(int size, const Gdk::RGBA& foreground,
                                          const Gdk::RGBA& background) const {
    if (!initialized_) {
        return Glib::RefPtr<Gdk::Pixbuf>();
    }

    int qrSize = getSize();
    int moduleSize = size / qrSize;
    int actualSize = moduleSize * qrSize;

    // Create a surface to draw on
    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, actualSize, actualSize);
    auto cr = Cairo::Context::create(surface);

    // Fill the background
    cr->set_source_rgba(background.get_red(), background.get_green(),
                       background.get_blue(), background.get_alpha());
    cr->paint();

    // Set the foreground color
    cr->set_source_rgba(foreground.get_red(), foreground.get_green(),
                       foreground.get_blue(), foreground.get_alpha());

    // Draw the QR code
    for (int row = 0; row < qrSize; row++) {
        for (int col = 0; col < qrSize; col++) {
            if (getModule(col, row)) {
                cr->rectangle(col * moduleSize, row * moduleSize, moduleSize, moduleSize);
                cr->fill();
            }
        }
    }

    // Convert the surface to a pixbuf
    return Gdk::Pixbuf::create(surface, 0, 0, actualSize, actualSize);
}

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
