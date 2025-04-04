#include "pch.h"

/**
 * @file config_manager.cpp
 * @brief Implementation of configuration management system
 */
#include "config_manager.h"
#include "../logging.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iomanip>

namespace utils {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::initialize(const std::filesystem::path& configDir) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Store the config directory
        configDir_ = configDir;
        
        // Create config directory if it doesn't exist
        if (!std::filesystem::exists(configDir_)) {
            if (!std::filesystem::create_directories(configDir_)) {
                Logging::error("Failed to create config directory: " + configDir_.string());
                return false;
            }
            Logging::info("Created config directory: " + configDir_.string());
        }
        
        // Load default settings
        loadDefaults();
        
        // Load all .ini files from config directory
        bool success = true;
        for (const auto& entry : std::filesystem::directory_iterator(configDir_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ini") {
                std::string filename = entry.path().filename().string();
                if (!loadConfig(filename)) {
                    Logging::warning("Failed to load config file: " + filename);
                    success = false;
                }
            }
        }
        
        Logging::info("Configuration system initialized.");
        return success;
    }
    catch (const std::exception& e) {
        Logging::error("Failed to initialize configuration system: " + std::string(e.what()));
        return false;
    }
}

bool ConfigManager::loadConfig(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if filename has .ini extension
        std::string actualFilename = filename;
        if (filename.find(".ini") == std::string::npos) {
            actualFilename += ".ini";
        }
        
        // Build full path
        std::filesystem::path filePath = configDir_ / actualFilename;
        
        // If file doesn't exist, create it with defaults
        if (!std::filesystem::exists(filePath)) {
            Logging::info("Config file doesn't exist, creating: " + filePath.string());
            configChanged_[actualFilename] = true;
            return saveConfig(actualFilename);
        }
        
        // Parse the config file
        std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>> tempData;
        if (!parseConfigFile(filePath, tempData)) {
            return false;
        }
        
        // Merge with existing data
        for (const auto& [section, keys] : tempData) {
            for (const auto& [key, value] : keys) {
                configData_[section][key] = value;
                
                // Notify callbacks
                notifyCallbacks(section, key);
            }
        }
        
        configChanged_[actualFilename] = false;
        Logging::info("Loaded config file: " + filePath.string());
        return true;
    }
    catch (const std::exception& e) {
        Logging::error("Failed to load config file: " + filename + " - " + std::string(e.what()));
        return false;
    }
}

bool ConfigManager::saveConfig(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if filename has .ini extension
        std::string actualFilename = filename;
        if (filename.find(".ini") == std::string::npos) {
            actualFilename += ".ini";
        }
        
        // Build full path
        std::filesystem::path filePath = configDir_ / actualFilename;
        
        // Only save if config has changed
        if (!configChanged_[actualFilename]) {
            return true;
        }
        
        // Write to file
        if (!writeConfigFile(filePath, configData_)) {
            return false;
        }
        
        configChanged_[actualFilename] = false;
        Logging::info("Saved config file: " + filePath.string());
        return true;
    }
    catch (const std::exception& e) {
        Logging::error("Failed to save config file: " + filename + " - " + std::string(e.what()));
        return false;
    }
}

int ConfigManager::getInt(const std::string& section, const std::string& key, int defaultValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& value = configData_[section][key];
            if (std::holds_alternative<int>(value)) {
                return std::get<int>(value);
            }
        }
        
        // Key not found or wrong type, set default
        setInt(section, key, defaultValue);
        return defaultValue;
    }
    catch (const std::exception& e) {
        Logging::error("Error getting int config value: " + std::string(e.what()));
        return defaultValue;
    }
}

std::string ConfigManager::getString(const std::string& section, const std::string& key, const std::string& defaultValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& value = configData_[section][key];
            if (std::holds_alternative<std::string>(value)) {
                return std::get<std::string>(value);
            }
        }
        
        // Key not found or wrong type, set default
        setString(section, key, defaultValue);
        return defaultValue;
    }
    catch (const std::exception& e) {
        Logging::error("Error getting string config value: " + std::string(e.what()));
        return defaultValue;
    }
}

