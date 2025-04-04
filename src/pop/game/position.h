/**
 * @file position.h
 * @brief Represents an entity's position and direction in the game world
 */
#pragma once

#include <cstdint>

namespace game {

/**
 * @brief Enumeration of possible entity facing directions
 */
enum class Direction : std::uint8_t {
    North = 0,
    East = 1,
    South = 2,
    West = 3,
    Unknown = 4
};

/**
 * @brief Represents a position and facing direction in the game world
 * 
 * Encapsulates coordinates and direction with methods for common operations.
 */
class Position {
public:
    /**
     * @brief Default constructor initializes at origin facing north
     */
    Position();
    
    /**
     * @brief Constructor with coordinates and optional direction
     * @param x X-coordinate
     * @param y Y-coordinate
     * @param facingDirection Direction the entity is facing
     */
    Position(std::uint16_t x, std::uint16_t y, Direction facingDirection = Direction::North);
    
    /**
     * @brief Get X-coordinate
     * @return X-coordinate
     */
    [[nodiscard]] std::uint16_t getX() const noexcept;
    
    /**
     * @brief Get Y-coordinate
     * @return Y-coordinate
     */
    [[nodiscard]] std::uint16_t getY() const noexcept;
    
    /**
     * @brief Get facing direction
     * @return Direction entity is facing
     */
    [[nodiscard]] Direction getFacingDirection() const noexcept;
    
    /**
     * @brief Set X-coordinate
     * @param x New X-coordinate
     */
    void setX(std::uint16_t x) noexcept;
    
    /**
     * @brief Set Y-coordinate
     * @param y New Y-coordinate
     */
    void setY(std::uint16_t y) noexcept;
    
    /**
     * @brief Set facing direction
     * @param direction New facing direction
     */
    void setFacingDirection(Direction direction) noexcept;
    
    /**
     * @brief Calculate distance to another position
     * @param other Other position
     * @return Euclidean distance
     */
    [[nodiscard]] float distanceTo(const Position& other) const;
    
    /**
     * @brief Check if this position is within specified range of another
     * @param other Other position to check against
     * @param range Maximum distance to be considered "in range"
     * @return True if positions are within specified range
     */
    [[nodiscard]] bool isInRange(const Position& other, float range) const;
    
    /**
     * @brief Move position one step in the current facing direction
     * @return Reference to this position for chaining
     */
    Position& moveForward();
    
    /**
     * @brief Calculate position after moving in specified direction
     * @param direction Direction to move
     * @return New position after move
     */
    [[nodiscard]] Position getPositionInDirection(Direction direction) const;
    
    /**
     * @brief Compare equality with another position
     * @param other Position to compare with
     * @return True if positions are the same (including direction)
     */
    bool operator==(const Position& other) const noexcept;
    
    /**
     * @brief Compare inequality with another position
     * @param other Position to compare with
     * @return True if positions differ
     */
    bool operator!=(const Position& other) const noexcept;

private:
    std::uint16_t x_;                   // X-coordinate in the game world
    std::uint16_t y_;                   // Y-coordinate in the game world
    Direction facingDirection_;         // Direction the entity is facing
};

} // namespace game
