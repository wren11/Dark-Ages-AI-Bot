/**
 * @file initialization.h
 * @brief Centralized initialization system for the bot
 */
#pragma once

#include "constants.h"
#include "network/packet_handler_registry.h"
#include "network/packet_handlers/chat_message_handler.h"
#include "utils/config/config_manager.h"
#include "utils/memory/memory_scanner.h"
#include "utils/memory/memory_manager.h"
#include "game/data_loader.h"
#include "utils/logging.h"
#include <string>
#include <filesystem>

namespace pop {

/**
 * @brief Initialize all bot subsystems
 * @param configPath Path to configuration directory
 * @param dataPath Path to game data directory
 * @return True if initialization was successful
 */
inline bool initialize(const std::string& configPath = "config", 
                      const std::string& dataPath = "data") {
    try {
        utils::Logging::info("Initializing Dark Ages AI Bot...");
        
        // Initialize configuration system
        if (!utils::ConfigManager::getInstance().initialize(configPath)) {
            utils::Logging::error("Failed to initialize configuration system");
            return false;
        }
        
        // Initialize memory scanning and management
        utils::MemoryScanner::getInstance().initialize();
        
        // Register memory patterns and initialize addresses
        constants::initializeAddresses();
        
        // Initialize game data
        if (!game::DataLoader::getInstance().initialize(dataPath)) {
            utils::Logging::warning("Failed to initialize game data, continuing with defaults");
        }
        
        // Register packet handlers
        auto& registry = network::PacketHandlerRegistry::getInstance();
        
        // Register chat message handler
        registry.registerHandler(0x0C, network::handlers::createChatMessageHandler(), false);
        
        utils::Logging::info("Initialization complete");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error during initialization: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief Auto-scan for memory patterns
 * @param scanType Type of scan to perform (0=quick, 1=thorough, 2=full)
 */
inline void performMemoryScan(int scanType = 0) {
    utils::Logging::info("Starting memory scan...");
    
    auto& scanner = utils::MemoryScanner::getInstance();
    
    if (scanType == 0) {
        // Quick scan - only look for critical patterns
        scanner.quickScan();
    }
    else if (scanType == 1) {
        // Thorough scan - look for all registered patterns
        scanner.scanAllPatterns();
    }
    else {
        // Full scan - search all memory and look for potential patterns
        scanner.deepScan();
    }
    
    // Log results
    int foundCount = scanner.getFoundPatternCount();
    int totalCount = scanner.getTotalPatternCount();
    
    utils::Logging::info("Memory scan complete. Found " + 
                       std::to_string(foundCount) + " out of " + 
                       std::to_string(totalCount) + " patterns.");
                       
    // Update configuration with found addresses
    if (foundCount > 0) {
        utils::ConfigManager::getInstance().save();
    }
}

/**
 * @brief Cleanup all bot subsystems
 */
inline void cleanup() {
    utils::Logging::info("Cleaning up...");
    
    // Make sure configuration changes are saved
    utils::ConfigManager::getInstance().save();
    
    utils::Logging::info("Cleanup complete");
}

} // namespace pop
