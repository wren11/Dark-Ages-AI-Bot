/**
 * @file chat_message_handler.cpp
 * @brief Implementation of the chat message packet handler
 */
#include "chat_message_handler.h"
#include "../packet_reader.h"
#include "../../utils/config/config_manager.h"
#include "../../utils/logging.h"
#include <regex>

namespace network {
namespace handlers {

ChatMessageHandler::ChatMessageHandler() {
    utils::Logging::info("Chat message handler initialized");
}

bool ChatMessageHandler::processPacket(const Packet& packet, bool isOutgoing) {
    try {
        if (isOutgoing) {
            return processOutgoingChatMessage(packet);
        } else {
            return processIncomingChatMessage(packet);
        }
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error processing chat message: " + std::string(e.what()));
        return false;
    }
}

std::string ChatMessageHandler::getName() const {
    return "ChatMessageHandler";
}

bool ChatMessageHandler::processIncomingChatMessage(const Packet& packet) {
    PacketReader reader(packet);
    
    // Skip packet type byte
    reader.skip(1);
    
    // Read message type (public, private, etc.)
    uint8_t messageType = reader.readByte();
    
    // Read sender name
    std::string senderName = reader.readString();
    
    // Read message content
    std::string message = reader.readString();
    
    // Log the chat message
    utils::Logging::info("Chat from " + senderName + ": " + message);
    
    // Check if this is a command directed at the bot
    bool isBotCommand = parseChatCommand(message, senderName);
    
    // If this is a bot command, mark as processed so other handlers don't get it
    return isBotCommand;
}

bool ChatMessageHandler::processOutgoingChatMessage(const Packet& packet) {
    PacketReader reader(packet);
    
    // Skip packet type byte
    reader.skip(1);
    
    // Read message content
    std::string message = reader.readString();
    
    // Log the outgoing message
    utils::Logging::info("Sending chat: " + message);
    
    // We don't modify outgoing chat messages, so just pass them through
    return false;
}

std::string ChatMessageHandler::extractPlayerName(const std::string& message) {
    // Look for command patterns like "@PlayerName action"
    std::regex commandPattern(R"(@(\w+)\s+.*)");
    std::smatch matches;
    
    if (std::regex_search(message, matches, commandPattern) && matches.size() > 1) {
        return matches[1].str();
    }
    
    return "";
}

bool ChatMessageHandler::parseChatCommand(const std::string& message, const std::string& playerName) {
    // Get bot owner name from config
    std::string botOwner = utils::ConfigManager::getInstance().getString("Bot", "OwnerName", "");
    
    // Only process commands from the bot owner
    if (!botOwner.empty() && playerName != botOwner) {
        return false;
    }
    
    // Check if message starts with '!' (command prefix)
    if (message.empty() || message[0] != '!') {
        return false;
    }
    
    // Extract command and arguments
    std::string command;
    std::string args;
    
    size_t spacePos = message.find(' ');
    if (spacePos != std::string::npos) {
        command = message.substr(1, spacePos - 1);
        args = message.substr(spacePos + 1);
    } else {
        command = message.substr(1);
        args = "";
    }
    
    // Convert command to lowercase for case-insensitive matching
    std::transform(command.begin(), command.end(), command.begin(), 
                 [](unsigned char c){ return std::tolower(c); });
    
    // Process commands
    if (command == "help") {
        // Show available commands
        // TODO: Implement a way to send chat messages back to the player
        utils::Logging::info("Processed help command from " + playerName);
        return true;
    }
    else if (command == "status") {
        // Show bot status
        utils::Logging::info("Processed status command from " + playerName);
        return true;
    }
    else if (command == "scan") {
        // Trigger memory scanning
        utils::Logging::info("Processed scan command from " + playerName);
        
        // Example: Queue a memory scan task in a separate thread
        // utils::MemoryScanner::getInstance().scanForPatterns();
        return true;
    }
    else if (command == "reload") {
        // Reload configuration
        utils::Logging::info("Processed reload command from " + playerName);
        utils::ConfigManager::getInstance().reload();
        
        // Also reload game data
        game::DataLoader::getInstance().loadAllData();
        return true;
    }
    
    // Command not recognized
    return false;
}

} // namespace handlers
} // namespace network
