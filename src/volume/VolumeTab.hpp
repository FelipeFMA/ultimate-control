/**
 * @file VolumeTab.hpp
 * @brief Volume control tab for the Ultimate Control application
 *
 * This file defines the VolumeTab class which provides a user interface
 * for controlling audio device volumes. It displays separate tabs for
 * input and output devices.
 */

#ifndef ULTIMATE_CONTROL_VOLUME_TAB_HPP
#define ULTIMATE_CONTROL_VOLUME_TAB_HPP

#include <gtkmm.h>
#include "VolumeManager.hpp"
#include "VolumeWidget.hpp"
#include <memory>
#include <vector>

namespace Volume {

/**
 * @class VolumeTab
 * @brief Tab for audio volume control
 *
 * Provides a user interface for controlling audio device volumes.
 * Displays separate tabs for input and output devices, each with
 * volume sliders and mute buttons.
 */
class VolumeTab : public Gtk::ScrolledWindow {
public:
    /**
     * @brief Constructor for the volume tab
     *
     * Initializes the volume manager, creates the UI components,
     * and performs an initial device scan.
     */
    VolumeTab();

    /**
     * @brief Virtual destructor
     */
    virtual ~VolumeTab();

private:
    /**
     * @brief Update the list of displayed audio devices
     * @param sinks Vector of AudioSink objects to display
     *
     * Clears the current list of volume widgets and creates new ones
     * for each audio device in the provided vector. Separates devices
     * into input and output categories.
     */
    void update_sink_list(const std::vector<AudioSink>& sinks);

    std::shared_ptr<VolumeManager> manager_;  ///< Volume manager for audio device operations

    Gtk::Notebook notebook_;  ///< Notebook widget for input/output tabs
    Gtk::Box output_box_;     ///< Container for output device widgets
    Gtk::Box input_box_;      ///< Container for input device widgets

    std::vector<std::unique_ptr<VolumeWidget>> output_widgets_;  ///< List of output device widgets
    std::vector<std::unique_ptr<VolumeWidget>> input_widgets_;   ///< List of input device widgets
};

} // namespace Volume

#endif // ULTIMATE_CONTROL_VOLUME_TAB_HPP