bool ConfigManager::getBool(const std::string& section, const std::string& key, bool defaultValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& value = configData_[section][key];
            if (std::holds_alternative<bool>(value)) {
                return std::get<bool>(value);
            }
        }
        
        // Key not found or wrong type, set default
        setBool(section, key, defaultValue);
        return defaultValue;
    }
    catch (const std::exception& e) {
        Logging::error("Error getting bool config value: " + std::string(e.what()));
        return defaultValue;
    }
}

double ConfigManager::getDouble(const std::string& section, const std::string& key, double defaultValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& value = configData_[section][key];
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
        }
        
        // Key not found or wrong type, set default
        setDouble(section, key, defaultValue);
        return defaultValue;
    }
    catch (const std::exception& e) {
        Logging::error("Error getting double config value: " + std::string(e.what()));
        return defaultValue;
    }
}

uintptr_t ConfigManager::getAddress(const std::string& section, const std::string& key, uintptr_t defaultValue) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& value = configData_[section][key];
            if (std::holds_alternative<uintptr_t>(value)) {
                return std::get<uintptr_t>(value);
            }
        }
        
        // Key not found or wrong type, set default
        setAddress(section, key, defaultValue);
        return defaultValue;
    }
    catch (const std::exception& e) {
        Logging::error("Error getting address config value: " + std::string(e.what()));
        return defaultValue;
    }
}

void ConfigManager::setInt(const std::string& section, const std::string& key, int value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if value changed
        bool changed = true;
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& oldValue = configData_[section][key];
            if (std::holds_alternative<int>(oldValue) && std::get<int>(oldValue) == value) {
                changed = false;
            }
        }
        
        // Store value
        configData_[section][key] = value;
        
        // Mark as changed and notify
        if (changed) {
            for (auto& [filename, isDirty] : configChanged_) {
                isDirty = true;
            }
            notifyCallbacks(section, key);
        }
    }
    catch (const std::exception& e) {
        Logging::error("Error setting int config value: " + std::string(e.what()));
    }
}

void ConfigManager::setString(const std::string& section, const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if value changed
        bool changed = true;
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& oldValue = configData_[section][key];
            if (std::holds_alternative<std::string>(oldValue) && std::get<std::string>(oldValue) == value) {
                changed = false;
            }
        }
        
        // Store value
        configData_[section][key] = value;
        
        // Mark as changed and notify
        if (changed) {
            for (auto& [filename, isDirty] : configChanged_) {
                isDirty = true;
            }
            notifyCallbacks(section, key);
        }
    }
    catch (const std::exception& e) {
        Logging::error("Error setting string config value: " + std::string(e.what()));
    }
}

void ConfigManager::setBool(const std::string& section, const std::string& key, bool value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if value changed
        bool changed = true;
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& oldValue = configData_[section][key];
            if (std::holds_alternative<bool>(oldValue) && std::get<bool>(oldValue) == value) {
                changed = false;
            }
        }
        
        // Store value
        configData_[section][key] = value;
        
        // Mark as changed and notify
        if (changed) {
            for (auto& [filename, isDirty] : configChanged_) {
                isDirty = true;
            }
            notifyCallbacks(section, key);
        }
    }
    catch (const std::exception& e) {
        Logging::error("Error setting bool config value: " + std::string(e.what()));
    }
}

void ConfigManager::setDouble(const std::string& section, const std::string& key, double value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if value changed
        bool changed = true;
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& oldValue = configData_[section][key];
            if (std::holds_alternative<double>(oldValue) && std::get<double>(oldValue) == value) {
                changed = false;
            }
        }
        
        // Store value
        configData_[section][key] = value;
        
        // Mark as changed and notify
        if (changed) {
            for (auto& [filename, isDirty] : configChanged_) {
                isDirty = true;
            }
            notifyCallbacks(section, key);
        }
    }
    catch (const std::exception& e) {
        Logging::error("Error setting double config value: " + std::string(e.what()));
    }
}

