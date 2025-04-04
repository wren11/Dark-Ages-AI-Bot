/**
 * @file packet_handler_registry.cpp
 * @brief Implementation of the PacketHandlerRegistry class
 */
#include "packet_handler_registry.h"
#include "packet_handlers/player_info_handler.h"
#include "../utils/logging.h"
#include <sstream>
#include <iomanip>

namespace network {

PacketHandlerRegistry& PacketHandlerRegistry::getInstance() {
    static PacketHandlerRegistry instance;
    return instance;
}

PacketHandlerRegistry::PacketHandlerRegistry()
    : incomingHandlers_(), outgoingHandlers_(),
      generalIncomingHandlers_(), generalOutgoingHandlers_(),
      incomingPacketNames_(), outgoingPacketNames_(),
      incomingStats_(), outgoingStats_() {
}

void PacketHandlerRegistry::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    utils::Logging::info("Initializing PacketHandlerRegistry");
    
    // Clear existing handlers
    incomingHandlers_.clear();
    outgoingHandlers_.clear();
    generalIncomingHandlers_.clear();
    generalOutgoingHandlers_.clear();
    
    // Clear packet names and stats
    incomingPacketNames_.clear();
    outgoingPacketNames_.clear();
    incomingStats_.clear();
    outgoingStats_.clear();
    
    // Register default handlers
    auto playerInfoHandler = std::make_shared<handlers::PlayerInfoHandler>();
    
    // These packet types are related to player information
    registerHandler(0x33, playerInfoHandler, false); // New player
    registerHandler(0x34, playerInfoHandler, false); // Player update
    registerHandler(0x35, playerInfoHandler, false); // Player appearance
    registerHandler(0x15, playerInfoHandler, false); // Player position
    registerHandler(0x36, playerInfoHandler, false); // Player remove
    
    // Register common packet names
    registerPacketName(0x33, "NewPlayer", false);
    registerPacketName(0x34, "PlayerUpdate", false);
    registerPacketName(0x35, "PlayerAppearance", false);
    registerPacketName(0x15, "PlayerPosition", false);
    registerPacketName(0x36, "PlayerRemove", false);
    
    registerPacketName(0x02, "Login", true);
    registerPacketName(0x10, "Chat", true);
    registerPacketName(0x15, "Move", true);
    registerPacketName(0x17, "Attack", true);
    registerPacketName(0x19, "SpellCast", true);
    
    utils::Logging::info("PacketHandlerRegistry initialized with default handlers and packet names");
}

void PacketHandlerRegistry::registerHandler(std::uint8_t packetType, handlers::PacketHandlerPtr handler, bool isOutgoing) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOutgoing) {
        outgoingHandlers_[packetType].push_back(handler);
        utils::Logging::debug("Registered outgoing handler for packet type 0x" + 
                            utils::Logging::hexString(packetType) + 
                            " (" + getPacketName(packetType, true) + ")");
    } else {
        incomingHandlers_[packetType].push_back(handler);
        utils::Logging::debug("Registered incoming handler for packet type 0x" + 
                            utils::Logging::hexString(packetType) + 
                            " (" + getPacketName(packetType, false) + ")");
    }
}

void PacketHandlerRegistry::registerGeneralHandler(handlers::PacketHandlerPtr handler, bool isOutgoing) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOutgoing) {
        generalOutgoingHandlers_.push_back(handler);
        utils::Logging::debug("Registered general outgoing handler");
    } else {
        generalIncomingHandlers_.push_back(handler);
        utils::Logging::debug("Registered general incoming handler");
    }
}

bool PacketHandlerRegistry::unregisterHandler(std::uint8_t packetType, bool isOutgoing) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOutgoing) {
        auto it = outgoingHandlers_.find(packetType);
        if (it != outgoingHandlers_.end()) {
            outgoingHandlers_.erase(it);
            utils::Logging::debug("Unregistered outgoing handler for packet type 0x" + 
                                utils::Logging::hexString(packetType) + 
                                " (" + getPacketName(packetType, true) + ")");
            return true;
        }
    } else {
        auto it = incomingHandlers_.find(packetType);
        if (it != incomingHandlers_.end()) {
            incomingHandlers_.erase(it);
            utils::Logging::debug("Unregistered incoming handler for packet type 0x" + 
                                utils::Logging::hexString(packetType) + 
                                " (" + getPacketName(packetType, false) + ")");
            return true;
        }
    }
    
    return false;
}

void PacketHandlerRegistry::registerPacketName(std::uint8_t packetType, const std::string& name, bool isOutgoing) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isOutgoing) {
        outgoingPacketNames_[packetType] = name;
    } else {
        incomingPacketNames_[packetType] = name;
    }
}

std::string PacketHandlerRegistry::getPacketName(std::uint8_t packetType, bool isOutgoing) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const auto& packetNames = isOutgoing ? outgoingPacketNames_ : incomingPacketNames_;
    auto it = packetNames.find(packetType);
    
    if (it != packetNames.end()) {
        return it->second;
    }
    
    // Return a default name if not found
    std::stringstream ss;
    ss << "Unknown_0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
       << static_cast<int>(packetType);
    return ss.str();
}

