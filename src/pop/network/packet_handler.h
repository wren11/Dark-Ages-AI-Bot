/**
 * @file packet_handler.h
 * @brief Defines the base packet handler interfaces and classes
 */
#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <memory>
#include "../core/interceptor.h"
#include "packet.h"

namespace network {

/**
 * @brief Interface for specific packet opcode handlers
 *
 * Following the Interface Segregation Principle, this interface
 * is focused only on handling a single packet opcode.
 */
class IOpcodeHandler {
public:
    virtual ~IOpcodeHandler() = default;
    
    /**
     * @brief Handle a packet with a specific opcode
     * @param packet The packet to handle
     * @return True if handling was successful, false otherwise
     */
    virtual bool handlePacket(const Packet& packet) = 0;
};

/**
 * @brief Base class for packet direction-specific handlers
 * 
 * Abstract base class that implements the core::IPacketHandler interface
 * with support for registering opcode-specific handlers.
 */
template <typename OpcodeType = std::uint8_t>
class BasePacketHandler : public core::IPacketHandler {
public:
    using OpcodeHandlerFunc = std::function<bool(const Packet&)>;
    
    BasePacketHandler() = default;
    virtual ~BasePacketHandler() = default;
    
    /**
     * @brief Register a handler for a specific opcode
     * @param opcode The packet opcode
     * @param handler Function to handle the packet
     */
    void registerHandler(OpcodeType opcode, OpcodeHandlerFunc handler) {
        handlers_[opcode] = handler;
    }
    
    /**
     * @brief Register a handler class for a specific opcode
     * @param opcode The packet opcode
     * @param handler Shared pointer to handler object
     */
    void registerHandler(OpcodeType opcode, std::shared_ptr<IOpcodeHandler> handler) {
        if (handler) {
            handlers_[opcode] = [handler](const Packet& packet) {
                return handler->handlePacket(packet);
            };
        }
    }
    
protected:
    /**
     * @brief Process a packet with registered handlers
     * @param packet The packet to process
     * @return True if a handler was found and executed successfully
     */
    bool processPacket(const Packet& packet) {
        // Ensure packet has at least an opcode
        if (packet.size() < 1) {
            return false;
        }
        
        // Extract the opcode (first byte of packet)
        OpcodeType opcode = static_cast<OpcodeType>(packet[0]);
        
        // Find and execute handler for this opcode
        auto it = handlers_.find(opcode);
        if (it != handlers_.end()) {
            try {
                return it->second(packet);
            }
            catch (const std::exception& e) {
                // Log error but continue processing
                return false;
            }
        }
        
        // No handler found for this opcode
        return false;
    }
    
private:
    std::unordered_map<OpcodeType, OpcodeHandlerFunc> handlers_;
};

/**
 * @brief Handler for outgoing packets from client to server
 */
class OutgoingPacketHandler : public BasePacketHandler<std::uint8_t> {
public:
    /**
     * @brief Handle an outgoing packet
     * @param packet The packet being sent
     * @return True if handling was successful
     */
    bool handleOutgoingPacket(const Packet& packet) override {
        return processPacket(packet);
    }
    
    /**
     * @brief Not used for outgoing handlers
     */
    bool handleIncomingPacket(const Packet& packet) override {
        // Outgoing handler doesn't process incoming packets
        return false;
    }
};

/**
 * @brief Handler for incoming packets from server to client
 */
class IncomingPacketHandler : public BasePacketHandler<std::uint8_t> {
public:
    /**
     * @brief Not used for incoming handlers
     */
    bool handleOutgoingPacket(const Packet& packet) override {
        // Incoming handler doesn't process outgoing packets
        return false;
    }
    
    /**
     * @brief Handle an incoming packet
     * @param packet The packet being received
     * @return True if handling was successful
     */
    bool handleIncomingPacket(const Packet& packet) override {
        return processPacket(packet);
    }
};

} // namespace network
