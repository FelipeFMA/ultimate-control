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

    std::shared_ptr<VolumeManager> manager_;
    std::string sink_name_;

    Gtk::Label label_;
    Gtk::Scale volume_scale_;
    Gtk::ToggleButton mute_button_;
};

} // namespace Volume

#endif // ULTIMATE_CONTROL_VOLUME_WIDGET_HPP
