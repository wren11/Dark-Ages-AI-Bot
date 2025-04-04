/**
 * @file sprite.h
 * @brief Enhanced sprite class for game rendering
 */
#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace game {

/**
 * @brief Direction enumeration
 */
enum class Direction : std::uint8_t {
    North = 0,
    East = 1,
    South = 2,
    West = 3,
    Unknown = 4
};

/**
 * @brief NPC type enumeration
 */
enum class NpcType : std::uint8_t {
    Mundane = 0, // Regular NPC
    Item = 1,    // Item vendor
    Monster = 2, // Monster
    Player = 3   // Player character
};

/**
 * @brief Base sprite class that represents any visible entity
 */
class Sprite {
public:
    /**
     * @brief Constructor with basic sprite information
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param id Serial ID
     * @param img Image ID
     */
    Sprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, std::uint16_t img);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Sprite() = default;
    
    /**
     * @brief Move constructor
     */
    Sprite(Sprite&&) noexcept = default;
    
    /**
     * @brief Move assignment
     */
    Sprite& operator=(Sprite&&) noexcept = default;
    
    /**
     * @brief Copy constructor (deleted)
     */
    Sprite(const Sprite&) = delete;
    
    /**
     * @brief Copy assignment (deleted)
     */
    Sprite& operator=(const Sprite&) = delete;
    
    /**
     * @brief Get sprite serial ID
     * @return Serial ID
     */
    std::uint32_t getSerial() const;
    
    /**
     * @brief Get X-coordinate
     * @return X-coordinate
     */
    std::uint16_t getX() const;
    
    /**
     * @brief Get Y-coordinate
     * @return Y-coordinate
     */
    std::uint16_t getY() const;
    
    /**
     * @brief Get image ID
     * @return Image ID
     */
    std::uint16_t getImage() const;
    
    /**
     * @brief Get color
     * @return Color value
     */
    std::uint8_t getColor() const;
    
    /**
     * @brief Get direction
     * @return Direction 
     */
    Direction getDirection() const;
    
    /**
     * @brief Get sprite name
     * @return Sprite name
     */
    const std::string& getName() const;
    
    /**
     * @brief Set sprite position
     * @param x X-coordinate
     * @param y Y-coordinate
     */
    void setPosition(std::uint16_t x, std::uint16_t y);
    
    /**
     * @brief Set sprite direction
     * @param dir Direction
     */
    void setDirection(Direction dir);
    
    /**
     * @brief Set sprite color
     * @param col Color value
     */
    void setColor(std::uint8_t col);
    
    /**
     * @brief Set sprite name
     * @param spriteName Name to set
     */
    void setName(const std::string& spriteName);
    
    /**
     * @brief Get sprite type
     * @return Sprite type
     */
    virtual NpcType getType() const;
    
    /**
     * @brief Calculate distance to another sprite
     * @param other Other sprite
     * @return Distance in game units
     */
    float distanceTo(const Sprite& other) const;
    
    /**
     * @brief Check if sprite is within range
     * @param other Other sprite
     * @param range Range in game units
     * @return True if within range
     */
    bool isInRange(const Sprite& other, float range) const;
    
    /**
     * @brief Update sprite from packet data
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param img Image ID
     * @param col Color value
     * @param dir Direction
     */
    void update(std::uint16_t x, std::uint16_t y, std::uint16_t img, 
               std::uint8_t col, Direction dir);

protected:
    std::uint16_t xCoord_;       // X-coordinate in the game world
    std::uint16_t yCoord_;       // Y-coordinate in the game world
    std::uint32_t serial_;       // Unique identifier for this sprite
    std::uint16_t image_;        // Image ID for rendering
    std::uint8_t color_;         // Color value (for tinting)
    Direction direction_;        // Direction the sprite is facing
    NpcType type_;              // Type of sprite
    std::string name_;          // Name of the sprite (if any)
};

/**
 * @brief Shared pointer type for Sprite objects
 */
using SpritePtr = std::shared_ptr<Sprite>;

/**
 * @brief Monster sprite class
 */
class MonsterSprite : public Sprite {
public:
    /**
     * @brief Constructor with monster-specific information
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param id Serial ID
     * @param img Image ID
     * @param spriteOffset Monster sprite offset
     */
    MonsterSprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, 
                 std::uint16_t img, std::uint16_t spriteOffset);
    
    /**
     * @brief Get monster sprite offset
     * @return Sprite offset
     */
    std::uint16_t getSpriteOffset() const;
    
    /**
     * @brief Get sprite type
     * @return NpcType::Monster
     */
    NpcType getType() const override;
    
private:
    std::uint16_t spriteOffset_;  // Offset for monster sprite
};

/**
 * @brief NPC sprite class
 */
class NpcSprite : public Sprite {
public:
    /**
     * @brief Constructor with NPC-specific information
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param id Serial ID
     * @param img Image ID
     * @param npcName NPC name
     */
    NpcSprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, 
             std::uint16_t img, const std::string& npcName);
    
    /**
     * @brief Get sprite type
     * @return NpcType::Mundane or NpcType::Item
     */
    NpcType getType() const override;
    
    /**
     * @brief Set NPC type
     * @param npcType NPC type
     */
    void setNpcType(NpcType npcType);
    
private:
    // No additional fields needed
};

/**
 * @brief Item sprite class
 */
class ItemSprite : public Sprite {
public:
    /**
     * @brief Constructor with item-specific information
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param id Serial ID
     * @param img Image ID
     * @param itemId Item ID
     */
    ItemSprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, 
              std::uint16_t img, std::uint16_t itemId);
    
    /**
     * @brief Get item ID
     * @return Item ID
     */
    std::uint16_t getItemId() const;
    
    /**
     * @brief Get sprite type
     * @return NpcType::Item
     */
    NpcType getType() const override;
    
private:
    std::uint16_t itemId_;  // Item ID
};

} // namespace game
