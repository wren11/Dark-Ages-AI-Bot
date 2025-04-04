/**
 * @file config_manager.h
 * @brief Configuration management system for the bot
 */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <optional>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <functional>
#include "../../pch.h"

namespace utils {

/**
 * @brief Configuration manager to handle loading/saving settings from files
 * 
 * Provides a central place to define and access configuration values
 * with type-safety and default values. Automatically saves changes.
 */
class ConfigManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton instance
     */
    static ConfigManager& getInstance();

    /**
     * @brief Initialize the configuration system
     * @param configDir Directory to store configuration files
     * @return True if initialization succeeded
     */
    bool initialize(const std::filesystem::path& configDir = "config");

    /**
     * @brief Load a specific configuration file
     * @param filename Name of the configuration file
     * @return True if loading succeeded
     */
    bool loadConfig(const std::string& filename);

    /**
     * @brief Save a specific configuration to file
     * @param filename Name of the configuration file
     * @return True if saving succeeded
     */
    bool saveConfig(const std::string& filename);

    /**
     * @brief Get an integer configuration value
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if not found
     * @return The configuration value
     */
    int getInt(const std::string& section, const std::string& key, int defaultValue = 0);

    /**
     * @brief Get a string configuration value
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if not found
     * @return The configuration value
     */
    std::string getString(const std::string& section, const std::string& key, const std::string& defaultValue = "");

    /**
     * @brief Get a boolean configuration value
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if not found
     * @return The configuration value
     */
    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false);

    /**
     * @brief Get a double configuration value
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if not found
     * @return The configuration value
     */
    double getDouble(const std::string& section, const std::string& key, double defaultValue = 0.0);

    /**
     * @brief Get a memory address value (uintptr_t)
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if not found
     * @return The configuration value
     */
    uintptr_t getAddress(const std::string& section, const std::string& key, uintptr_t defaultValue = 0);

    /**
     * @brief Set an integer configuration value
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void setInt(const std::string& section, const std::string& key, int value);

    /**
     * @brief Set a string configuration value
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void setString(const std::string& section, const std::string& key, const std::string& value);

    /**
     * @brief Set a boolean configuration value
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void setBool(const std::string& section, const std::string& key, bool value);

    /**
     * @brief Set a double configuration value
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void setDouble(const std::string& section, const std::string& key, double value);

    /**
     * @brief Set a memory address value
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void setAddress(const std::string& section, const std::string& key, uintptr_t value);

    /**
     * @brief Load default memory addresses
     * Initializes memory addresses with default values from the game
     */
    void loadDefaultAddresses();

    /**
     * @brief Register a callback for when a configuration value changes
     * @param section Section name
     * @param key Key name
     * @param callback Function to call when value changes
     * @return ID that can be used to unregister the callback
     */
    size_t registerChangeCallback(const std::string& section, const std::string& key,
        std::function<void(const std::string&, const std::string&)> callback);

    /**
     * @brief Unregister a callback
     * @param callbackId ID of the callback to unregister
     * @return True if callback was found and unregistered
     */
    bool unregisterChangeCallback(size_t callbackId);

private:
    // Private constructor for singleton pattern
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    // Prevent copy/assignment
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // Config value variant type
    using ConfigValue = std::variant<int, double, bool, std::string, uintptr_t>;
    
    // Configuration data storage
    std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>> configData_;
    
    // Changed flag for each file
    std::unordered_map<std::string, bool> configChanged_;
    
    // Base directory for config files
    std::filesystem::path configDir_;
    
    // Thread safety mutex
    mutable std::mutex mutex_;
    
    // Tracking for callbacks
    size_t nextCallbackId_ = 1;
    std::unordered_map<size_t, std::tuple<std::string, std::string, 
        std::function<void(const std::string&, const std::string&)>>> callbacks_;
    
    // Helper methods
    void loadDefaults();
    void notifyCallbacks(const std::string& section, const std::string& key);
    
    // Actually parse the config file
    bool parseConfigFile(const std::filesystem::path& path, std::unordered_map<std::string, 
        std::unordered_map<std::string, ConfigValue>>& data);
    
    // Write config data to file
    bool writeConfigFile(const std::filesystem::path& path, const std::unordered_map<std::string, 
        std::unordered_map<std::string, ConfigValue>>& data);
};

} // namespace utils
