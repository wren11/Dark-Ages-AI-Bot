/**
 * @file sprite_manager.cpp
 * @brief Implementation of the SpriteManager class
 */
#include "pch.h"
#include "sprite_manager.h"
#include "../utils/logging.h"
#include "../network/packet_reader.h"

#include <algorithm>
#include <exception>
#include <cmath>

namespace game {

SpriteManager& SpriteManager::getInstance() {
    static SpriteManager instance;
    return instance;
}

SpriteManager::SpriteManager() 
    : nextCallbackId_(0) 
{
    utils::Logging::info("SpriteManager initialized");
}

SpriteManager::~SpriteManager() {
    utils::Logging::info("SpriteManager destroyed");
}

void SpriteManager::processSpritePacket(const network::Packet& packet) {
    try {
        network::PacketReader reader(packet.getData(), packet.size());
        reader.readByte(); // Skip packet ID

        auto count = reader.read<uint16_t>();
        for (uint16_t i = 0; i < count; ++i) {
            auto xCoord = reader.read<uint16_t>();
            auto yCoord = reader.read<uint16_t>();
            auto serial = reader.read<uint32_t>();
            auto image = reader.read<uint16_t>();
            auto color = reader.read<uint8_t>();
            auto display = reader.read<uint16_t>();

            SpritePtr sprite = getSprite(serial);
            if (sprite) {
                // Update existing sprite
                Direction dir = static_cast<Direction>(display & 0x03);
                sprite->update(xCoord, yCoord, image, color, dir);
                utils::Logging::debug("Updated sprite: ID=" + std::to_string(serial) + 
                                     " at X=" + std::to_string(xCoord) + 
                                     " Y=" + std::to_string(yCoord));
            } else {
                // Create new sprite
                sprite = createSprite(xCoord, yCoord, serial, image, color, display);
                if (sprite) {
                    addSprite(sprite);
                    utils::Logging::debug("Added sprite: ID=" + std::to_string(serial) + 
                                         " at X=" + std::to_string(xCoord) + 
                                         " Y=" + std::to_string(yCoord));
                }
            }
        }
    } catch (const std::exception& e) {
        utils::Logging::error("Exception in processSpritePacket: " + std::string(e.what()));
    } catch (...) {
        utils::Logging::error("Unknown exception in processSpritePacket");
    }
}

SpritePtr SpriteManager::createSprite(std::uint16_t x, std::uint16_t y, std::uint32_t serial, 
                                    std::uint16_t image, std::uint8_t color, std::uint16_t display) {
    try {
        Direction dir = static_cast<Direction>(display & 0x03);
        
        if (image >= 0x4000 && image <= 0x8000) {
            // Monster or NPC
            auto spriteOffset = image - 0x4000;
            
            // Read additional data (these would come from packet data in a real implementation)
            // For the refactoring, we're keeping these as placeholders
            auto unknown1 = 0; // Would be read from packet
            auto num4 = 0;     // Would be read from packet
            auto unknown2 = 0; // Would be read from packet
            auto type = 0;     // Would be read from packet
            
            if (type == 0x2) {
                // NPC
                std::string name = "NPC"; // Would be read from packet
                auto npcSprite = std::make_shared<NpcSprite>(x, y, serial, image, name);
                npcSprite->setDirection(dir);
                npcSprite->setColor(color);
                return npcSprite;
            } else {
                // Monster
                auto monsterSprite = std::make_shared<MonsterSprite>(x, y, serial, image, spriteOffset);
                monsterSprite->setDirection(dir);
                monsterSprite->setColor(color);
                return monsterSprite;
            }
        } else if (image >= 0x8000) {
            // Item
            auto itemId = image - 0x8000;
            auto itemSprite = std::make_shared<ItemSprite>(x, y, serial, image, itemId);
            itemSprite->setDirection(dir);
            itemSprite->setColor(color);
            return itemSprite;
        } else {
            // Generic sprite (shouldn't typically happen)
            auto genericSprite = std::make_shared<Sprite>(x, y, serial, image);
            genericSprite->setDirection(dir);
            genericSprite->setColor(color);
            return genericSprite;
        }
    } catch (const std::exception& e) {
        utils::Logging::error("Exception in createSprite: " + std::string(e.what()));
        return nullptr;
    } catch (...) {
        utils::Logging::error("Unknown exception in createSprite");
        return nullptr;
    }
}

void SpriteManager::addSprite(SpritePtr sprite) {
    if (!sprite) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(spriteMutex_);
    auto serial = sprite->getSerial();
    sprites_[serial] = sprite;
    
    // Notify callbacks
    std::lock_guard<std::mutex> callbackLock(callbackMutex_);
    for (const auto& callback : spriteAddedCallbacks_) {
        try {
            callback.second(sprite);
        } catch (...) {
            utils::Logging::error("Exception in sprite added callback");
        }
    }
}

bool SpriteManager::removeSprite(std::uint32_t spriteId) {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    auto it = sprites_.find(spriteId);
    if (it != sprites_.end()) {
        sprites_.erase(it);
        
        // Notify callbacks
        std::lock_guard<std::mutex> callbackLock(callbackMutex_);
        for (const auto& callback : spriteRemovedCallbacks_) {
            try {
                callback.second(spriteId);
            } catch (...) {
                utils::Logging::error("Exception in sprite removed callback");
            }
        }
        return true;
    }
    return false;
}

SpritePtr SpriteManager::getSprite(std::uint32_t spriteId) {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    auto it = sprites_.find(spriteId);
    if (it != sprites_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<SpritePtr> SpriteManager::getAllSprites() {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    std::vector<SpritePtr> result;
    result.reserve(sprites_.size());
    
    for (const auto& pair : sprites_) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::vector<SpritePtr> SpriteManager::getSpritesInRadius(std::uint16_t x, std::uint16_t y, float radius) {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    std::vector<SpritePtr> result;
    
    // Create a dummy sprite at the center point for distance calculations
    Sprite centerSprite(x, y, 0, 0);
    
    for (const auto& pair : sprites_) {
        auto sprite = pair.second;
        if (sprite) {
            const float dx = static_cast<float>(sprite->getX()) - static_cast<float>(x);
            const float dy = static_cast<float>(sprite->getY()) - static_cast<float>(y);
            const float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= radius) {
                result.push_back(sprite);
            }
        }
    }
    
    return result;
}

std::vector<SpritePtr> SpriteManager::getSpritesByType(NpcType type) {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    std::vector<SpritePtr> result;
    
    for (const auto& pair : sprites_) {
        auto sprite = pair.second;
        if (sprite && sprite->getType() == type) {
            result.push_back(sprite);
        }
    }
    
    return result;
}

int SpriteManager::registerSpriteAddedCallback(std::function<void(SpritePtr)> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    int id = nextCallbackId_++;
    spriteAddedCallbacks_[id] = callback;
    return id;
}

int SpriteManager::registerSpriteRemovedCallback(std::function<void(std::uint32_t)> callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    int id = nextCallbackId_++;
    spriteRemovedCallbacks_[id] = callback;
    return id;
}

void SpriteManager::unregisterSpriteAddedCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    spriteAddedCallbacks_.erase(callbackId);
}

void SpriteManager::unregisterSpriteRemovedCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    spriteRemovedCallbacks_.erase(callbackId);
}

void SpriteManager::clear() {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    sprites_.clear();
}

size_t SpriteManager::getCount() const {
    std::lock_guard<std::mutex> lock(spriteMutex_);
    return sprites_.size();
}

} // namespace game
