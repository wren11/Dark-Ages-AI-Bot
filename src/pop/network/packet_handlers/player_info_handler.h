/**
 * @file player_info_handler.h
 * @brief Handler for player information packets
 */
#pragma once

#include "base_packet_handler.h"
#include "../../game/game_data_manager.h"

namespace network::handlers {

/**
 * @brief Handles packets related to player information
 * 
 * Processes packets containing player data, including appearance,
 * position, and other attributes, and updates the game data manager.
 */
class PlayerInfoHandler : public BasePacketHandler {
public:
    /**
     * @brief Constructor
     */
    PlayerInfoHandler();
    
    /**
     * @brief Destructor
     */
    ~PlayerInfoHandler() override = default;

protected:
    /**
     * @brief Handle a player information packet
     * 
     * Parses player data from the packet and updates the game data manager.
     * 
     * @param packet The packet to handle
     * @param reader Packet reader for accessing packet data
     * @param isOutgoing Whether the packet is outgoing or incoming
     * @return True if handling is successful, false otherwise
     */
    bool handlePacket(const Packet& packet, PacketReader& reader, bool isOutgoing) override;

private:
    /**
     * @brief Handle a new player packet
     * 
     * Parses a packet that introduces a new player.
     * 
     * @param reader Packet reader positioned at the player data
     * @return The parsed player object
     */
    game::Player parseNewPlayerPacket(PacketReader& reader);
    
    /**
     * @brief Handle a player update packet
     * 
     * Parses a packet that updates an existing player.
     * 
     * @param reader Packet reader positioned at the player data
     * @return The parsed player object
     */
    game::Player parsePlayerUpdatePacket(PacketReader& reader);
    
    /**
     * @brief Handle a player appearance packet
     * 
     * Parses a packet that updates a player's appearance.
     * 
     * @param reader Packet reader positioned at the player data
     * @return The parsed player object
     */
    game::Player parsePlayerAppearancePacket(PacketReader& reader);
    
    /**
     * @brief Handle a player position update packet
     * 
     * Parses a packet that updates a player's position.
     * 
     * @param reader Packet reader positioned at the player data
     * @return The parsed player object
     */
    game::Player parsePlayerPositionPacket(PacketReader& reader);
    
    /**
     * @brief Get a reference to the game data manager
     * @return Reference to the game data manager
     */
    game::GameDataManager& getGameDataManager();
};

} // namespace network::handlers
