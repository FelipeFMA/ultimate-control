/**
 * @file TabSettings.hpp
 * @brief Tab configuration management for Ultimate Control
 *
 * This file defines the TabSettings class which provides functionality
 * for managing tab order, visibility, and other tab-related settings.
 * Settings are persisted to a configuration file.
 */

#ifndef ULTIMATE_CONTROL_TAB_SETTINGS_HPP
#define ULTIMATE_CONTROL_TAB_SETTINGS_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

/**
 * @namespace Settings
 * @brief Contains settings management functionality
 */
namespace Settings {

/**
 * @struct TabInfo
 * @brief Information about a tab in the application
 *
 * Contains all the information needed to display and manage a tab,
 * including its identifier, display name, icon, and enabled state.
 */
struct TabInfo {
    std::string id;        ///< Unique identifier for the tab
    std::string name;      ///< Display name shown in the UI
    std::string icon_name; ///< Name of the icon to display
    bool enabled;          ///< Whether the tab is currently enabled
};

/**
 * @class TabSettings
 * @brief Manages tab configuration settings
 *
 * Provides functionality for loading, saving, and accessing tab settings
 * such as tab order and visibility. Settings are persisted to a
 * configuration file in the user's home directory.
 */
class TabSettings {
public:
    /**
     * @brief Constructor
     *
     * Initializes default tab settings and loads any existing settings
     * from the configuration file.
     */
    TabSettings();

    /**
     * @brief Destructor
     *
     * Saves settings to the configuration file before destruction.
     */
    ~TabSettings();

    /**
     * @brief Load settings from the configuration file
     *
     * Reads tab settings from the configuration file. If the file
     * doesn't exist or can't be read, default settings are used.
     */
    void load();

    /**
     * @brief Save settings to the configuration file
     *
     * Writes the current tab settings to the configuration file.
     * Creates the configuration directory if it doesn't exist.
     */
    void save() const;

    /**
     * @brief Get the current tab order
     * @return Vector of tab IDs in display order
     *
     * Returns the ordered list of tab IDs that determines
     * the display order of tabs in the UI.
     */
    std::vector<std::string> get_tab_order() const;

    /**
     * @brief Set the tab order
     * @param order Vector of tab IDs in the desired order
     *
     * Sets the order in which tabs should be displayed in the UI.
     */
    void set_tab_order(const std::vector<std::string>& order);

    /**
     * @brief Check if a tab is enabled
     * @param tab_id The ID of the tab to check
     * @return true if the tab is enabled, false otherwise
     *
     * Determines whether a tab should be displayed in the UI.
     */
    bool is_tab_enabled(const std::string& tab_id) const;

    /**
     * @brief Enable or disable a tab
     * @param tab_id The ID of the tab to modify
     * @param enabled Whether the tab should be enabled
     *
     * Sets whether a tab should be displayed in the UI.
     */
    void set_tab_enabled(const std::string& tab_id, bool enabled);

    /**
     * @brief Get information about all tabs
     * @return Vector of TabInfo structures for all tabs
     *
     * Returns information about all tabs in their current display order,
     * including their enabled state.
     */
    std::vector<TabInfo> get_all_tabs() const;

    /**
     * @brief Move a tab up in the order
     * @param tab_id The ID of the tab to move
     * @return true if the tab was moved, false if it couldn't be moved
     *
     * Moves the specified tab one position earlier in the display order.
     * Returns false if the tab is already at the beginning of the list.
     */
    bool move_tab_up(const std::string& tab_id);

    /**
     * @brief Move a tab down in the order
     * @param tab_id The ID of the tab to move
     * @return true if the tab was moved, false if it couldn't be moved
     *
     * Moves the specified tab one position later in the display order.
     * Returns false if the tab is already at the end of the list.
     */
    bool move_tab_down(const std::string& tab_id);

private:
    /**
     * @brief Create the configuration directory if it doesn't exist
     *
     * Creates all directories in the path to the configuration file
     * if they don't already exist.
     */
    void ensure_config_dir() const;

    std::string config_path_;                  ///< Path to the configuration file
    std::vector<std::string> tab_order_;        ///< Ordered list of tab IDs
    std::map<std::string, bool> tab_enabled_;   ///< Map of tab IDs to enabled state
    std::map<std::string, TabInfo> tab_info_;   ///< Map of tab IDs to tab information
};

} // namespace Settings

#endif // ULTIMATE_CONTROL_TAB_SETTINGS_HPP
