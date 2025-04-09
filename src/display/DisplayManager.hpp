#ifndef ULTIMATE_CONTROL_DISPLAY_MANAGER_HPP
#define ULTIMATE_CONTROL_DISPLAY_MANAGER_HPP

#include <functional>

namespace Display {

class DisplayManager {
public:
    using BrightnessCallback = std::function<void(int)>; // 0-100

    DisplayManager();
    ~DisplayManager();

    int get_brightness() const;
    void set_brightness(int value);
    void set_update_callback(BrightnessCallback cb);

private:
    void notify();

    int brightness_;
    BrightnessCallback callback_;
};

} // namespace Display

#endif // ULTIMATE_CONTROL_DISPLAY_MANAGER_HPP
