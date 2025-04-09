#include "VolumeTab.hpp"

namespace Volume {

VolumeTab::VolumeTab()
: manager_(std::make_shared<VolumeManager>()),
  output_box_(Gtk::ORIENTATION_VERTICAL, 5),
  input_box_(Gtk::ORIENTATION_VERTICAL, 5)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(notebook_);

    notebook_.append_page(output_box_, "Output Devices");
    notebook_.append_page(input_box_, "Input Devices");

    manager_->set_update_callback([this](const std::vector<AudioSink>& sinks) {
        update_sink_list(sinks);
    });

    manager_->refresh_sinks();

    show_all_children();
}

VolumeTab::~VolumeTab() = default;

void VolumeTab::update_sink_list(const std::vector<AudioSink>& sinks) {
    for (auto& widget : output_widgets_) {
        output_box_.remove(*widget);
    }
    output_widgets_.clear();

    for (auto& widget : input_widgets_) {
        input_box_.remove(*widget);
    }
    input_widgets_.clear();

    for (const auto& sink : sinks) {
        if (sink.description.find("Monitor of") != std::string::npos || sink.name.find("Monitor of") != std::string::npos) {
            continue; // Skip monitor devices
        }
        auto widget = std::make_unique<VolumeWidget>(sink, manager_);
        if (sink.name.find("input") != std::string::npos || sink.name.find("source") != std::string::npos) {
            input_box_.pack_start(*widget, Gtk::PACK_SHRINK);
            input_widgets_.push_back(std::move(widget));
        } else {
            output_box_.pack_start(*widget, Gtk::PACK_SHRINK);
            output_widgets_.push_back(std::move(widget));
        }
    }

    show_all_children();
}

} // namespace Volume
