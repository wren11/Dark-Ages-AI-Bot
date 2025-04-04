/**
 * @file chat_message_handler.h
 * @brief Handler for chat message packets
 */
#pragma once

#include "../packet_handler.h"
#include "../../game/data_loader.h"
#include "../../utils/logging.h"
#include <string>
#include <memory>

namespace network {
namespace handlers {

/**
 * @brief Handles chat message packets
 * 
 * Processes incoming chat messages from the server and outgoing
 * chat messages from the client.
 */
class ChatMessageHandler : public PacketHandler {
public:
    /**
     * @brief Constructor
     */
    ChatMessageHandler();
    
    /**
     * @brief Process a chat message packet
     * @param packet The packet to process
     * @param isOutgoing Whether the packet is outgoing
     * @return True if packet was processed
     */
    bool processPacket(const Packet& packet, bool isOutgoing) override;
    
    /**
     * @brief Get handler name
     * @return Name of this handler
     */
    std::string getName() const override;
    
private:
    /**
     * @brief Process an incoming chat message
     * @param packet The packet to process
     * @return True if packet was processed
     */
    bool processIncomingChatMessage(const Packet& packet);
    
    /**
     * @brief Process an outgoing chat message
     * @param packet The packet to process
     * @return True if packet was processed
     */
    bool processOutgoingChatMessage(const Packet& packet);
    
    /**
     * @brief Extract player name from a chat message
     * @param message The chat message
     * @return Player name, or empty string if not found
     */
    std::string extractPlayerName(const std::string& message);
    
    /**
     * @brief Parse chat commands
     * @param message The chat message
     * @param playerName The player name
     * @return True if a command was processed
     */
    bool parseChatCommand(const std::string& message, const std::string& playerName);
};

/**
 * @brief Create a shared pointer to a ChatMessageHandler
 * @return Shared pointer to a new ChatMessageHandler instance
 */
inline PacketHandlerPtr createChatMessageHandler() {
    return std::make_shared<ChatMessageHandler>();
}

} // namespace handlers
} // namespace network
