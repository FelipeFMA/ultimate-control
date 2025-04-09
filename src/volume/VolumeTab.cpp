#include "VolumeTab.hpp"

namespace Volume {

VolumeTab::VolumeTab()
: manager_(std::make_shared<VolumeManager>()),
  output_box_(Gtk::ORIENTATION_VERTICAL, 10),
  input_box_(Gtk::ORIENTATION_VERTICAL, 10)
{
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(notebook_);

    // Style the notebook
    notebook_.set_scrollable(true);

    // Create output devices tab with icon
    auto output_tab_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));
    auto output_icon = Gtk::manage(new Gtk::Image());
    output_icon->set_from_icon_name("audio-speakers-symbolic", Gtk::ICON_SIZE_MENU);
    auto output_label = Gtk::manage(new Gtk::Label("Output Devices"));
    output_tab_box->pack_start(*output_icon, Gtk::PACK_SHRINK);
    output_tab_box->pack_start(*output_label, Gtk::PACK_SHRINK);
    output_tab_box->show_all();

    // Create input devices tab with icon
    auto input_tab_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));
    auto input_icon = Gtk::manage(new Gtk::Image());
    input_icon->set_from_icon_name("audio-input-microphone-symbolic", Gtk::ICON_SIZE_MENU);
    auto input_label = Gtk::manage(new Gtk::Label("Input Devices"));
    input_tab_box->pack_start(*input_icon, Gtk::PACK_SHRINK);
    input_tab_box->pack_start(*input_label, Gtk::PACK_SHRINK);
    input_tab_box->show_all();

    // Add styled tabs to notebook
    notebook_.append_page(output_box_, *output_tab_box);
    notebook_.append_page(input_box_, *input_tab_box);

    // Add padding to the boxes
    output_box_.set_margin_start(10);
    output_box_.set_margin_end(10);
    output_box_.set_margin_top(10);
    output_box_.set_margin_bottom(10);

    input_box_.set_margin_start(10);
    input_box_.set_margin_end(10);
    input_box_.set_margin_top(10);
    input_box_.set_margin_bottom(10);

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
