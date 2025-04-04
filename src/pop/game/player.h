/**
 * @file player.h
 * @brief Player entity class representing a player character in the game
 */
#pragma once

#include "entity.h"
#include <vector>
#include <ctime>
#include <memory>
#include <iomanip>
#include <iostream>

namespace game {

/**
 * @brief Represents a player character in the game world
 * 
 * Extends the base Entity class with player-specific attributes like
 * appearance, equipment, and stats. Provides methods for updating and
 * querying player state.
 */
class Player : public Entity {
public:
    /**
     * @brief Default constructor
     */
    Player();
    
    /**
     * @brief Constructor with serial ID
     * @param serialId Unique identifier for this player
     */
    explicit Player(std::uint32_t serialId);
    
    /**
     * @brief Destructor
     */
    ~Player() override = default;
    
    /**
     * @brief Update player state based on current time
     * @param currentTime Current game time
     */
    void update(std::uint64_t currentTime) override;
    
    // Appearance getters
    [[nodiscard]] std::uint16_t getHead() const noexcept;
    [[nodiscard]] std::uint16_t getForm() const noexcept;
    [[nodiscard]] std::uint16_t getBody() const noexcept;
    [[nodiscard]] std::uint16_t getArms() const noexcept;
    [[nodiscard]] std::uint16_t getBoots() const noexcept;
    [[nodiscard]] std::uint16_t getArmor() const noexcept;
    [[nodiscard]] std::uint16_t getShield() const noexcept;
    [[nodiscard]] std::uint16_t getWeapon() const noexcept;
    
    // Accessory getters
    [[nodiscard]] std::uint16_t getAcc1() const noexcept;
    [[nodiscard]] std::uint16_t getAcc2() const noexcept;
    [[nodiscard]] std::uint16_t getAcc3() const noexcept;
    [[nodiscard]] std::uint16_t getOvercoat() const noexcept;
    
    // Color getters
    [[nodiscard]] std::uint16_t getHeadColor() const noexcept;
    [[nodiscard]] std::uint16_t getBootColor() const noexcept;
    [[nodiscard]] std::uint16_t getAcc1Color() const noexcept;
    [[nodiscard]] std::uint16_t getAcc2Color() const noexcept;
    [[nodiscard]] std::uint16_t getOvercoatColor() const noexcept;
    [[nodiscard]] std::uint16_t getSkinColor() const noexcept;
    
    // Appearance flags
    [[nodiscard]] std::uint8_t getRestCloak() const noexcept;
    [[nodiscard]] std::uint8_t getHideBool() const noexcept;
    [[nodiscard]] std::uint8_t getFaceShape() const noexcept;
    
    // Additional info
    [[nodiscard]] std::uint8_t getNameTagStyle() const noexcept;
    [[nodiscard]] const std::string& getGroupName() const noexcept;
    [[nodiscard]] bool isHostile() const noexcept;
    [[nodiscard]] std::time_t getLastSealSeen() const noexcept;
    [[nodiscard]] std::time_t getKelbLastSeen() const noexcept;

    // Appearance setters
    void setHead(std::uint16_t head) noexcept;
    void setForm(std::uint16_t form) noexcept;
    void setBody(std::uint16_t body) noexcept;
    void setArms(std::uint16_t arms) noexcept;
    void setBoots(std::uint16_t boots) noexcept;
    void setArmor(std::uint16_t armor) noexcept;
    void setShield(std::uint16_t shield) noexcept;
    void setWeapon(std::uint16_t weapon) noexcept;
    
    // Accessory setters
    void setAcc1(std::uint16_t acc1) noexcept;
    void setAcc2(std::uint16_t acc2) noexcept;
    void setAcc3(std::uint16_t acc3) noexcept;
    void setOvercoat(std::uint16_t overcoat) noexcept;
    
    // Color setters
    void setHeadColor(std::uint16_t headColor) noexcept;
    void setBootColor(std::uint16_t bootColor) noexcept;
    void setAcc1Color(std::uint16_t acc1Color) noexcept;
    void setAcc2Color(std::uint16_t acc2Color) noexcept;
    void setOvercoatColor(std::uint16_t overcoatColor) noexcept;
    void setSkinColor(std::uint16_t skinColor) noexcept;
    
    // Appearance flags setters
    void setRestCloak(std::uint8_t restCloak) noexcept;
    void setHideBool(std::uint8_t hideBool) noexcept;
    void setFaceShape(std::uint8_t faceShape) noexcept;
    
    // Additional info setters
    void setNameTagStyle(std::uint8_t nameTagStyle) noexcept;
    void setGroupName(const std::string& groupName);
    void setHostile(bool hostile) noexcept;
    void setLastSealSeen(std::time_t time) noexcept;
    void setKelbLastSeen(std::time_t time) noexcept;

    /**
     * @brief Check if this player is in the hostiles list
     * @param hostileList List of hostile player names
     * @return True if player is hostile
     */
    [[nodiscard]] bool isHostile(const std::vector<std::string>& hostileList) const;
    
    /**
     * @brief Update player data from another player
     * @param updatedPlayer Player with updated data
     */
    void updateFrom(const Player& updatedPlayer);
    
    /**
     * @brief Print all player data for debugging
     */
    void printData() const;

private:
    // Appearance
    std::uint16_t head_;
    std::uint16_t form_;
    std::uint16_t body_;
    std::uint16_t arms_;
    std::uint16_t boots_;
    std::uint16_t armor_;
    std::uint16_t shield_;
    std::uint16_t weapon_;
    
    // Accessories
    std::uint16_t acc1_;
    std::uint16_t acc2_;
    std::uint16_t acc3_;
    std::uint16_t overcoat_;
    
    // Colors
    std::uint16_t headColor_;
    std::uint16_t bootColor_;
    std::uint16_t acc1Color_;
    std::uint16_t acc2Color_;
    std::uint16_t overcoatColor_;
    std::uint16_t skinColor_;
    
    // Appearance flags
    std::uint8_t restCloak_;
    std::uint8_t hideBool_;
    std::uint8_t faceShape_;
    std::uint8_t unknown1_;
    std::uint8_t unknown2_;
    
    // Additional information
    std::string groupName_;
    std::uint8_t nameTagStyle_;
    bool hostile_;
    std::time_t kelbLastSeen_;
    std::time_t lastSealSeen_;
};

/**
 * @brief Shared pointer type for Player objects
 */
using PlayerPtr = std::shared_ptr<Player>;

} // namespace game