void ConfigManager::setAddress(const std::string& section, const std::string& key, uintptr_t value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Check if value changed
        bool changed = true;
        if (configData_.count(section) && configData_[section].count(key)) {
            const auto& oldValue = configData_[section][key];
            if (std::holds_alternative<uintptr_t>(oldValue) && std::get<uintptr_t>(oldValue) == value) {
                changed = false;
            }
        }
        
        // Store value
        configData_[section][key] = value;
        
        // Mark as changed and notify
        if (changed) {
            for (auto& [filename, isDirty] : configChanged_) {
                isDirty = true;
            }
            notifyCallbacks(section, key);
        }
    }
    catch (const std::exception& e) {
        Logging::error("Error setting address config value: " + std::string(e.what()));
    }
}

void ConfigManager::loadDefaultAddresses() {
    // Set default addresses that the game uses
    // NOTE: These would be overridden by values in config files if they exist
    
    // Network function addresses (defaults from current game version)
    setAddress("NetworkAddresses", "SendPacket", 0x00487120);  // Example address
    setAddress("NetworkAddresses", "RecvPacket", 0x00487350);  // Example address
    
    // Game entity addresses
    setAddress("GameAddresses", "PlayerBase", 0x00A54D20);     // Example address
    setAddress("GameAddresses", "SpriteManager", 0x00A65F10);  // Example address
    setAddress("GameAddresses", "GameState", 0x00A48B30);      // Example address
    
    // DirectDraw function addresses
    setAddress("GraphicsAddresses", "DirectDrawCreate", 0x00492F80);  // Example address
    setAddress("GraphicsAddresses", "BlitSurface", 0x004A1270);       // Example address
    
    Logging::info("Loaded default memory addresses");
}

size_t ConfigManager::registerChangeCallback(const std::string& section, const std::string& key,
    std::function<void(const std::string&, const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t callbackId = nextCallbackId_++;
    callbacks_[callbackId] = std::make_tuple(section, key, callback);
    return callbackId;
}

bool ConfigManager::unregisterChangeCallback(size_t callbackId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (callbacks_.count(callbackId)) {
        callbacks_.erase(callbackId);
        return true;
    }
    return false;
}

void ConfigManager::loadDefaults() {
    // Load default game settings
    setBool("General", "EnableLogging", true);
    setInt("General", "LogLevel", static_cast<int>(LogLevel::Info));
    setBool("General", "MemoryScanning", true);
    
    // Packet handling defaults
    setBool("Network", "LogPackets", true);
    setBool("Network", "ShowOutgoingPackets", true);
    setBool("Network", "ShowIncomingPackets", true);
    
    // Intercept settings
    setBool("Intercept", "EnableHooks", true);
    setBool("Intercept", "TraceDirectDraw", false);
    setBool("Intercept", "TraceTcpFunctions", true);
    
    // Memory scan settings
    setBool("MemoryScan", "ScanOnStartup", true);
    setBool("MemoryScan", "AutoUpdateAddresses", true);
    setInt("MemoryScan", "ScanInterval", 0);  // 0 means only on startup, otherwise in milliseconds
    
    // Load default memory addresses
    loadDefaultAddresses();
}

void ConfigManager::notifyCallbacks(const std::string& section, const std::string& key) {
    for (const auto& [id, callbackData] : callbacks_) {
        const auto& [cbSection, cbKey, callback] = callbackData;
        
        // Call if matches section/key
        if (cbSection == section && cbKey == key) {
            try {
                callback(section, key);
            }
            catch (const std::exception& e) {
                Logging::error("Error in config change callback: " + std::string(e.what()));
            }
        }
    }
}

bool ConfigManager::parseConfigFile(const std::filesystem::path& path, 
    std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>>& data) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logging::error("Failed to open config file: " + path.string());
        return false;
    }
    
    std::string line;
    std::string currentSection;
    std::regex sectionRegex(R"(\s*\[([^\]]+)\]\s*)");
    std::regex keyValueRegex(R"(\s*([^=]+?)\s*=\s*(.*?)\s*$)");
    std::regex commentRegex(R"(\s*[;#].*)");
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || std::regex_match(line, commentRegex)) {
            continue;
        }
        
        // Check for section header
        std::smatch sectionMatch;
        if (std::regex_match(line, sectionMatch, sectionRegex)) {
            currentSection = sectionMatch[1].str();
            continue;
        }
        
        // Must have a section
        if (currentSection.empty()) {
            continue;
        }
        
        // Check for key-value pair
        std::smatch keyValueMatch;
        if (std::regex_match(line, keyValueMatch, keyValueRegex)) {
            std::string key = keyValueMatch[1].str();
            std::string valueStr = keyValueMatch[2].str();
            
            // Try to parse different value types
            // First, see if it's a boolean
            if (valueStr == "true" || valueStr == "True" || valueStr == "TRUE" || valueStr == "1") {
                data[currentSection][key] = true;
            }
            else if (valueStr == "false" || valueStr == "False" || valueStr == "FALSE" || valueStr == "0") {
                data[currentSection][key] = false;
            }
            // Check if it's a memory address (hex number with 0x prefix)
            else if (valueStr.size() > 2 && valueStr.substr(0, 2) == "0x") {
                try {
                    uintptr_t address = std::stoull(valueStr, nullptr, 16);
                    data[currentSection][key] = address;
                }
                catch (...) {
                    // Not a valid hex value, store as string
                    data[currentSection][key] = valueStr;
                }
            }
            // Check if it's an integer
            else if (std::all_of(valueStr.begin(), valueStr.end(), [](char c) {
                return std::isdigit(c) || c == '-' || c == '+';
            })) {
                try {
                    int intValue = std::stoi(valueStr);
                    data[currentSection][key] = intValue;
                }
                catch (...) {
                    // Not a valid int, store as string
                    data[currentSection][key] = valueStr;
                }
            }
            // Check if it's a double
            else if (std::count(valueStr.begin(), valueStr.end(), '.') == 1 &&
                std::all_of(valueStr.begin(), valueStr.end(), [](char c) {
                    return std::isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E';
                })) {
                try {
                    double doubleValue = std::stod(valueStr);
                    data[currentSection][key] = doubleValue;
                }
                catch (...) {
                    // Not a valid double, store as string
                    data[currentSection][key] = valueStr;
                }
            }
            // Otherwise, store as string
            else {
                data[currentSection][key] = valueStr;
            }
        }
    }
    
    return true;
}

