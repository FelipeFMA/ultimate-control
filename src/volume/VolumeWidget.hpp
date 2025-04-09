#ifndef ULTIMATE_CONTROL_VOLUME_WIDGET_HPP
#define ULTIMATE_CONTROL_VOLUME_WIDGET_HPP

#include <gtkmm.h>
#include "VolumeManager.hpp"
#include <memory>

namespace Volume {

class VolumeWidget : public Gtk::Box {
public:
    VolumeWidget(const AudioSink& sink, std::shared_ptr<VolumeManager> manager);
    virtual ~VolumeWidget();

private:
    void on_volume_changed();
    void on_mute_toggled();
    void update_volume_icon(int volume, bool muted);

    std::shared_ptr<VolumeManager> manager_;
    std::string sink_name_;
    bool is_input_device_;

    Gtk::Box device_box_;
    Gtk::Box control_box_;
    Gtk::Image device_icon_;
    Gtk::Label label_;
    Gtk::Scale volume_scale_;
    Gtk::Image volume_icon_;
    Gtk::ToggleButton mute_button_;
};

} // namespace Volume

#endif // ULTIMATE_CONTROL_VOLUME_WIDGET_HPP
