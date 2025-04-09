#include "VolumeWidget.hpp"

namespace Volume {

VolumeWidget::VolumeWidget(const AudioSink& sink, std::shared_ptr<VolumeManager> manager)
: Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10),
  manager_(std::move(manager)),
  sink_name_(sink.name),
  label_(sink.description),
  volume_scale_(Gtk::ORIENTATION_HORIZONTAL),
  mute_button_("Mute")
{
    set_margin_start(5);
    set_margin_end(5);
    set_margin_top(5);
    set_margin_bottom(5);

    volume_scale_.set_range(0, 100);
    volume_scale_.set_value(sink.volume);
    volume_scale_.set_size_request(150, -1);
    volume_scale_.set_draw_value(true);

    mute_button_.set_active(sink.muted);

    // Disable controls for input devices (sources)
    if (sink.name.find("input") != std::string::npos || sink.name.find("source") != std::string::npos) {
        volume_scale_.set_sensitive(true);
        mute_button_.set_sensitive(true);
    }

    pack_start(label_, Gtk::PACK_SHRINK);
    pack_start(volume_scale_, Gtk::PACK_EXPAND_WIDGET);
    pack_start(mute_button_, Gtk::PACK_SHRINK);

    volume_scale_.signal_value_changed().connect([this]() {
        int vol = static_cast<int>(volume_scale_.get_value());
        manager_->set_volume(sink_name_, vol);
    });
    mute_button_.signal_toggled().connect([this]() {
        manager_->toggle_mute(sink_name_);
    });

    show_all_children();
}

void VolumeWidget::on_volume_changed() {
    int vol = static_cast<int>(volume_scale_.get_value());
    manager_->set_volume(sink_name_, vol);
}

void VolumeWidget::on_mute_toggled() {
    manager_->toggle_mute(sink_name_);
}

VolumeWidget::~VolumeWidget() = default;


} // namespace Volume
