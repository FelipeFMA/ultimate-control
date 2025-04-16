/**
 * @file VolumeWidget.hpp
 * @brief Widget for displaying and controlling an audio device
 *
 * This file defines the VolumeWidget class which displays a single
 * audio device with volume slider and mute button controls.
 */

#pragma once

#include <gtkmm.h>
#include "VolumeManager.hpp"
#include <memory>

namespace Volume
{

    /**
     * @class VolumeWidget
     * @brief Widget for displaying and controlling an audio device
     *
     * Displays information about an audio device including its name and type.
     * Provides a volume slider and mute button for controlling the device.
     * Updates icons based on volume level and mute state.
     */
    class VolumeWidget : public Gtk::Box
    {
    public:
        /**
         * @brief Constructor for the volume widget
         * @param sink The audio device to display
         * @param manager Shared pointer to the volume manager
         *
         * Creates a widget that displays information about an audio device and
         * provides controls for adjusting its volume and mute state.
         */
        VolumeWidget(const AudioSink &sink, std::shared_ptr<VolumeManager> manager);

        /**
         * @brief Virtual destructor
         */
        virtual ~VolumeWidget();

    private:
        /**
         * @brief Handler for volume slider changes
         *
         * Updates the device volume and icon when the slider is moved.
         */
        void on_volume_changed();

        /**
         * @brief Handler for mute button toggles
         *
         * Toggles the device mute state and updates the icon.
         */
        void on_mute_toggled();

        /**
         * @brief Update the volume icon based on level and mute state
         * @param volume The current volume level (0-100)
         * @param muted Whether the device is muted
         *
         * Sets appropriate icons for both the volume indicator and mute button
         * based on the current volume level and mute state.
         */
        void update_volume_icon(int volume, bool muted);

        /**
         * @brief Handler for default device check button toggles
         *
         * Sets this device as the default device for its type when checked.
         */
        void on_default_toggled();

        std::shared_ptr<VolumeManager> manager_; ///< Volume manager for audio operations
        std::string sink_name_;                  ///< Name of the audio device
        bool is_input_device_;                   ///< Whether this is an input device (mic) or output (speaker)
        bool is_default_;                        ///< Whether this device is currently the default

        Gtk::Box device_box_;            ///< Container for device icon and name
        Gtk::Box control_box_;           ///< Container for volume controls
        Gtk::Box default_box_;           ///< Container for default device checkbox
        Gtk::Image device_icon_;         ///< Icon showing device type (speaker/mic)
        Gtk::Label label_;               ///< Label showing device name
        Gtk::Scale volume_scale_;        ///< Slider for volume control
        Gtk::Image volume_icon_;         ///< Icon showing current volume level
        Gtk::ToggleButton mute_button_;  ///< Button to toggle mute state
        Gtk::CheckButton default_check_; ///< Check button to set as default device
    };

} // namespace Volume
