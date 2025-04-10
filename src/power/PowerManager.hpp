/**
 * @file PowerManager.hpp
 * @brief Power management functionality for Ultimate Control
 *
 * This file defines the PowerManager class which provides an interface for
 * system power operations like shutdown, reboot, suspend, and hibernate,
 * as well as power profile management.
 */

#ifndef ULTIMATE_CONTROL_POWER_MANAGER_HPP
#define ULTIMATE_CONTROL_POWER_MANAGER_HPP

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "PowerSettings.hpp"

/**
 * @namespace Power
 * @brief Contains power management functionality
 */
namespace Power {

/**
 * @class PowerManager
 * @brief Manages system power operations and power profiles
 *
 * Provides functionality for system power operations (shutdown, reboot, etc.)
 * and power profile management. Uses the PowerSettings class to retrieve
 * configured commands for power operations.
 */
class PowerManager {
public:
    /**
     * @typedef Callback
     * @brief Function type for update notifications
     *
     * Callback function type used to notify when power operations are performed.
     */
    using Callback = std::function<void()>;

    /**
     * @brief Constructor
     *
     * Initializes the power manager and loads power settings.
     */
    PowerManager();

    /**
     * @brief Destructor
     */
    ~PowerManager();

    /**
     * @brief Shut down the system
     *
     * Executes the configured shutdown command.
     */
    void shutdown();

    /**
     * @brief Reboot the system
     *
     * Executes the configured reboot command.
     */
    void reboot();

    /**
     * @brief Suspend the system
     *
     * Executes the configured suspend command.
     */
    void suspend();

    /**
     * @brief Hibernate the system
     *
     * Executes the configured hibernate command.
     */
    void hibernate();

    /**
     * @brief List available power profiles
     * @return Vector of available power profile names
     *
     * Retrieves the list of available power profiles from the system.
     */
    std::vector<std::string> list_power_profiles();

    /**
     * @brief Set the current power profile
     * @param profile The name of the profile to set
     *
     * Sets the system power profile to the specified profile.
     */
    void set_power_profile(const std::string& profile);

    /**
     * @brief Get the current power profile
     * @return The name of the current power profile
     *
     * Retrieves the name of the currently active power profile.
     */
    std::string get_current_power_profile();

    /**
     * @brief Set the update callback function
     * @param cb The callback function to call when power operations are performed
     *
     * Sets the callback function that will be called when power operations are performed.
     */
    void set_update_callback(Callback cb);

    /**
     * @brief Get the power settings object
     * @return Shared pointer to the PowerSettings object
     *
     * Returns the PowerSettings object that contains the configured power commands.
     */
    std::shared_ptr<PowerSettings> get_settings() const;

private:
    Callback callback_;                          ///< Callback function for update notifications
    std::shared_ptr<PowerSettings> settings_;    ///< Power settings object

    /**
     * @brief Notify listeners of power operations
     *
     * Calls the registered callback function if one is set.
     */
    void notify();
};

} // namespace Power

#endif // ULTIMATE_CONTROL_POWER_MANAGER_HPP
