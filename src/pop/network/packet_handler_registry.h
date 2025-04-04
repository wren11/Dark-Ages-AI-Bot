/**
 * @file packet_handler_registry.h
 * @brief Registry for packet handlers
 */
#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <atomic>
#include <chrono>
#include "packet.h"
#include "packet_reader.h"
#include "packet_writer.h"
#include "packet_handler.h"
#include "packet_handlers/base_packet_handler.h"

namespace network {

/**
 * @brief Registry for packet handlers
 * 
 * Manages a collection of packet handlers and routes packets
 * to the appropriate handlers based on packet type.
 * Thread-safe singleton pattern implementation.
 */
class PacketHandlerRegistry {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static PacketHandlerRegistry& getInstance();
    
    /**
     * @brief Prevent copy construction
     */
    PacketHandlerRegistry(const PacketHandlerRegistry&) = delete;
    
    /**
     * @brief Prevent assignment
     */
    PacketHandlerRegistry& operator=(const PacketHandlerRegistry&) = delete;

    /**
     * @brief Initialize the registry with default handlers
     */
    void initialize();
    
    /**
     * @brief Register a handler for a specific packet type
     * @param packetType Type of packet to register handler for
     * @param handler Handler to register
     * @param isOutgoing Whether this handler should process outgoing packets
     */
    void registerHandler(std::uint8_t packetType, handlers::PacketHandlerPtr handler, bool isOutgoing);
    
    /**
     * @brief Register a general handler that processes all packets
     * @param handler Handler to register
     * @param isOutgoing Whether this handler should process outgoing packets
     */
    void registerGeneralHandler(handlers::PacketHandlerPtr handler, bool isOutgoing);
    
    /**
     * @brief Unregister a handler for a specific packet type
     * @param packetType Type of packet to unregister handler for
     * @param isOutgoing Whether to unregister from outgoing or incoming handlers
     * @return True if a handler was found and unregistered
     */
    bool unregisterHandler(std::uint8_t packetType, bool isOutgoing);
    
    /**
     * @brief Process a packet through registered handlers
     * @param packet Packet to process
     * @param isOutgoing Whether this is an outgoing packet
     * @return True if any handler processed the packet
     */
    bool processPacket(const Packet& packet, bool isOutgoing);
    
    /**
     * @brief Process a packet with a specific handler type
     * @tparam HandlerType Type of handler to use for processing
     * @param packet Packet to process
     * @param isOutgoing Whether this is an outgoing packet
     * @return True if the handler processed the packet
     */
    template <typename HandlerType>
    bool processWithHandler(const Packet& packet, bool isOutgoing) {
        auto handler = std::make_shared<HandlerType>();
        return handler->processPacket(packet, isOutgoing);
    }
    
    /**
     * @brief Register a name for a packet type
     * @param packetType The packet type to register a name for
     * @param name The name to associate with the packet type
     * @param isOutgoing Whether this is an outgoing packet type
     */
    void registerPacketName(std::uint8_t packetType, const std::string& name, bool isOutgoing);
    
    /**
     * @brief Get the name of a packet type
     * @param packetType The packet type to get the name for
     * @param isOutgoing Whether this is an outgoing packet type
     * @return The name of the packet type, or "Unknown" if not registered
     */
    std::string getPacketName(std::uint8_t packetType, bool isOutgoing) const;
    
    /**
     * @brief Get statistics for a packet type
     * @param packetType The packet type to get statistics for
     * @param isOutgoing Whether this is an outgoing packet type
     * @return A string containing statistics for the packet type
     */
    std::string getPacketStats(std::uint8_t packetType, bool isOutgoing) const;
    
    /**
     * @brief Reset packet statistics
     */
    void resetStats();
    
    /**
     * @brief Create a packet reader for a packet
     * @param packet The packet to create a reader for
     * @return A reader for the packet
     */
    PacketReader createReader(const Packet& packet) const;
    
    /**
     * @brief Create a packet writer with a specific packet type
     * @param packetType The packet type to use
     * @param initialCapacity The initial capacity of the writer
     * @return A writer initialized with the packet type
     */
    PacketWriter createWriter(std::uint8_t packetType, std::size_t initialCapacity = 256) const;

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    PacketHandlerRegistry();
    
    /**
     * @brief Private destructor
     */
    ~PacketHandlerRegistry() = default;

    /**
     * @brief Process packet with type-specific handlers
     * @param packet Packet to process
     * @param packetType Type of the packet
     * @param isOutgoing Whether this is an outgoing packet
     * @return True if any handler processed the packet
     */
    bool processSpecificHandlers(const Packet& packet, std::uint8_t packetType, bool isOutgoing);
    
    /**
     * @brief Process packet with general handlers
     * @param packet Packet to process
     * @param isOutgoing Whether this is an outgoing packet
     * @return True if any handler processed the packet
     */
    bool processGeneralHandlers(const Packet& packet, bool isOutgoing);
    
    /**
     * @brief Record packet statistics
     * @param packetType The packet type to record statistics for
     * @param packetSize The size of the packet
     * @param isOutgoing Whether this is an outgoing packet
     */
    void recordPacketStats(std::uint8_t packetType, std::size_t packetSize, bool isOutgoing);

    // Maps packet types to their handlers
    std::unordered_map<std::uint8_t, std::vector<handlers::PacketHandlerPtr>> incomingHandlers_;
    std::unordered_map<std::uint8_t, std::vector<handlers::PacketHandlerPtr>> outgoingHandlers_;
    
    // General handlers that process all packets
    std::vector<handlers::PacketHandlerPtr> generalIncomingHandlers_;
    std::vector<handlers::PacketHandlerPtr> generalOutgoingHandlers_;
    
    // Maps packet types to their names
    std::unordered_map<std::uint8_t, std::string> incomingPacketNames_;
    std::unordered_map<std::uint8_t, std::string> outgoingPacketNames_;
    
    // Packet statistics
    struct PacketStats {
        std::atomic<std::uint64_t> count{0};
        std::atomic<std::uint64_t> totalSize{0};
        std::chrono::steady_clock::time_point lastSeen;
    };
    
    std::unordered_map<std::uint8_t, PacketStats> incomingStats_;
    std::unordered_map<std::uint8_t, PacketStats> outgoingStats_;
    
    // Mutex for thread safety
    mutable std::mutex mutex_;
};

} // namespace network
