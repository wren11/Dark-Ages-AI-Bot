/**
 * @file game_data_manager.cpp
 * @brief Implementation of the GameDataManager class
 */
#include "pch.h"
#include "game_data_manager.h"
#include <algorithm>
#include "../utils/logging.h"

namespace game {

GameDataManager& GameDataManager::getInstance() {
    static GameDataManager instance;
    return instance;
}

GameDataManager::GameDataManager()
    : players_(), localPlayerId_(0), hostileList_() {
}

void GameDataManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    utils::Logging::info("Initializing GameDataManager");
    // Reset data structures to ensure a clean state
    players_.clear();
    localPlayerId_ = 0;
    hostileList_.clear();
}

void GameDataManager::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    utils::Logging::info("Resetting GameDataManager");
    players_.clear();
    localPlayerId_ = 0;
    // Note: We don't clear the hostile list as it's persisted between sessions
}

void GameDataManager::updatePlayer(std::uint32_t serialId, const Player& player) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = players_.find(serialId);
    if (it != players_.end()) {
        // Update existing player
        it->second->updateFrom(player);
        utils::Logging::debug("Updated player: " + player.getName() + " (ID: " + std::to_string(serialId) + ")");
    } else {
        // Add new player
        auto newPlayer = std::make_shared<Player>(player);
        players_[serialId] = newPlayer;
        utils::Logging::debug("Added new player: " + player.getName() + " (ID: " + std::to_string(serialId) + ")");
        
        // Check if this player is in the hostile list
        if (isHostile(player.getName())) {
            newPlayer->setHostile(true);
            utils::Logging::warning("Detected hostile player: " + player.getName());
        }
    }
}

PlayerPtr GameDataManager::getPlayer(std::uint32_t serialId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = players_.find(serialId);
    if (it != players_.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool GameDataManager::removePlayer(std::uint32_t serialId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = players_.find(serialId);
    if (it != players_.end()) {
        utils::Logging::debug("Removed player: " + it->second->getName() + " (ID: " + std::to_string(serialId) + ")");
        players_.erase(it);
        return true;
    }
    
    return false;
}

PlayerPtr GameDataManager::getPlayerByName(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& pair : players_) {
        if (pair.second->getName() == name) {
            return pair.second;
        }
    }
    
    return nullptr;
}

std::vector<PlayerPtr> GameDataManager::getAllPlayers() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<PlayerPtr> result;
    result.reserve(players_.size());
    
    for (const auto& pair : players_) {
        result.push_back(pair.second);
    }
    
    return result;
}

void GameDataManager::setLocalPlayer(std::uint32_t serialId) {
    std::lock_guard<std::mutex> lock(mutex_);
    localPlayerId_ = serialId;
    
    auto player = getPlayer(serialId);
    if (player) {
        utils::Logging::info("Local player set: " + player->getName() + " (ID: " + std::to_string(serialId) + ")");
    } else {
        utils::Logging::warning("Local player set with ID: " + std::to_string(serialId) + ", but player not found");
    }
}

PlayerPtr GameDataManager::getLocalPlayer() {
    if (localPlayerId_ == 0) {
        return nullptr;
    }
    
    return getPlayer(localPlayerId_);
}

void GameDataManager::addHostile(const std::string& playerName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if player is already in the hostile list
    if (std::find(hostileList_.begin(), hostileList_.end(), playerName) == hostileList_.end()) {
        hostileList_.push_back(playerName);
        utils::Logging::warning("Added hostile player: " + playerName);
        
        // Update player's hostile flag if they're currently in the game
        auto player = getPlayerByName(playerName);
        if (player) {
            player->setHostile(true);
        }
    }
}

bool GameDataManager::removeHostile(const std::string& playerName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(hostileList_.begin(), hostileList_.end(), playerName);
    if (it != hostileList_.end()) {
        hostileList_.erase(it);
        utils::Logging::info("Removed hostile player: " + playerName);
        
        // Update player's hostile flag if they're currently in the game
        auto player = getPlayerByName(playerName);
        if (player) {
            player->setHostile(false);
        }
        
        return true;
    }
    
    return false;
}

bool GameDataManager::isHostile(const std::string& playerName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    return std::find(hostileList_.begin(), hostileList_.end(), playerName) != hostileList_.end();
}

const std::vector<std::string>& GameDataManager::getHostileList() const {
    return hostileList_;
}

} // namespace game