std::string PacketHandlerRegistry::getPacketStats(std::uint8_t packetType, bool isOutgoing) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    const auto& stats = isOutgoing ? outgoingStats_ : incomingStats_;
    auto it = stats.find(packetType);
    
    std::stringstream ss;
    ss << "Packet Type: 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
       << static_cast<int>(packetType) << " (" << getPacketName(packetType, isOutgoing) << ")\n";
       
    if (it != stats.end()) {
        ss << "Count: " << std::dec << it->second.count << "\n";
        ss << "Total Size: " << it->second.totalSize << " bytes\n";
        
        if (it->second.count > 0) {
            double avgSize = static_cast<double>(it->second.totalSize) / it->second.count;
            ss << "Average Size: " << std::fixed << std::setprecision(2) << avgSize << " bytes\n";
        }
        
        // Calculate time since last seen
        auto now = std::chrono::steady_clock::now();
        auto lastSeen = it->second.lastSeen;
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastSeen).count();
        
        if (duration < 60) {
            ss << "Last Seen: " << duration << " seconds ago\n";
        } else if (duration < 3600) {
            ss << "Last Seen: " << (duration / 60) << " minutes ago\n";
        } else {
            ss << "Last Seen: " << (duration / 3600) << " hours ago\n";
        }
    } else {
        ss << "No statistics available for this packet type.\n";
    }
    
    return ss.str();
}

void PacketHandlerRegistry::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    incomingStats_.clear();
    outgoingStats_.clear();
    
    utils::Logging::info("Packet statistics reset");
}

PacketReader PacketHandlerRegistry::createReader(const Packet& packet) const {
    return PacketReader(packet);
}

PacketWriter PacketHandlerRegistry::createWriter(std::uint8_t packetType, std::size_t initialCapacity) const {
    return PacketWriter(packetType, initialCapacity);
}

bool PacketHandlerRegistry::processPacket(const Packet& packet, bool isOutgoing) {
    if (packet.size() < 1) {
        utils::Logging::warning("Received empty packet");
        return false;
    }
    
    std::uint8_t packetType = packet[0];
    
    // Record statistics for this packet
    recordPacketStats(packetType, packet.size(), isOutgoing);
    
    // Log packet information at trace level
    utils::Logging::trace((isOutgoing ? "Outgoing" : "Incoming") + 
                         std::string(" packet: 0x") + 
                         utils::Logging::hexString(packetType) + 
                         " (" + getPacketName(packetType, isOutgoing) + "), " +
                         std::to_string(packet.size()) + " bytes");
    
    // Process packet with type-specific handlers first
    bool processed = processSpecificHandlers(packet, packetType, isOutgoing);
    
    // Then process with general handlers
    bool generalProcessed = processGeneralHandlers(packet, isOutgoing);
    
    // Return true if either specific or general handlers processed the packet
    return processed || generalProcessed;
}

bool PacketHandlerRegistry::processSpecificHandlers(
    const Packet& packet, std::uint8_t packetType, bool isOutgoing) {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Get the appropriate handler map
    const auto& handlers = isOutgoing ? outgoingHandlers_ : incomingHandlers_;
    
    // Find handlers for this packet type
    auto it = handlers.find(packetType);
    if (it == handlers.end()) {
        // No specific handlers for this packet type
        return false;
    }
    
    bool processed = false;
    
    // Process packet with all registered handlers for this type
    for (const auto& handler : it->second) {
        try {
            if (handler->processPacket(packet, isOutgoing)) {
                processed = true;
            }
        } catch (const std::exception& e) {
            utils::Logging::error("Exception in packet handler for type 0x" + 
                                 utils::Logging::hexString(packetType) + 
                                 " (" + getPacketName(packetType, isOutgoing) + "): " + 
                                 std::string(e.what()));
        } catch (...) {
            utils::Logging::error("Unknown exception in packet handler for type 0x" + 
                                 utils::Logging::hexString(packetType) + 
                                 " (" + getPacketName(packetType, isOutgoing) + ")");
        }
    }
    
    return processed;
}

bool PacketHandlerRegistry::processGeneralHandlers(const Packet& packet, bool isOutgoing) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Get the appropriate handler list
    const auto& handlers = isOutgoing ? generalOutgoingHandlers_ : generalIncomingHandlers_;
    
    bool processed = false;
    
    // Process packet with all general handlers
    for (const auto& handler : handlers) {
        try {
            if (handler->processPacket(packet, isOutgoing)) {
                processed = true;
            }
        } catch (const std::exception& e) {
            utils::Logging::error("Exception in general packet handler: " + std::string(e.what()));
        } catch (...) {
            utils::Logging::error("Unknown exception in general packet handler");
        }
    }
    
    return processed;
}

void PacketHandlerRegistry::recordPacketStats(std::uint8_t packetType, std::size_t packetSize, bool isOutgoing) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto& stats = isOutgoing ? outgoingStats_[packetType] : incomingStats_[packetType];
    
    // Update statistics
    stats.count++;
    stats.totalSize += packetSize;
    stats.lastSeen = std::chrono::steady_clock::now();
}

} // namespace network
