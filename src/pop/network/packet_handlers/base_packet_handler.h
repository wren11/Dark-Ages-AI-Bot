/**
 * @file base_packet_handler.h
 * @brief Base class for packet handlers with common functionality
 */
#pragma once

#include "../packet.h"
#include "../packet_handler.h"
#include "../packet_reader.h"
#include "../../utils/logging.h"
#include <string>
#include <memory>

namespace network::handlers {

/**
 * @brief Base implementation for packet handlers
 * 
 * Provides common functionality for packet handlers, including
 * logging and error handling. Concrete handlers should derive from
 * this class and implement the handlePacket method.
 */
class BasePacketHandler : public IPacketHandler {
public:
    /**
     * @brief Constructor
     * @param handlerName Name of the handler for logging purposes
     */
    explicit BasePacketHandler(std::string handlerName);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~BasePacketHandler() = default;
    
    /**
     * @brief Process a packet
     * 
     * Base implementation performs error handling and logging.
     * 
     * @param packet The packet to process
     * @param isOutgoing Whether the packet is outgoing or incoming
     * @return True if processing is successful, false otherwise
     */
    bool processPacket(const Packet& packet, bool isOutgoing) override final;

protected:
    /**
     * @brief Implementation-specific packet handling
     * 
     * This method should be implemented by derived classes to provide
     * handler-specific logic.
     * 
     * @param packet The packet to handle
     * @param reader Packet reader for accessing packet data
     * @param isOutgoing Whether the packet is outgoing or incoming
     * @return True if handling is successful, false otherwise
     */
    virtual bool handlePacket(const Packet& packet, PacketReader& reader, bool isOutgoing) = 0;
    
    /**
     * @brief Log a message with the handler name prefix
     * @param level Log level
     * @param message Message to log
     */
    void log(utils::LogLevel level, const std::string& message) const;
    
    /**
     * @brief Get the handler name
     * @return Handler name
     */
    [[nodiscard]] const std::string& getHandlerName() const;

private:
    std::string handlerName_;  // Name of this handler for logging
};

/**
 * @brief Shared pointer for packet handlers
 */
using PacketHandlerPtr = std::shared_ptr<IPacketHandler>;

} // namespace network::handlers
