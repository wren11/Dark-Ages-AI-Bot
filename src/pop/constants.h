/**
 * @file constants.h
 * @brief Game memory addresses and constants
 * 
 * This file provides access to game memory addresses and constants.
 * All values are now loaded from configuration files and can be dynamically
 * updated through memory scanning.
 */
#pragma once

#include "utils/config/config_manager.h"
#include "utils/memory/memory_scanner.h"
#include <cstdint>

namespace constants {

/**
 * @brief Get a memory address from configuration or scanner
 * @param name Name of the address
 * @param defaultValue Default value if not found
 * @return Memory address
 */
inline uintptr_t getAddress(const std::string& name, uintptr_t defaultValue = 0) {
    // First check if memory scanner has found this address
    uintptr_t address = utils::MemoryScanner::getInstance().getAddressForPattern(name);
    
    // If not found, get from config
    if (address == 0) {
        address = utils::ConfigManager::getInstance().getAddress("GameAddresses", name, defaultValue);
    }
    
    return address;
}

// Network function addresses
// These can be dynamically updated through memory scanning
inline uintptr_t getSenderOffset() { return getAddress("SenderOffset", 0x0073D958); }
inline uintptr_t getSendOffset() { return getAddress("SendOffset", 0x00563E00); }
inline uintptr_t getSendPacketOut() { return getAddress("SendPacketOut", 0x00567FB0); }
inline uintptr_t getRecvPacketIn() { return getAddress("RecvPacketIn", 0x00467060); }

// Game data addresses
inline uintptr_t getUserNameOffset() { return getAddress("UserNameOffset", 0x0073D910); }
inline uintptr_t getOnCharacterHandler() { return getAddress("OnCharacterHandler", 0x004C1B60); }
inline uintptr_t getClickFunction() { return getAddress("ClickFunction", 0x005AA0E0); }
inline uintptr_t getClickNpcFunction() { return getAddress("ClickNpcFunction", 0x005F4430); }

// Initialize all addresses in the configuration system
inline void initializeAddresses() {
    auto& config = utils::ConfigManager::getInstance();
    
    // Network function addresses
    config.setAddress("GameAddresses", "SenderOffset", 0x0073D958);
    config.setAddress("GameAddresses", "SendOffset", 0x00563E00);
    config.setAddress("GameAddresses", "SendPacketOut", 0x00567FB0);
    config.setAddress("GameAddresses", "RecvPacketIn", 0x00467060);
    
    // Game data addresses
    config.setAddress("GameAddresses", "UserNameOffset", 0x0073D910);
    config.setAddress("GameAddresses", "OnCharacterHandler", 0x004C1B60);
    config.setAddress("GameAddresses", "ClickFunction", 0x005AA0E0);
    config.setAddress("GameAddresses", "ClickNpcFunction", 0x005F4430);
    
    // Register patterns for memory scanning
    auto& scanner = utils::MemoryScanner::getInstance();
    
    // Example pattern registration (replace with actual patterns when known)
    scanner.addPatternDefinition({
        "SendPacketOut",
        "55 8B EC 83 EC ? 56 8B 75 ? 8D 4D ? E8",  // Example pattern
        "",
        0,
        false,
        "Dark Ages function to send packets"
    });
    
    scanner.addPatternDefinition({
        "RecvPacketIn",
        "55 8B EC 83 EC ? 53 56 57 8B 7D ? 8B",  // Example pattern
        "",
        0,
        false,
        "Dark Ages function to receive packets"
    });
}

} // namespace constants
