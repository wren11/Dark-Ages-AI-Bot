/**
 * @file entity.h
 * @brief Base class for game entities with common properties
 */
#pragma once

#include <cstdint>
#include <string>
#include "position.h"

namespace game {

/**
 * @brief Base class for all game entities
 * 
 * Defines common properties shared by all entities in the game world.
 * Following the Single Responsibility Principle by focusing only
 * on basic entity attributes.
 */
class Entity {
public:
    /**
     * @brief Constructor with serial ID
     * @param serialId Unique identifier for this entity
     */
    explicit Entity(std::uint32_t serialId = 0);
    
    /**
     * @brief Virtual destructor for proper inheritance
     */
    virtual ~Entity() = default;
    
    /**
     * @brief Get the entity's unique serial ID
     * @return Serial ID
     */
    [[nodiscard]] std::uint32_t getSerialId() const noexcept;
    
    /**
     * @brief Set the entity's unique serial ID
     * @param serialId New serial ID
     */
    void setSerialId(std::uint32_t serialId) noexcept;
    
    /**
     * @brief Get the entity's position in the game world
     * @return Reference to position
     */
    [[nodiscard]] const Position& getPosition() const noexcept;
    
    /**
     * @brief Set the entity's position in the game world
     * @param position New position
     */
    void setPosition(const Position& position);
    
    /**
     * @brief Get the entity's name
     * @return Entity name
     */
    [[nodiscard]] const std::string& getName() const noexcept;
    
    /**
     * @brief Set the entity's name
     * @param name New entity name
     */
    void setName(const std::string& name);
    
    /**
     * @brief Check if this entity is valid (has a non-zero ID)
     * @return True if the entity is valid
     */
    [[nodiscard]] bool isValid() const noexcept;
    
    /**
     * @brief Update method to be implemented by derived classes
     * @param currentTime Current game time
     */
    virtual void update(std::uint64_t currentTime) = 0;

private:
    std::uint32_t serialId_;  // Unique identifier for this entity
    Position position_;       // Position in the game world
    std::string name_;        // Entity name
};

} // namespace game