bool ConfigManager::writeConfigFile(const std::filesystem::path& path,
    const std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>>& data) {
    // Open file
    std::ofstream file(path);
    if (!file.is_open()) {
        Logging::error("Failed to open config file for writing: " + path.string());
        return false;
    }
    
    // Write header comment
    file << "# Dark Ages AI Bot Configuration" << std::endl;
    file << "# Generated on " << std::put_time(std::localtime(&std::time(nullptr)), "%Y-%m-%d %H:%M:%S") << std::endl;
    file << "# Do not edit while the bot is running" << std::endl << std::endl;
    
    // Get sections and sort them for consistent output
    std::vector<std::string> sections;
    for (const auto& [section, _] : data) {
        sections.push_back(section);
    }
    std::sort(sections.begin(), sections.end());
    
    // Write each section
    for (const auto& section : sections) {
        file << "[" << section << "]" << std::endl;
        
        // Get keys and sort them
        std::vector<std::string> keys;
        for (const auto& [key, _] : data.at(section)) {
            keys.push_back(key);
        }
        std::sort(keys.begin(), keys.end());
        
        // Write each key-value pair
        for (const auto& key : keys) {
            const auto& value = data.at(section).at(key);
            
            file << key << " = ";
            
            // Format based on value type
            if (std::holds_alternative<int>(value)) {
                file << std::get<int>(value);
            }
            else if (std::holds_alternative<double>(value)) {
                file << std::fixed << std::setprecision(6) << std::get<double>(value);
            }
            else if (std::holds_alternative<bool>(value)) {
                file << (std::get<bool>(value) ? "true" : "false");
            }
            else if (std::holds_alternative<std::string>(value)) {
                file << std::get<std::string>(value);
            }
            else if (std::holds_alternative<uintptr_t>(value)) {
                file << "0x" << std::hex << std::uppercase << std::get<uintptr_t>(value) << std::dec;
            }
            
            file << std::endl;
        }
        
        file << std::endl;
    }
    
    return true;
}

} // namespace utils
