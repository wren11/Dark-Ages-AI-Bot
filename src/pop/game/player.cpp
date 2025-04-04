#include "pch.h"

/**
 * @file player.cpp
 * @brief Implementation of the Player class
 */
#include "player.h"

namespace game {

Player::Player()
    : Entity(0),
      head_(0), form_(0), body_(0), arms_(0), boots_(0), armor_(0), shield_(0), weapon_(0),
      acc1_(0), acc2_(0), acc3_(0), overcoat_(0),
      headColor_(0), bootColor_(0), acc1Color_(0), acc2Color_(0), overcoatColor_(0), skinColor_(0),
      restCloak_(0), hideBool_(0), faceShape_(0), unknown1_(0), unknown2_(0),
      groupName_(), nameTagStyle_(0), hostile_(false),
      kelbLastSeen_(0), lastSealSeen_(0) {
}

Player::Player(std::uint32_t serialId)
    : Entity(serialId),
      head_(0), form_(0), body_(0), arms_(0), boots_(0), armor_(0), shield_(0), weapon_(0),
      acc1_(0), acc2_(0), acc3_(0), overcoat_(0),
      headColor_(0), bootColor_(0), acc1Color_(0), acc2Color_(0), overcoatColor_(0), skinColor_(0),
      restCloak_(0), hideBool_(0), faceShape_(0), unknown1_(0), unknown2_(0),
      groupName_(), nameTagStyle_(0), hostile_(false),
      kelbLastSeen_(0), lastSealSeen_(0) {
}

void Player::update(std::uint64_t currentTime) {
    // Currently empty - will be implemented with game-specific update logic
    // such as monitoring buffs, status effects, etc.
}

// Appearance getters
std::uint16_t Player::getHead() const noexcept { return head_; }
std::uint16_t Player::getForm() const noexcept { return form_; }
std::uint16_t Player::getBody() const noexcept { return body_; }
std::uint16_t Player::getArms() const noexcept { return arms_; }
std::uint16_t Player::getBoots() const noexcept { return boots_; }
std::uint16_t Player::getArmor() const noexcept { return armor_; }
std::uint16_t Player::getShield() const noexcept { return shield_; }
std::uint16_t Player::getWeapon() const noexcept { return weapon_; }

// Accessory getters
std::uint16_t Player::getAcc1() const noexcept { return acc1_; }
std::uint16_t Player::getAcc2() const noexcept { return acc2_; }
std::uint16_t Player::getAcc3() const noexcept { return acc3_; }
std::uint16_t Player::getOvercoat() const noexcept { return overcoat_; }

// Color getters
std::uint16_t Player::getHeadColor() const noexcept { return headColor_; }
std::uint16_t Player::getBootColor() const noexcept { return bootColor_; }
std::uint16_t Player::getAcc1Color() const noexcept { return acc1Color_; }
std::uint16_t Player::getAcc2Color() const noexcept { return acc2Color_; }
std::uint16_t Player::getOvercoatColor() const noexcept { return overcoatColor_; }
std::uint16_t Player::getSkinColor() const noexcept { return skinColor_; }

// Appearance flags
std::uint8_t Player::getRestCloak() const noexcept { return restCloak_; }
std::uint8_t Player::getHideBool() const noexcept { return hideBool_; }
std::uint8_t Player::getFaceShape() const noexcept { return faceShape_; }

// Additional info
std::uint8_t Player::getNameTagStyle() const noexcept { return nameTagStyle_; }
const std::string& Player::getGroupName() const noexcept { return groupName_; }
bool Player::isHostile() const noexcept { return hostile_; }
std::time_t Player::getLastSealSeen() const noexcept { return lastSealSeen_; }
std::time_t Player::getKelbLastSeen() const noexcept { return kelbLastSeen_; }

// Appearance setters
void Player::setHead(std::uint16_t head) noexcept { head_ = head; }
void Player::setForm(std::uint16_t form) noexcept { form_ = form; }
void Player::setBody(std::uint16_t body) noexcept { body_ = body; }
void Player::setArms(std::uint16_t arms) noexcept { arms_ = arms; }
void Player::setBoots(std::uint16_t boots) noexcept { boots_ = boots; }
void Player::setArmor(std::uint16_t armor) noexcept { armor_ = armor; }
void Player::setShield(std::uint16_t shield) noexcept { shield_ = shield; }
void Player::setWeapon(std::uint16_t weapon) noexcept { weapon_ = weapon; }

// Accessory setters
void Player::setAcc1(std::uint16_t acc1) noexcept { acc1_ = acc1; }
void Player::setAcc2(std::uint16_t acc2) noexcept { acc2_ = acc2; }
void Player::setAcc3(std::uint16_t acc3) noexcept { acc3_ = acc3; }
void Player::setOvercoat(std::uint16_t overcoat) noexcept { overcoat_ = overcoat; }

// Color setters
void Player::setHeadColor(std::uint16_t headColor) noexcept { headColor_ = headColor; }
void Player::setBootColor(std::uint16_t bootColor) noexcept { bootColor_ = bootColor; }
void Player::setAcc1Color(std::uint16_t acc1Color) noexcept { acc1Color_ = acc1Color; }
void Player::setAcc2Color(std::uint16_t acc2Color) noexcept { acc2Color_ = acc2Color; }
void Player::setOvercoatColor(std::uint16_t overcoatColor) noexcept { overcoatColor_ = overcoatColor; }
void Player::setSkinColor(std::uint16_t skinColor) noexcept { skinColor_ = skinColor; }

// Appearance flags setters
void Player::setRestCloak(std::uint8_t restCloak) noexcept { restCloak_ = restCloak; }
void Player::setHideBool(std::uint8_t hideBool) noexcept { hideBool_ = hideBool; }
void Player::setFaceShape(std::uint8_t faceShape) noexcept { faceShape_ = faceShape; }

// Additional info setters
void Player::setNameTagStyle(std::uint8_t nameTagStyle) noexcept { nameTagStyle_ = nameTagStyle; }
void Player::setGroupName(const std::string& groupName) { groupName_ = groupName; }
void Player::setHostile(bool hostile) noexcept { hostile_ = hostile; }
void Player::setLastSealSeen(std::time_t time) noexcept { lastSealSeen_ = time; }
void Player::setKelbLastSeen(std::time_t time) noexcept { kelbLastSeen_ = time; }

bool Player::isHostile(const std::vector<std::string>& hostileList) const {
    return std::find(hostileList.begin(), hostileList.end(), getName()) != hostileList.end();
}

void Player::updateFrom(const Player& updatedPlayer) {
    // Update base entity properties
    setSerialId(updatedPlayer.getSerialId());
    setName(updatedPlayer.getName());
    setPosition(updatedPlayer.getPosition());
    
    // Update appearance
    head_ = updatedPlayer.head_;
    form_ = updatedPlayer.form_;
    body_ = updatedPlayer.body_;
    arms_ = updatedPlayer.arms_;
    boots_ = updatedPlayer.boots_;
    armor_ = updatedPlayer.armor_;
    shield_ = updatedPlayer.shield_;
    weapon_ = updatedPlayer.weapon_;
    
    // Update accessories
    acc1_ = updatedPlayer.acc1_;
    acc2_ = updatedPlayer.acc2_;
    acc3_ = updatedPlayer.acc3_;
    overcoat_ = updatedPlayer.overcoat_;
    
    // Update colors
    headColor_ = updatedPlayer.headColor_;
    bootColor_ = updatedPlayer.bootColor_;
    acc1Color_ = updatedPlayer.acc1Color_;
    acc2Color_ = updatedPlayer.acc2Color_;
    overcoatColor_ = updatedPlayer.overcoatColor_;
    skinColor_ = updatedPlayer.skinColor_;
    
    // Update appearance flags
    restCloak_ = updatedPlayer.restCloak_;
    hideBool_ = updatedPlayer.hideBool_;
    faceShape_ = updatedPlayer.faceShape_;
    
    // Update additional info
    groupName_ = updatedPlayer.groupName_;
    nameTagStyle_ = updatedPlayer.nameTagStyle_;
    
    // Update timestamps
    lastSealSeen_ = updatedPlayer.lastSealSeen_;
    kelbLastSeen_ = updatedPlayer.kelbLastSeen_;
}

void Player::printData() const {
    constexpr int width = 15;
    std::cout << std::left << std::setw(width) << "Field" << std::setw(width) << "Value" << '\n';
    std::cout << std::string(30, '-') << '\n';
    
    std::cout << std::setw(width) << "Serial" << std::setw(width) << getSerialId() << '\n';
    
    // Print position
    const auto& pos = getPosition();
    std::cout << std::setw(width) << "Position"
              << "X: " << pos.getX() << ", Y: " << pos.getY() 
              << ", Direction: " << static_cast<int>(pos.getFacingDirection()) << '\n';
    
    // Print name and group
    std::cout << std::setw(width) << "Name" << std::setw(width) << getName() << '\n';
    std::cout << std::setw(width) << "GroupName" << std::setw(width) << groupName_ << '\n';
    
    // Print appearance
    std::cout << std::setw(width) << "Head" << std::setw(width) << head_ << '\n';
    std::cout << std::setw(width) << "Form" << std::setw(width) << form_ << '\n';
    std::cout << std::setw(width) << "Body" << std::setw(width) << body_ << '\n';
    std::cout << std::setw(width) << "Arms" << std::setw(width) << arms_ << '\n';
    std::cout << std::setw(width) << "Boots" << std::setw(width) << boots_ << '\n';
    std::cout << std::setw(width) << "Armor" << std::setw(width) << armor_ << '\n';
    std::cout << std::setw(width) << "Shield" << std::setw(width) << shield_ << '\n';
    std::cout << std::setw(width) << "Weapon" << std::setw(width) << weapon_ << '\n';
    
    // Print colors
    std::cout << std::setw(width) << "HeadColor" << std::setw(width) << headColor_ << '\n';
    std::cout << std::setw(width) << "BootColor" << std::setw(width) << bootColor_ << '\n';
    std::cout << std::setw(width) << "Acc1Color" << std::setw(width) << acc1Color_ << '\n';
    std::cout << std::setw(width) << "Acc2Color" << std::setw(width) << acc2Color_ << '\n';
    std::cout << std::setw(width) << "OvercoatColor" << std::setw(width) << overcoatColor_ << '\n';
    std::cout << std::setw(width) << "SkinColor" << std::setw(width) << skinColor_ << '\n';
    
    // Print accessories
    std::cout << std::setw(width) << "Acc1" << std::setw(width) << acc1_ << '\n';
    std::cout << std::setw(width) << "Acc2" << std::setw(width) << acc2_ << '\n';
    std::cout << std::setw(width) << "Acc3" << std::setw(width) << acc3_ << '\n';
    std::cout << std::setw(width) << "Overcoat" << std::setw(width) << overcoat_ << '\n';
    
    // Print flags
    std::cout << std::setw(width) << "RestCloak" << std::setw(width) << static_cast<int>(restCloak_) << '\n';
    std::cout << std::setw(width) << "HideBool" << std::setw(width) << static_cast<int>(hideBool_) << '\n';
    std::cout << std::setw(width) << "FaceShape" << std::setw(width) << static_cast<int>(faceShape_) << '\n';
    std::cout << std::setw(width) << "NameTagStyle" << std::setw(width) << static_cast<int>(nameTagStyle_) << '\n';
    
    // Print status
    std::cout << std::setw(width) << "Hostile" << std::setw(width) << (hostile_ ? "Yes" : "No") << '\n';
    
    std::cout << std::string(30, '-') << '\n';
}

} // namespace game
