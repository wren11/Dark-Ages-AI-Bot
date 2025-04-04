#include "pch.h"

/**
 * @file data_loader.cpp
 * @brief Implementation of game data loading from configuration files
 */
#include "data_loader.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include "json.h" // Changed from json/json.h

namespace game {

DataLoader& DataLoader::getInstance() {
    static DataLoader instance;
    return instance;
}

bool DataLoader::initialize(const std::filesystem::path& dataDir) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Store the data directory
        dataDir_ = dataDir;
        
        // Create data directory if it doesn't exist
        if (!std::filesystem::exists(dataDir_)) {
            if (!std::filesystem::create_directories(dataDir_)) {
                utils::Logging::error("Failed to create data directory: " + dataDir_.string());
                return false;
            }
            utils::Logging::info("Created data directory: " + dataDir_.string());
            
            // Create default data files
            createDefaultDataFiles();
        }
        
        // Load all data files
        bool success = loadAllData();
        
        utils::Logging::info("Game data system initialized.");
        return success;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to initialize game data system: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::loadAllData() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    bool success = true;
    
    if (!loadItems()) {
        utils::Logging::warning("Failed to load items data");
        success = false;
    }
    
    if (!loadSpells()) {
        utils::Logging::warning("Failed to load spells data");
        success = false;
    }
    
    if (!loadPackets()) {
        utils::Logging::warning("Failed to load packets data");
        success = false;
    }
    
    return success;
}

bool DataLoader::loadDataFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (filename == "items.json") {
        return loadItems();
    }
    else if (filename == "spells.json") {
        return loadSpells();
    }
    else if (filename == "packets.json") {
        return loadPackets();
    }
    
    utils::Logging::warning("Unknown data file: " + filename);
    return false;
}

uint32_t DataLoader::getItemId(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = itemNameToId_.find(name);
    if (it != itemNameToId_.end()) {
        return it->second;
    }
    
    // Try case-insensitive search
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), 
                  [](unsigned char c){ return std::tolower(c); });
    
    for (const auto& [itemName, itemId] : itemNameToId_) {
        std::string lowerItemName = itemName;
        std::transform(lowerItemName.begin(), lowerItemName.end(), lowerItemName.begin(), 
                      [](unsigned char c){ return std::tolower(c); });
        
        if (lowerItemName == lowerName) {
            return itemId;
        }
    }
    
    return 0;
}

std::string DataLoader::getItemName(uint32_t id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = itemIdToName_.find(id);
    if (it != itemIdToName_.end()) {
        return it->second;
    }
    
    return "";
}

uint32_t DataLoader::getSpellId(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = spellNameToId_.find(name);
    if (it != spellNameToId_.end()) {
        return it->second;
    }
    
    // Try case-insensitive search
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), 
                  [](unsigned char c){ return std::tolower(c); });
    
    for (const auto& [spellName, spellId] : spellNameToId_) {
        std::string lowerSpellName = spellName;
        std::transform(lowerSpellName.begin(), lowerSpellName.end(), lowerSpellName.begin(), 
                      [](unsigned char c){ return std::tolower(c); });
        
        if (lowerSpellName == lowerName) {
            return spellId;
        }
    }
    
    return 0;
}

std::string DataLoader::getSpellName(uint32_t id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = spellIdToName_.find(id);
    if (it != spellIdToName_.end()) {
        return it->second;
    }
    
    return "";
}

std::string DataLoader::getPacketName(uint8_t id, bool isOutgoing) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOutgoing) {
        auto it = outgoingPacketIdToName_.find(id);
        if (it != outgoingPacketIdToName_.end()) {
            return it->second;
        }
    }
    else {
        auto it = incomingPacketIdToName_.find(id);
        if (it != incomingPacketIdToName_.end()) {
            return it->second;
        }
    }
    
    // Return hex representation if not found
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << static_cast<int>(id);
    return ss.str();
}

uint8_t DataLoader::getPacketId(const std::string& name, bool isOutgoing) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOutgoing) {
        auto it = outgoingPacketNameToId_.find(name);
        if (it != outgoingPacketNameToId_.end()) {
            return it->second;
        }
    }
    else {
        auto it = incomingPacketNameToId_.find(name);
        if (it != incomingPacketNameToId_.end()) {
            return it->second;
        }
    }
    
    // Try to parse as hex if not found
    if (name.size() > 2 && name.substr(0, 2) == "0x") {
        try {
            return static_cast<uint8_t>(std::stoul(name.substr(2), nullptr, 16));
        }
        catch (...) {
            // Ignore conversion errors
        }
    }
    
    return 0;
}

bool DataLoader::createDefaultDataFiles() {
    bool success = true;
    
    if (!createDefaultItemsFile()) {
        utils::Logging::warning("Failed to create default items file");
        success = false;
    }
    
    if (!createDefaultSpellsFile()) {
        utils::Logging::warning("Failed to create default spells file");
        success = false;
    }
    
    if (!createDefaultPacketsFile()) {
        utils::Logging::warning("Failed to create default packets file");
        success = false;
    }
    
    return success;
}

