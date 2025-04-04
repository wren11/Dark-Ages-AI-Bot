/**
 * @file data_loader.h
 * @brief Loads game data from configuration files
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <mutex>
#include "../utils/config/config_manager.h"
#include "../utils/logging.h"

namespace game {

/**
 * @brief Loads and manages game data from configuration files
 * 
 * This class handles loading game data such as items, spells, and
 * packet definitions from external files, making it easier to update
 * without recompiling.
 */
class DataLoader {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static DataLoader& getInstance();
    
    /**
     * @brief Initialize the data loader
     * @param dataDir Directory containing data files
     * @return True if initialization succeeded
     */
    bool initialize(const std::filesystem::path& dataDir = "data");
    
    /**
     * @brief Load all data files
     * @return True if loading succeeded
     */
    bool loadAllData();
    
    /**
     * @brief Load a specific data file
     * @param filename Name of the data file
     * @return True if loading succeeded
     */
    bool loadDataFile(const std::string& filename);
    
    /**
     * @brief Get an item ID by name
     * @param name Name of the item
     * @return Item ID, or 0 if not found
     */
    uint32_t getItemId(const std::string& name) const;
    
    /**
     * @brief Get an item name by ID
     * @param id Item ID
     * @return Item name, or empty string if not found
     */
    std::string getItemName(uint32_t id) const;
    
    /**
     * @brief Get a spell ID by name
     * @param name Name of the spell
     * @return Spell ID, or 0 if not found
     */
    uint32_t getSpellId(const std::string& name) const;
    
    /**
     * @brief Get a spell name by ID
     * @param id Spell ID
     * @return Spell name, or empty string if not found
     */
    std::string getSpellName(uint32_t id) const;
    
    /**
     * @brief Get a packet name by ID
     * @param id Packet ID
     * @param isOutgoing Whether this is an outgoing packet
     * @return Packet name, or empty string if not found
     */
    std::string getPacketName(uint8_t id, bool isOutgoing) const;
    
    /**
     * @brief Get a packet ID by name
     * @param name Name of the packet
     * @param isOutgoing Whether this is an outgoing packet
     * @return Packet ID, or 0 if not found
     */
    uint8_t getPacketId(const std::string& name, bool isOutgoing) const;
    
    /**
     * @brief Create default data files if they don't exist
     * @return True if creation succeeded
     */
    bool createDefaultDataFiles();

private:
    // Private constructor for singleton pattern
    DataLoader() = default;
    ~DataLoader() = default;
    
    // Prevent copy/assignment
    DataLoader(const DataLoader&) = delete;
    DataLoader& operator=(const DataLoader&) = delete;
    
    // Data storage
    std::unordered_map<std::string, uint32_t> itemNameToId_;
    std::unordered_map<uint32_t, std::string> itemIdToName_;
    std::unordered_map<std::string, uint32_t> spellNameToId_;
    std::unordered_map<uint32_t, std::string> spellIdToName_;
    std::unordered_map<std::string, uint8_t> outgoingPacketNameToId_;
    std::unordered_map<uint8_t, std::string> outgoingPacketIdToName_;
    std::unordered_map<std::string, uint8_t> incomingPacketNameToId_;
    std::unordered_map<uint8_t, std::string> incomingPacketIdToName_;
    
    // Data directory
    std::filesystem::path dataDir_;
    
    // Thread safety mutex
    mutable std::mutex mutex_;
    
    // Helper methods
    bool loadItems();
    bool loadSpells();
    bool loadPackets();
    bool createDefaultItemsFile();
    bool createDefaultSpellsFile();
    bool createDefaultPacketsFile();
};

} // namespace game
