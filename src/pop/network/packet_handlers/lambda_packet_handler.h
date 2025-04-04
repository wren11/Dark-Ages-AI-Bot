/**
 * @file lambda_packet_handler.h
 * @brief Lambda-based packet handler implementation
 */
#pragma once

#include "../packet_handler.h"
#include "../../utils/logging.h"
#include <functional>

namespace network {
namespace handlers {

/**
 * @brief Packet handler implementation using lambda functions
 * 
 * Provides a convenient way to create packet handlers from lambda
 * functions without having to create a new class for each handler.
 */
class LambdaPacketHandler : public PacketHandler {
public:
    /**
     * @brief Constructor with handler function
     * @param handlerFunc Function to call when processing packets
     */
    explicit LambdaPacketHandler(std::function<void(const Packet&)> handlerFunc)
        : handlerFunc_(handlerFunc) {}
        
    /**
     * @brief Process a packet using the stored lambda
     * @param packet Packet to process
     * @param isOutgoing Whether the packet is outgoing
     * @return True if packet was processed
     */
    bool processPacket(const Packet& packet, bool isOutgoing) override {
        try {
            if (handlerFunc_) {
                handlerFunc_(packet);
                return true;
            }
        }
        catch (const std::exception& e) {
            utils::Logging::error("Error in lambda packet handler: " + std::string(e.what()));
        }
        return false;
    }
    
    /**
     * @brief Get the name of this handler
     * @return Handler name
     */
    std::string getName() const override {
        return "LambdaPacketHandler";
    }

private:
    std::function<void(const Packet&)> handlerFunc_;
};

} // namespace handlers
} // namespace network
