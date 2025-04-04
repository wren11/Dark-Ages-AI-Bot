/**
 * @file sprite_manager.h
 * @brief Manages game sprites and provides rendering functionality
 */
#pragma once

#include "sprite.h"
#include "../network/packet.h"

#include <unordered_map>
#include <mutex>
#include <vector>
#include <functional>
#include <memory>

namespace game {

/**
 * @brief Manages all sprites in the game world
 * 
 * This class handles the creation, updating, and removal of sprites
 * as well as providing access to sprites by ID.
 */
class SpriteManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static SpriteManager& getInstance();
    
    /**
     * @brief Process a sprite packet from the server
     * @param packet Packet containing sprite information
     */
    void processSpritePacket(const network::Packet& packet);
    
    /**
     * @brief Add a sprite to the manager
     * @param sprite Sprite to add
     */
    void addSprite(SpritePtr sprite);
    
    /**
     * @brief Remove a sprite by ID
     * @param spriteId ID of sprite to remove
     * @return True if sprite was removed
     */
    bool removeSprite(std::uint32_t spriteId);
    
    /**
     * @brief Get a sprite by ID
     * @param spriteId ID of sprite to get
     * @return Shared pointer to sprite or nullptr if not found
     */
    SpritePtr getSprite(std::uint32_t spriteId);
    
    /**
     * @brief Get all sprites
     * @return Vector of all sprites
     */
    std::vector<SpritePtr> getAllSprites();
    
    /**
     * @brief Get sprites within a radius of a point
     * @param x X-coordinate of center point
     * @param y Y-coordinate of center point
     * @param radius Radius to search within
     * @return Vector of sprites within radius
     */
    std::vector<SpritePtr> getSpritesInRadius(std::uint16_t x, std::uint16_t y, float radius);
    
    /**
     * @brief Get sprites of a specific type
     * @param type Type of sprites to get
     * @return Vector of sprites of the specified type
     */
    std::vector<SpritePtr> getSpritesByType(NpcType type);
    
    /**
     * @brief Register a callback for sprite added events
     * @param callback Function to call when a sprite is added
     * @return ID of the registered callback
     */
    int registerSpriteAddedCallback(std::function<void(SpritePtr)> callback);
    
    /**
     * @brief Register a callback for sprite removed events
     * @param callback Function to call when a sprite is removed
     * @return ID of the registered callback
     */
    int registerSpriteRemovedCallback(std::function<void(std::uint32_t)> callback);
    
    /**
     * @brief Unregister a sprite added callback
     * @param callbackId ID of callback to unregister
     */
    void unregisterSpriteAddedCallback(int callbackId);
    
    /**
     * @brief Unregister a sprite removed callback
     * @param callbackId ID of callback to unregister
     */
    void unregisterSpriteRemovedCallback(int callbackId);
    
    /**
     * @brief Clear all sprites
     */
    void clear();
    
    /**
     * @brief Get the count of sprites
     * @return Number of sprites
     */
    size_t getCount() const;
    
private:
    /**
     * @brief Private constructor for singleton pattern
     */
    SpriteManager();
    
    /**
     * @brief Private destructor
     */
    ~SpriteManager();
    
    // Prevent copy and move
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;
    SpriteManager(SpriteManager&&) = delete;
    SpriteManager& operator=(SpriteManager&&) = delete;
    
    /**
     * @brief Create a sprite from packet data
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param serial Serial ID
     * @param image Image ID
     * @param color Color value
     * @param display Display flags
     * @return Shared pointer to created sprite
     */
    SpritePtr createSprite(std::uint16_t x, std::uint16_t y, std::uint32_t serial, 
                          std::uint16_t image, std::uint8_t color, std::uint16_t display);

    std::unordered_map<std::uint32_t, SpritePtr> sprites_;  // Map of sprite ID to sprite
    std::mutex spriteMutex_;  // Mutex for thread-safe access to sprites
    
    std::unordered_map<int, std::function<void(SpritePtr)>> spriteAddedCallbacks_;  // Callbacks for sprite added events
    std::unordered_map<int, std::function<void(std::uint32_t)>> spriteRemovedCallbacks_;  // Callbacks for sprite removed events
    int nextCallbackId_;  // ID for next callback
    std::mutex callbackMutex_;  // Mutex for thread-safe access to callbacks
};

} // namespace game
