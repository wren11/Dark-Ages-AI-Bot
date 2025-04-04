/**
 * @file migration_helper.h
 * @brief Utility functions to help migrate from old packet structures to new ones
 * 
 * This file provides conversion functions between old and new packet types
 * to ease the transition during refactoring. This file should be removed
 * once migration is complete.
 */
#pragma once

#include "../pch.h"
#include "../packet_structures.h"
#include "../network/packet.h"
#include "../network/packet_reader.h"
#include "../network/packet_writer.h"
#include "../utils/logging.h"

namespace utils {

/**
 * @brief Migration utilities for transitioning between old and new packet systems
 */
class MigrationHelper {
public:
    /**
     * @brief Convert old packet structure to new network::Packet
     * @param oldPacket Legacy packet structure
     * @return Equivalent network::Packet
     */
    static network::Packet convertToNewPacket(const packet& oldPacket) {
        return network::Packet(oldPacket.data, oldPacket.length);
    }
    
    /**
     * @brief Convert new network::Packet to old packet structure
     * @param newPacket New network::Packet
     * @return Equivalent legacy packet structure
     */
    static packet convertToOldPacket(const network::Packet& newPacket) {
        packet oldPacket;
        oldPacket.length = static_cast<DWORD>(newPacket.size());
        
        // Copy data up to the maximum size of the old packet structure
        const size_t copySize = std::min(oldPacket.length, static_cast<DWORD>(sizeof(oldPacket.data)));
        std::memcpy(oldPacket.data, newPacket.data(), copySize);
        
        return oldPacket;
    }
    
    /**
     * @brief Create a new PacketReader from an old packet
     * @param oldPacket Legacy packet structure
     * @return Configured network::PacketReader
     */
    static network::PacketReader createReaderFromOldPacket(const packet& oldPacket) {
        return network::PacketReader(convertToNewPacket(oldPacket));
    }
    
    /**
     * @brief Wrapper to call new packet handlers from old handler functions
     * @param oldHandler Old style handler function
     * @param newHandler New style handler function
     * @return Function that bridges old and new systems
     */
    static std::function<void(const packet&)> createHandlerWrapper(
        std::function<void(const network::Packet&)> newHandler) {
        
        return [newHandler](const packet& oldPacket) {
            try {
                network::Packet newPacket = convertToNewPacket(oldPacket);
                newHandler(newPacket);
            }
            catch (const std::exception& e) {
                Logging::error("Error in packet handler wrapper: " + std::string(e.what()));
            }
        };
    }
    
    /**
     * @brief Register both old and new handler for a packet type
     * @param packetType Type of packet to register handler for
     * @param newHandler New style handler function
     * @param isOutgoing Whether this is for outgoing packets
     */
    static void registerDualHandlers(
        std::uint8_t packetType,
        std::function<void(const network::Packet&)> newHandler,
        bool isOutgoing) {
        
        // Register with new system directly
        network::handlers::PacketHandlerPtr handler = 
            std::make_shared<network::handlers::LambdaPacketHandler>(newHandler);
        network::PacketHandlerRegistry::getInstance().registerHandler(
            packetType, handler, isOutgoing);
        
        // TODO: Also register with old system during transition
        // This part depends on how the old system registers handlers
    }
};

} // namespace utils
