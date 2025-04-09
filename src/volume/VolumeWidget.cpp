#include "VolumeWidget.hpp"

namespace Volume {

VolumeWidget::VolumeWidget(const AudioSink& sink, std::shared_ptr<VolumeManager> manager)
: Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5),
  manager_(std::move(manager)),
  sink_name_(sink.name),
  is_input_device_(sink.name.find("input") != std::string::npos || sink.name.find("source") != std::string::npos),
  device_box_(Gtk::ORIENTATION_HORIZONTAL, 5),
  control_box_(Gtk::ORIENTATION_HORIZONTAL, 10),
  label_(sink.description),
  volume_scale_(Gtk::ORIENTATION_HORIZONTAL),
  mute_button_()
{
    // Set up the main container
    set_margin_start(10);
    set_margin_end(10);
    set_margin_top(8);
    set_margin_bottom(8);

    // Add a frame around the widget for better visual separation
    Gtk::Frame* frame = Gtk::manage(new Gtk::Frame());
    frame->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    pack_start(*frame, Gtk::PACK_EXPAND_WIDGET);

    // Create a box inside the frame
    Gtk::Box* inner_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5));
    inner_box->set_margin_start(10);
    inner_box->set_margin_end(10);
    inner_box->set_margin_top(10);
    inner_box->set_margin_bottom(10);
    frame->add(*inner_box);

    // Set up the device icon based on device type
    if (is_input_device_) {
        device_icon_.set_from_icon_name("audio-input-microphone-symbolic", Gtk::ICON_SIZE_LARGE_TOOLBAR);
    } else {
        device_icon_.set_from_icon_name("audio-speakers-symbolic", Gtk::ICON_SIZE_LARGE_TOOLBAR);
    }

    // Set up the device name box
    device_box_.pack_start(device_icon_, Gtk::PACK_SHRINK);
    device_box_.pack_start(label_, Gtk::PACK_SHRINK);
    inner_box->pack_start(device_box_, Gtk::PACK_SHRINK);

    // Set up the volume scale
    volume_scale_.set_range(0, 100);
    volume_scale_.set_value(sink.volume);
    volume_scale_.set_size_request(200, -1);
    volume_scale_.set_draw_value(true);
    volume_scale_.set_has_origin(true);

    // Set up the mute button with an icon
    mute_button_.set_active(sink.muted);
    mute_button_.set_relief(Gtk::RELIEF_NONE);

    // Set up the volume icon
    update_volume_icon(sink.volume, sink.muted);

    // Set up the control box
    control_box_.pack_start(volume_icon_, Gtk::PACK_SHRINK);
    control_box_.pack_start(volume_scale_, Gtk::PACK_EXPAND_WIDGET);
    control_box_.pack_start(mute_button_, Gtk::PACK_SHRINK);
    inner_box->pack_start(control_box_, Gtk::PACK_SHRINK);

    // Connect signals
    volume_scale_.signal_value_changed().connect([this]() {
        int vol = static_cast<int>(volume_scale_.get_value());
        manager_->set_volume(sink_name_, vol);
        update_volume_icon(vol, mute_button_.get_active());
    });

    mute_button_.signal_toggled().connect([this]() {
        manager_->toggle_mute(sink_name_);
        update_volume_icon(static_cast<int>(volume_scale_.get_value()), mute_button_.get_active());
    });

    show_all_children();
}

void VolumeWidget::on_volume_changed() {
    int vol = static_cast<int>(volume_scale_.get_value());
    manager_->set_volume(sink_name_, vol);
    update_volume_icon(vol, mute_button_.get_active());
}

void VolumeWidget::on_mute_toggled() {
    manager_->toggle_mute(sink_name_);
    update_volume_icon(static_cast<int>(volume_scale_.get_value()), mute_button_.get_active());
}

void VolumeWidget::update_volume_icon(int volume, bool muted) {
    std::string icon_name;

    if (muted) {
        if (is_input_device_) {
            icon_name = "microphone-sensitivity-muted-symbolic";
            mute_button_.set_image_from_icon_name("microphone-sensitivity-muted-symbolic", Gtk::ICON_SIZE_BUTTON);
        } else {
            icon_name = "audio-volume-muted-symbolic";
            mute_button_.set_image_from_icon_name("audio-volume-muted-symbolic", Gtk::ICON_SIZE_BUTTON);
        }
    } else {
        if (is_input_device_) {
            if (volume < 30) {
                icon_name = "microphone-sensitivity-low-symbolic";
                mute_button_.set_image_from_icon_name("microphone-sensitivity-high-symbolic", Gtk::ICON_SIZE_BUTTON);
            } else if (volume < 70) {
                icon_name = "microphone-sensitivity-medium-symbolic";
                mute_button_.set_image_from_icon_name("microphone-sensitivity-high-symbolic", Gtk::ICON_SIZE_BUTTON);
            } else {
                icon_name = "microphone-sensitivity-high-symbolic";
                mute_button_.set_image_from_icon_name("microphone-sensitivity-high-symbolic", Gtk::ICON_SIZE_BUTTON);
            }
        } else {
            if (volume < 30) {
                icon_name = "audio-volume-low-symbolic";
                mute_button_.set_image_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);
            } else if (volume < 70) {
                icon_name = "audio-volume-medium-symbolic";
                mute_button_.set_image_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);
            } else {
                icon_name = "audio-volume-high-symbolic";
                mute_button_.set_image_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);
            }
        }
    }

    volume_icon_.set_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
}

VolumeWidget::~VolumeWidget() = default;

} // namespace Volume