bool DataLoader::loadItems() {
    try {
        std::filesystem::path filePath = dataDir_ / "items.json";
        
        if (!std::filesystem::exists(filePath)) {
            utils::Logging::warning("Items file doesn't exist: " + filePath.string());
            return createDefaultItemsFile();
        }
        
        // Clear existing data
        itemNameToId_.clear();
        itemIdToName_.clear();
        
        // Read JSON file
        std::ifstream file(filePath);
        if (!file.is_open()) {
            utils::Logging::error("Failed to open items file: " + filePath.string());
            return false;
        }
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING errs;
        if (!Json::parseFromStream(builder, file, &root, &errs)) {
            utils::Logging::error("Failed to parse items file: " + errs);
            return false;
        }
        
        // Load items
        for (const auto& item : root) {
            if (!item.isMember("id") || !item.isMember("name")) {
                continue;
            }
            
            uint32_t id = item["id"].asUInt();
            std::string name = item["name"].asString();
            
            itemNameToId_[name] = id;
            itemIdToName_[id] = name;
        }
        
        utils::Logging::info("Loaded " + std::to_string(itemNameToId_.size()) + " items");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error loading items: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::loadSpells() {
    try {
        std::filesystem::path filePath = dataDir_ / "spells.json";
        
        if (!std::filesystem::exists(filePath)) {
            utils::Logging::warning("Spells file doesn't exist: " + filePath.string());
            return createDefaultSpellsFile();
        }
        
        // Clear existing data
        spellNameToId_.clear();
        spellIdToName_.clear();
        
        // Read JSON file
        std::ifstream file(filePath);
        if (!file.is_open()) {
            utils::Logging::error("Failed to open spells file: " + filePath.string());
            return false;
        }
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING errs;
        if (!Json::parseFromStream(builder, file, &root, &errs)) {
            utils::Logging::error("Failed to parse spells file: " + errs);
            return false;
        }
        
        // Load spells
        for (const auto& spell : root) {
            if (!spell.isMember("id") || !spell.isMember("name")) {
                continue;
            }
            
            uint32_t id = spell["id"].asUInt();
            std::string name = spell["name"].asString();
            
            spellNameToId_[name] = id;
            spellIdToName_[id] = name;
        }
        
        utils::Logging::info("Loaded " + std::to_string(spellNameToId_.size()) + " spells");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error loading spells: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::loadPackets() {
    try {
        std::filesystem::path filePath = dataDir_ / "packets.json";
        
        if (!std::filesystem::exists(filePath)) {
            utils::Logging::warning("Packets file doesn't exist: " + filePath.string());
            return createDefaultPacketsFile();
        }
        
        // Clear existing data
        incomingPacketNameToId_.clear();
        incomingPacketIdToName_.clear();
        outgoingPacketNameToId_.clear();
        outgoingPacketIdToName_.clear();
        
        // Read JSON file
        std::ifstream file(filePath);
        if (!file.is_open()) {
            utils::Logging::error("Failed to open packets file: " + filePath.string());
            return false;
        }
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        JSONCPP_STRING errs;
        if (!Json::parseFromStream(builder, file, &root, &errs)) {
            utils::Logging::error("Failed to parse packets file: " + errs);
            return false;
        }
        
        // Load incoming packets
        if (root.isMember("incoming") && root["incoming"].isArray()) {
            for (const auto& packet : root["incoming"]) {
                if (!packet.isMember("id") || !packet.isMember("name")) {
                    continue;
                }
                
                uint8_t id = static_cast<uint8_t>(packet["id"].asUInt());
                std::string name = packet["name"].asString();
                
                incomingPacketNameToId_[name] = id;
                incomingPacketIdToName_[id] = name;
            }
        }
        
        // Load outgoing packets
        if (root.isMember("outgoing") && root["outgoing"].isArray()) {
            for (const auto& packet : root["outgoing"]) {
                if (!packet.isMember("id") || !packet.isMember("name")) {
                    continue;
                }
                
                uint8_t id = static_cast<uint8_t>(packet["id"].asUInt());
                std::string name = packet["name"].asString();
                
                outgoingPacketNameToId_[name] = id;
                outgoingPacketIdToName_[id] = name;
            }
        }
        
        utils::Logging::info("Loaded " + std::to_string(incomingPacketNameToId_.size()) + 
                           " incoming and " + std::to_string(outgoingPacketNameToId_.size()) + 
                           " outgoing packets");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error loading packets: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::createDefaultItemsFile() {
    try {
        std::filesystem::path filePath = dataDir_ / "items.json";
        
        // Create default items JSON
        Json::Value root(Json::arrayValue);
        
        // Add some common items
        Json::Value item1;
        item1["id"] = 1001;
        item1["name"] = "Short Sword";
        item1["type"] = "weapon";
        item1["slot"] = "weapon";
        root.append(item1);
        
        Json::Value item2;
        item2["id"] = 1002;
        item2["name"] = "Wooden Shield";
        item2["type"] = "shield";
        item2["slot"] = "shield";
        root.append(item2);
        
        Json::Value item3;
        item3["id"] = 1003;
        item3["name"] = "Leather Armor";
        item3["type"] = "armor";
        item3["slot"] = "armor";
        root.append(item3);
        
        Json::Value item4;
        item4["id"] = 1004;
        item4["name"] = "Health Potion";
        item4["type"] = "consumable";
        item4["slot"] = "inventory";
        root.append(item4);
        
        // Write to file
        std::ofstream file(filePath);
        if (!file.is_open()) {
            utils::Logging::error("Failed to create items file: " + filePath.string());
            return false;
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        
        utils::Logging::info("Created default items file");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error creating default items file: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::createDefaultSpellsFile() {
    try {
        std::filesystem::path filePath = dataDir_ / "spells.json";
        
        // Create default spells JSON
        Json::Value root(Json::arrayValue);
        
        // Add some common spells
        Json::Value spell1;
        spell1["id"] = 101;
        spell1["name"] = "Minor Heal";
        spell1["type"] = "healing";
        spell1["mana_cost"] = 20;
        root.append(spell1);
        
        Json::Value spell2;
        spell2["id"] = 102;
        spell2["name"] = "Fireball";
        spell2["type"] = "offensive";
        spell2["mana_cost"] = 30;
        root.append(spell2);
        
        Json::Value spell3;
        spell3["id"] = 103;
        spell3["name"] = "Frost Nova";
        spell3["type"] = "offensive";
        spell3["mana_cost"] = 40;
        root.append(spell3);
        
        Json::Value spell4;
        spell4["id"] = 104;
        spell4["name"] = "Invisibility";
        spell4["type"] = "utility";
        spell4["mana_cost"] = 50;
        root.append(spell4);
        
        // Write to file
        std::ofstream file(filePath);
        if (!file.is_open()) {
            utils::Logging::error("Failed to create spells file: " + filePath.string());
            return false;
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        
        utils::Logging::info("Created default spells file");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error creating default spells file: " + std::string(e.what()));
        return false;
    }
}

bool DataLoader::createDefaultPacketsFile() {
    try {
        std::filesystem::path filePath = dataDir_ / "packets.json";
        
        // Create default packets JSON
        Json::Value root;
        
        // Create incoming packets array
        Json::Value incoming(Json::arrayValue);
        
        Json::Value packet1;
        packet1["id"] = 0x04;
        packet1["name"] = "PlayerMovement";
        packet1["description"] = "Player movement packet";
        incoming.append(packet1);
        
        Json::Value packet2;
        packet2["id"] = 0x0B;
        packet2["name"] = "EntityUpdate";
        packet2["description"] = "Entity update packet";
        incoming.append(packet2);
        
        Json::Value packet3;
        packet3["id"] = 0x0C;
        packet3["name"] = "ChatMessage";
        packet3["description"] = "Chat message packet";
        incoming.append(packet3);
        
        Json::Value packet4;
        packet4["id"] = 0x17;
        packet4["name"] = "InventoryUpdate";
        packet4["description"] = "Inventory update packet";
        incoming.append(packet4);
        
        Json::Value packet5;
        packet5["id"] = 0x0E;
        packet5["name"] = "SpellCast";
        packet5["description"] = "Spell cast packet";
        incoming.append(packet5);
        
        // Create outgoing packets array
        Json::Value outgoing(Json::arrayValue);
        
        Json::Value packet6;
        packet6["id"] = 0x1C;
        packet6["name"] = "PlayerAction";
        packet6["description"] = "Player action packet";
        outgoing.append(packet6);
        
        Json::Value packet7;
        packet7["id"] = 0x38;
        packet7["name"] = "UseItem";
        packet7["description"] = "Use item packet";
        outgoing.append(packet7);
        
        Json::Value packet8;
        packet8["id"] = 0x10;
        packet8["name"] = "CastSpell";
        packet8["description"] = "Cast spell packet";
        outgoing.append(packet8);
        
        Json::Value packet9;
        packet9["id"] = 0x0F;
        packet9["name"] = "Move";
        packet9["description"] = "Move packet";
        outgoing.append(packet9);
        
        Json::Value packet10;
        packet10["id"] = 0x13;
        packet10["name"] = "DropItem";
        packet10["description"] = "Drop item packet";
        outgoing.append(packet10);
        
        // Add to root
        root["incoming"] = incoming;
        root["outgoing"] = outgoing;
        
        // Write to file
        std::ofstream file(filePath);
        if (!file.is_open()) {
            utils::Logging::error("Failed to create packets file: " + filePath.string());
            return false;
        }
        
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        
        utils::Logging::info("Created default packets file");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error creating default packets file: " + std::string(e.what()));
        return false;
    }
}

} // namespace game
