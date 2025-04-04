/**
 * @file position.cpp
 * @brief Implementation of the Position class
 */
#include "pch.h"
#include "position.h"
#include <cmath>

namespace game {

Position::Position()
    : x_(0), y_(0), facingDirection_(Direction::North) {
}

Position::Position(std::uint16_t x, std::uint16_t y, Direction facingDirection)
    : x_(x), y_(y), facingDirection_(facingDirection) {
}

std::uint16_t Position::getX() const noexcept {
    return x_;
}

std::uint16_t Position::getY() const noexcept {
    return y_;
}

Direction Position::getFacingDirection() const noexcept {
    return facingDirection_;
}

void Position::setX(std::uint16_t x) noexcept {
    x_ = x;
}

void Position::setY(std::uint16_t y) noexcept {
    y_ = y;
}

void Position::setFacingDirection(Direction direction) noexcept {
    facingDirection_ = direction;
}

float Position::distanceTo(const Position& other) const {
    // Calculate Euclidean distance (squared)
    const float dx = static_cast<float>(other.x_) - static_cast<float>(x_);
    const float dy = static_cast<float>(other.y_) - static_cast<float>(y_);
    
    return std::sqrt(dx * dx + dy * dy);
}

bool Position::isInRange(const Position& other, float range) const {
    return distanceTo(other) <= range;
}

Position& Position::moveForward() {
    switch (facingDirection_) {
        case Direction::North:
            if (y_ > 0) y_--;
            break;
        case Direction::South:
            y_++;
            break;
        case Direction::East:
            x_++;
            break;
        case Direction::West:
            if (x_ > 0) x_--;
            break;
        case Direction::Unknown:
            // No movement for unknown direction
            break;
    }
    
    return *this;
}

Position Position::getPositionInDirection(Direction direction) const {
    Position newPosition = *this;
    newPosition.setFacingDirection(direction);
    newPosition.moveForward();
    return newPosition;
}

bool Position::operator==(const Position& other) const noexcept {
    return x_ == other.x_ && 
           y_ == other.y_ && 
           facingDirection_ == other.facingDirection_;
}

bool Position::operator!=(const Position& other) const noexcept {
    return !(*this == other);
}

} // namespace game
