#ifndef ULTIMATE_CONTROL_VOLUME_TAB_HPP
#define ULTIMATE_CONTROL_VOLUME_TAB_HPP

#include <gtkmm.h>
#include "VolumeManager.hpp"
#include "VolumeWidget.hpp"
#include <memory>
#include <vector>

namespace Volume {

class VolumeTab : public Gtk::ScrolledWindow {
public:
    VolumeTab();
    virtual ~VolumeTab();

private:
    void update_sink_list(const std::vector<AudioSink>& sinks);

    std::shared_ptr<VolumeManager> manager_;

    Gtk::Notebook notebook_;
    Gtk::Box output_box_;
    Gtk::Box input_box_;

    std::vector<std::unique_ptr<VolumeWidget>> output_widgets_;
    std::vector<std::unique_ptr<VolumeWidget>> input_widgets_;
};

} // namespace Volume

#endif // ULTIMATE_CONTROL_VOLUME_TAB_HPP
