/**
 * @file base_packet_handler.cpp
 * @brief Implementation of the BasePacketHandler class
 */
#include "base_packet_handler.h"

namespace network::handlers {

BasePacketHandler::BasePacketHandler(std::string handlerName)
    : handlerName_(std::move(handlerName)) {
}

bool BasePacketHandler::processPacket(const Packet& packet, bool isOutgoing) {
    try {
        // Create a reader for the packet
        PacketReader reader(packet);
        
        // Call the derived class implementation
        return handlePacket(packet, reader, isOutgoing);
    } catch (const std::exception& e) {
        // Log any exceptions during packet processing
        log(utils::LogLevel::Error, "Exception while processing packet: " + std::string(e.what()));
        return false;
    } catch (...) {
        // Catch any other exceptions
        log(utils::LogLevel::Error, "Unknown exception while processing packet");
        return false;
    }
}

void BasePacketHandler::log(utils::LogLevel level, const std::string& message) const {
    // Prefix all logs with the handler name
    utils::Logging::log(level, "[" + handlerName_ + "] " + message);
}

const std::string& BasePacketHandler::getHandlerName() const {
    return handlerName_;
}

} // namespace network::handlers
