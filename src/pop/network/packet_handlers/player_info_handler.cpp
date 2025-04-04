/**
 * @file player_info_handler.cpp
 * @brief Implementation of the PlayerInfoHandler class
 */
#include "player_info_handler.h"
#include "../../game/position.h"

namespace network::handlers {

// Packet types
constexpr uint8_t PACKET_NEW_PLAYER = 0x33;
constexpr uint8_t PACKET_PLAYER_UPDATE = 0x34;
constexpr uint8_t PACKET_PLAYER_APPEARANCE = 0x35;
constexpr uint8_t PACKET_PLAYER_POSITION = 0x15;
constexpr uint8_t PACKET_PLAYER_REMOVE = 0x36;

PlayerInfoHandler::PlayerInfoHandler()
    : BasePacketHandler("PlayerInfoHandler") {
}

bool PlayerInfoHandler::handlePacket(const Packet& packet, PacketReader& reader, bool isOutgoing) {
    // We don't process outgoing packets in this handler
    if (isOutgoing) {
        return false;
    }
    
    // Get packet type (first byte)
    if (packet.size() < 1) {
        log(utils::LogLevel::Warning, "Packet too small to contain type");
        return false;
    }
    
    uint8_t packetType = packet[0];
    
    // Skip the header byte
    reader.skip(1);
    
    try {
        switch (packetType) {
            case PACKET_NEW_PLAYER: {
                game::Player player = parseNewPlayerPacket(reader);
                getGameDataManager().updatePlayer(player.getSerialId(), player);
                log(utils::LogLevel::Info, "New player added: " + player.getName());
                return true;
            }
            
            case PACKET_PLAYER_UPDATE: {
                game::Player player = parsePlayerUpdatePacket(reader);
                getGameDataManager().updatePlayer(player.getSerialId(), player);
                log(utils::LogLevel::Debug, "Player updated: " + player.getName());
                return true;
            }
            
            case PACKET_PLAYER_APPEARANCE: {
                game::Player player = parsePlayerAppearancePacket(reader);
                getGameDataManager().updatePlayer(player.getSerialId(), player);
                log(utils::LogLevel::Debug, "Player appearance updated: " + player.getName());
                return true;
            }
            
            case PACKET_PLAYER_POSITION: {
                game::Player player = parsePlayerPositionPacket(reader);
                getGameDataManager().updatePlayer(player.getSerialId(), player);
                log(utils::LogLevel::Debug, "Player position updated: " + player.getName());
                return true;
            }
            
            case PACKET_PLAYER_REMOVE: {
                // Read player serial ID
                uint32_t serialId = reader.read<uint32_t>();
                
                // Remove player from game data manager
                auto player = getGameDataManager().getPlayer(serialId);
                if (player) {
                    log(utils::LogLevel::Info, "Player removed: " + player->getName());
                } else {
                    log(utils::LogLevel::Warning, "Unknown player removed: " + std::to_string(serialId));
                }
                
                getGameDataManager().removePlayer(serialId);
                return true;
            }
            
            default:
                // Not a packet we handle
                return false;
        }
    } catch (const std::exception& e) {
        log(utils::LogLevel::Error, "Error processing packet: " + std::string(e.what()));
        return false;
    }
}

game::Player PlayerInfoHandler::parseNewPlayerPacket(PacketReader& reader) {
    // Read player serial ID
    uint32_t serialId = reader.read<uint32_t>();
    
    // Create player object
    game::Player player(serialId);
    
    // Read player name (8-bit length prefixed string)
    std::string name = reader.readString8();
    player.setName(name);
    
    // Read player position
    uint16_t x = reader.read<uint16_t>();
    uint16_t y = reader.read<uint16_t>();
    uint8_t directionByte = reader.read<uint8_t>();
    
    // Convert direction byte to enum
    game::Direction direction = game::Direction::Unknown;
    if (directionByte < 4) {
        direction = static_cast<game::Direction>(directionByte);
    }
    
    // Set player position
    game::Position position(x, y, direction);
    player.setPosition(position);
    
    // Read appearance data
    player.setForm(reader.read<uint16_t>());
    player.setHead(reader.read<uint16_t>());
    player.setBody(reader.read<uint16_t>());
    player.setArms(reader.read<uint16_t>());
    player.setBoots(reader.read<uint16_t>());
    player.setSkinColor(reader.read<uint16_t>());
    player.setHeadColor(reader.read<uint16_t>());
    player.setBootColor(reader.read<uint16_t>());
    
    // Read accessory data
    player.setAcc1(reader.read<uint16_t>());
    player.setAcc2(reader.read<uint16_t>());
    player.setAcc3(reader.read<uint16_t>());
    player.setAcc1Color(reader.read<uint16_t>());
    player.setAcc2Color(reader.read<uint16_t>());
    
    // Read equipment data
    player.setArmor(reader.read<uint16_t>());
    player.setShield(reader.read<uint16_t>());
    player.setWeapon(reader.read<uint16_t>());
    
    // Read group name (if any)
    if (!reader.isEndOfPacket()) {
        std::string groupName = reader.readString8();
        player.setGroupName(groupName);
    }
    
    return player;
}

game::Player PlayerInfoHandler::parsePlayerUpdatePacket(PacketReader& reader) {
    // Basic update packet with minimal data
    
    // Read player serial ID
    uint32_t serialId = reader.read<uint32_t>();
    
    // Get existing player or create new one
    game::PlayerPtr existingPlayer = getGameDataManager().getPlayer(serialId);
    game::Player player = existingPlayer ? *existingPlayer : game::Player(serialId);
    
    // Read player position
    uint16_t x = reader.read<uint16_t>();
    uint16_t y = reader.read<uint16_t>();
    uint8_t directionByte = reader.read<uint8_t>();
    
    // Convert direction byte to enum
    game::Direction direction = game::Direction::Unknown;
    if (directionByte < 4) {
        direction = static_cast<game::Direction>(directionByte);
    }
    
    // Set player position
    game::Position position(x, y, direction);
    player.setPosition(position);
    
    return player;
}

game::Player PlayerInfoHandler::parsePlayerAppearancePacket(PacketReader& reader) {
    // Appearance update packet with full appearance data
    
    // Read player serial ID
    uint32_t serialId = reader.read<uint32_t>();
    
    // Get existing player or create new one
    game::PlayerPtr existingPlayer = getGameDataManager().getPlayer(serialId);
    game::Player player = existingPlayer ? *existingPlayer : game::Player(serialId);
    
    // Read appearance data
    player.setForm(reader.read<uint16_t>());
    player.setHead(reader.read<uint16_t>());
    player.setBody(reader.read<uint16_t>());
    player.setArms(reader.read<uint16_t>());
    player.setBoots(reader.read<uint16_t>());
    player.setSkinColor(reader.read<uint16_t>());
    player.setHeadColor(reader.read<uint16_t>());
    player.setBootColor(reader.read<uint16_t>());
    
    // Read accessory data
    player.setAcc1(reader.read<uint16_t>());
    player.setAcc2(reader.read<uint16_t>());
    player.setAcc3(reader.read<uint16_t>());
    player.setAcc1Color(reader.read<uint16_t>());
    player.setAcc2Color(reader.read<uint16_t>());
    
    // Read equipment data
    player.setArmor(reader.read<uint16_t>());
    player.setShield(reader.read<uint16_t>());
    player.setWeapon(reader.read<uint16_t>());
    
    // Read facial features and flags
    player.setFaceShape(reader.read<uint8_t>());
    player.setRestCloak(reader.read<uint8_t>());
    player.setHideBool(reader.read<uint8_t>());
    
    return player;
}

game::Player PlayerInfoHandler::parsePlayerPositionPacket(PacketReader& reader) {
    // Position update packet
    
    // Read player serial ID
    uint32_t serialId = reader.read<uint32_t>();
    
    // Get existing player or create new one
    game::PlayerPtr existingPlayer = getGameDataManager().getPlayer(serialId);
    game::Player player = existingPlayer ? *existingPlayer : game::Player(serialId);
    
    // Read player position
    uint16_t x = reader.read<uint16_t>();
    uint16_t y = reader.read<uint16_t>();
    uint8_t directionByte = reader.read<uint8_t>();
    
    // Convert direction byte to enum
    game::Direction direction = game::Direction::Unknown;
    if (directionByte < 4) {
        direction = static_cast<game::Direction>(directionByte);
    }
    
    // Set player position
    game::Position position(x, y, direction);
    player.setPosition(position);
    
    return player;
}

game::GameDataManager& PlayerInfoHandler::getGameDataManager() {
    return game::GameDataManager::getInstance();
}

} // namespace network::handlers
