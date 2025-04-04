/**
 * @file sprite.cpp
 * @brief Implementation of the Sprite class and derived classes
 */
#include "pch.h"
#include "sprite.h"
#include "../utils/logging.h"

#include <cmath>

namespace game {

Sprite::Sprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, std::uint16_t img)
    : xCoord_(x)
    , yCoord_(y)
    , serial_(id)
    , image_(img)
    , color_(0)
    , direction_(Direction::Unknown)
    , type_(NpcType::Mundane)
    , name_("")
{
}

std::uint32_t Sprite::getSerial() const
{
    return serial_;
}

std::uint16_t Sprite::getX() const
{
    return xCoord_;
}

std::uint16_t Sprite::getY() const
{
    return yCoord_;
}

std::uint16_t Sprite::getImage() const
{
    return image_;
}

std::uint8_t Sprite::getColor() const
{
    return color_;
}

Direction Sprite::getDirection() const
{
    return direction_;
}

const std::string& Sprite::getName() const
{
    return name_;
}

void Sprite::setPosition(std::uint16_t x, std::uint16_t y)
{
    xCoord_ = x;
    yCoord_ = y;
}

void Sprite::setDirection(Direction dir)
{
    direction_ = dir;
}

void Sprite::setColor(std::uint8_t col)
{
    color_ = col;
}

void Sprite::setName(const std::string& spriteName)
{
    name_ = spriteName;
}

NpcType Sprite::getType() const
{
    return type_;
}

float Sprite::distanceTo(const Sprite& other) const
{
    const float dx = static_cast<float>(xCoord_) - static_cast<float>(other.xCoord_);
    const float dy = static_cast<float>(yCoord_) - static_cast<float>(other.yCoord_);
    return std::sqrt(dx * dx + dy * dy);
}

bool Sprite::isInRange(const Sprite& other, float range) const
{
    return distanceTo(other) <= range;
}

void Sprite::update(std::uint16_t x, std::uint16_t y, std::uint16_t img, 
                  std::uint8_t col, Direction dir)
{
    xCoord_ = x;
    yCoord_ = y;
    image_ = img;
    color_ = col;
    direction_ = dir;
}

// MonsterSprite implementation
MonsterSprite::MonsterSprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, 
                           std::uint16_t img, std::uint16_t spriteOffset)
    : Sprite(x, y, id, img)
    , spriteOffset_(spriteOffset)
{
    type_ = NpcType::Monster;
}

std::uint16_t MonsterSprite::getSpriteOffset() const
{
    return spriteOffset_;
}

NpcType MonsterSprite::getType() const
{
    return NpcType::Monster;
}

// NpcSprite implementation
NpcSprite::NpcSprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, 
                   std::uint16_t img, const std::string& npcName)
    : Sprite(x, y, id, img)
{
    type_ = NpcType::Mundane;
    name_ = npcName;
}

NpcType NpcSprite::getType() const
{
    return type_;
}

void NpcSprite::setNpcType(NpcType npcType)
{
    type_ = npcType;
}

// ItemSprite implementation
ItemSprite::ItemSprite(std::uint16_t x, std::uint16_t y, std::uint32_t id, 
                     std::uint16_t img, std::uint16_t itemId)
    : Sprite(x, y, id, img)
    , itemId_(itemId)
{
    type_ = NpcType::Item;
}

std::uint16_t ItemSprite::getItemId() const
{
    return itemId_;
}

NpcType ItemSprite::getType() const
{
    return NpcType::Item;
}

} // namespace game
