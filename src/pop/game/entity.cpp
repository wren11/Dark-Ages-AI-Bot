/**
 * @file entity.cpp
 * @brief Implementation of the Entity base class
 */
#include "pch.h"
#include "entity.h"

namespace game {

Entity::Entity(std::uint32_t serialId)
    : serialId_(serialId), position_(), name_() {
}

std::uint32_t Entity::getSerialId() const noexcept {
    return serialId_;
}

void Entity::setSerialId(std::uint32_t serialId) noexcept {
    serialId_ = serialId;
}

const Position& Entity::getPosition() const noexcept {
    return position_;
}

void Entity::setPosition(const Position& position) {
    position_ = position;
}

const std::string& Entity::getName() const noexcept {
    return name_;
}

void Entity::setName(const std::string& name) {
    name_ = name;
}

bool Entity::isValid() const noexcept {
    return serialId_ != 0;
}

} // namespace game
