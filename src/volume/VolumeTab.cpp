/**
 * @file VolumeTab.cpp
 * @brief Implementation of the volume control tab
 *
 * This file implements the VolumeTab class which provides a user interface
 * for controlling audio device volumes.
 */

#include "VolumeTab.hpp"
#include <iostream>

namespace Volume {

/**
 * @brief Constructor for the volume tab
 *
 * Initializes the volume manager, creates the UI components,
 * and performs an initial device scan.
 */
VolumeTab::VolumeTab()
: manager_(std::make_shared<VolumeManager>()),  // Create volume manager
  output_box_(Gtk::ORIENTATION_VERTICAL, 10),   // Vertical container for output devices
  input_box_(Gtk::ORIENTATION_VERTICAL, 10)     // Vertical container for input devices
{
    // Set scrolling policy for the main window
    set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    add(notebook_);

    // Enable scrolling for tabs if there are many devices
    notebook_.set_scrollable(true);

    // Create output devices tab with speaker icon
    auto output_tab_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));
    auto output_icon = Gtk::manage(new Gtk::Image());
    output_icon->set_from_icon_name("audio-speakers-symbolic", Gtk::ICON_SIZE_MENU);
    auto output_label = Gtk::manage(new Gtk::Label("Output Devices"));
    output_tab_box->pack_start(*output_icon, Gtk::PACK_SHRINK);
    output_tab_box->pack_start(*output_label, Gtk::PACK_SHRINK);
    output_tab_box->show_all();

    // Create input devices tab with microphone icon
    auto input_tab_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 5));
    auto input_icon = Gtk::manage(new Gtk::Image());
    input_icon->set_from_icon_name("audio-input-microphone-symbolic", Gtk::ICON_SIZE_MENU);
    auto input_label = Gtk::manage(new Gtk::Label("Input Devices"));
    input_tab_box->pack_start(*input_icon, Gtk::PACK_SHRINK);
    input_tab_box->pack_start(*input_label, Gtk::PACK_SHRINK);
    input_tab_box->show_all();

    // Add the tabs with icons to the notebook
    notebook_.append_page(output_box_, *output_tab_box);
    notebook_.append_page(input_box_, *input_tab_box);

    // Add padding to the device container boxes
    output_box_.set_margin_start(10);
    output_box_.set_margin_end(10);
    output_box_.set_margin_top(10);
    output_box_.set_margin_bottom(10);

    input_box_.set_margin_start(10);
    input_box_.set_margin_end(10);
    input_box_.set_margin_top(10);
    input_box_.set_margin_bottom(10);

    // Register callback for audio device list updates
    manager_->set_update_callback([this](const std::vector<AudioSink>& sinks) {
        update_sink_list(sinks);
    });

    // Perform initial scan for audio devices
    manager_->refresh_sinks();

    show_all_children();
    std::cout << "Volume tab loaded!" << std::endl;
}

/**
 * @brief Destructor for the volume tab
 */
VolumeTab::~VolumeTab() = default;

/**
 * @brief Update the list of displayed audio devices
 * @param sinks Vector of AudioSink objects to display
 *
 * Clears the current list of volume widgets and creates new ones
 * for each audio device in the provided vector. Separates devices
 * into input and output categories.
 */
void VolumeTab::update_sink_list(const std::vector<AudioSink>& sinks) {
    // Remove and clear all existing output device widgets
    for (auto& widget : output_widgets_) {
        output_box_.remove(*widget);
    }
    output_widgets_.clear();

    // Remove and clear all existing input device widgets
    for (auto& widget : input_widgets_) {
        input_box_.remove(*widget);
    }
    input_widgets_.clear();

    // Create new widgets for each audio device
    for (const auto& sink : sinks) {
        // Skip monitor devices (virtual loopback devices)
        if (sink.description.find("Monitor of") != std::string::npos || sink.name.find("Monitor of") != std::string::npos) {
            continue;
        }
        // Create a widget for this audio device
        auto widget = std::make_unique<VolumeWidget>(sink, manager_);

        // Add to either input or output tab based on device type
        if (sink.name.find("input") != std::string::npos || sink.name.find("source") != std::string::npos) {
            // This is an input device (microphone, line-in, etc.)
            input_box_.pack_start(*widget, Gtk::PACK_SHRINK);
            input_widgets_.push_back(std::move(widget));
        } else {
            // This is an output device (speakers, headphones, etc.)
            output_box_.pack_start(*widget, Gtk::PACK_SHRINK);
            output_widgets_.push_back(std::move(widget));
        }
    }

    // Make sure all new widgets are visible
    show_all_children();
}

} // namespace Volume
