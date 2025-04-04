/**
 * @file game_data_manager.h
 * @brief Manager class for tracking game entities and state
 */
#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include <string>

#include "player.h"

namespace game {

/**
 * @brief Thread-safe singleton manager for game data
 * 
 * Maintains collections of game entities and provides thread-safe
 * access to them. Implements the Singleton pattern to ensure
 * global access to game state.
 */
class GameDataManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static GameDataManager& getInstance();
    
    /**
     * @brief Prevent copy construction
     */
    GameDataManager(const GameDataManager&) = delete;
    
    /**
     * @brief Prevent assignment
     */
    GameDataManager& operator=(const GameDataManager&) = delete;

    /**
     * @brief Initialize the game data manager
     */
    void initialize();
    
    /**
     * @brief Reset all game data
     */
    void reset();
    
    /**
     * @brief Add or update a player in the player collection
     * @param serialId Player's unique identifier
     * @param player Player object to add or update
     */
    void updatePlayer(std::uint32_t serialId, const Player& player);
    
    /**
     * @brief Get a player by serial ID
     * @param serialId Player's unique identifier
     * @return Shared pointer to the player, or nullptr if not found
     */
    PlayerPtr getPlayer(std::uint32_t serialId);
    
    /**
     * @brief Remove a player from the collection
     * @param serialId Player's unique identifier
     * @return True if player was found and removed
     */
    bool removePlayer(std::uint32_t serialId);
    
    /**
     * @brief Get a player by name
     * @param name Player's name
     * @return Shared pointer to the player, or nullptr if not found
     */
    PlayerPtr getPlayerByName(const std::string& name);
    
    /**
     * @brief Get all players
     * @return Vector of player shared pointers
     */
    std::vector<PlayerPtr> getAllPlayers();
    
    /**
     * @brief Set the local player (the player the client controls)
     * @param serialId Serial ID of the local player
     */
    void setLocalPlayer(std::uint32_t serialId);
    
    /**
     * @brief Get the local player
     * @return Shared pointer to the local player, or nullptr if not set
     */
    PlayerPtr getLocalPlayer();
    
    /**
     * @brief Add a player name to the hostile list
     * @param playerName Name of the hostile player
     */
    void addHostile(const std::string& playerName);
    
    /**
     * @brief Remove a player name from the hostile list
     * @param playerName Name of the player to remove
     * @return True if player was in the list and was removed
     */
    bool removeHostile(const std::string& playerName);
    
    /**
     * @brief Check if a player is in the hostile list
     * @param playerName Name of the player to check
     * @return True if player is in the hostile list
     */
    bool isHostile(const std::string& playerName) const;
    
    /**
     * @brief Get the list of hostile player names
     * @return Reference to the hostile list
     */
    const std::vector<std::string>& getHostileList() const;

private:
    /**
     * @brief Private constructor to enforce singleton pattern
     */
    GameDataManager();
    
    /**
     * @brief Private destructor
     */
    ~GameDataManager() = default;

    std::unordered_map<std::uint32_t, PlayerPtr> players_;  // Map of all players by serial ID
    std::uint32_t localPlayerId_;                          // Serial ID of the local player
    std::vector<std::string> hostileList_;                 // List of hostile player names
    
    mutable std::mutex mutex_;  // Mutex for thread-safe access
};

} // namespace game
