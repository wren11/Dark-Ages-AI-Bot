#include "pch.h"
#include "data_manager.h"

// Add or update a player's information
void GameDataManager::AddOrUpdatePlayer(unsigned int serial, const Player& player) {
    players_[serial] = player;
    std::cout << "Player added/updated: " << player.Name << " [Serial: " << serial << "]\n";
}

// Get a player's information
std::optional<Player> GameDataManager::GetPlayer(unsigned int serial) {
    if (players_.find(serial) != players_.end()) {
        return players_[serial];
    }
    return std::nullopt;
}

// Add or update an animation
void GameDataManager::AddOrUpdateAnimation(unsigned int targetId, const Animation& animation) {
    auto& animationList = animations_[targetId];

    // Check if the animation already exists and update it
    for (auto& existingAnimation : animationList) {
        if (existingAnimation.animationId == animation.animationId) {
            existingAnimation.startTime = animation.startTime;
            existingAnimation.duration = animation.duration;
            existingAnimation.animationName = animation.animationName;
            std::cout << "Animation updated for target: " << targetId << " [Animation ID: " << animation.animationId << "]\n";
            return;
        }
    }

    // If the animation doesn't exist, add a new one
    animationList.push_back(animation);
    std::cout << "Animation added for target: " << targetId << " [Animation ID: " << animation.animationId << "]\n";
}

// Check if an animation is still active
bool GameDataManager::IsAnimationActive(unsigned int targetId, unsigned short animationId) {
    if (animations_.find(targetId) != animations_.end()) {
        for (const auto& animation : animations_[targetId]) {
            if (animation.animationId == animationId) {
                auto currentTime = std::chrono::steady_clock::now();
                if (currentTime - animation.startTime < animation.duration) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Remove expired animations
void GameDataManager::RemoveExpiredAnimations() {
    auto currentTime = std::chrono::steady_clock::now();
    for (auto& [targetId, animationList] : animations_) {
        animationList.erase(
            std::remove_if(animationList.begin(), animationList.end(),
                [currentTime](const Animation& animation) {
                    return currentTime - animation.startTime >= animation.duration;
                }),
            animationList.end());
    }
}

// Check if a player has a specific animation
bool GameDataManager::HasAnimation(unsigned int targetId, unsigned short animationId) {
    if (animations_.find(targetId) != animations_.end()) {
        for (const auto& animation : animations_[targetId]) {
            if (animation.animationId == animationId) {
                return true;
            }
        }
    }
    return false;
}

// Get all active animations for a player
std::vector<Animation> GameDataManager::GetActiveAnimations(unsigned int targetId) {
    std::vector<Animation> activeAnimations;
    if (animations_.find(targetId) != animations_.end()) {
        auto& animationList = animations_[targetId];
        for (const auto& animation : animationList) {
            auto currentTime = std::chrono::steady_clock::now();
            if (currentTime - animation.startTime < animation.duration) {
                activeAnimations.push_back(animation);
            }
        }
    }
    return activeAnimations;
}

// Log game events
void GameDataManager::LogEvent(const std::string& event) {
    eventLog_.push_back(event);
    std::cout << "Event Logged: " << event << "\n";
}

// Print all players for debugging
void GameDataManager::PrintAllPlayers() const {
    std::cout << "\n=== Player List ===\n";
    for (const auto& [serial, player] : players_) {
        std::cout << "Serial: " << serial << " | Name: " << player.Name
            << " | Position: (" << player.Position.X << ", " << player.Position.Y << ")\n";
    }
    std::cout << "===================\n\n";
}

// Print all animations for debugging
void GameDataManager::PrintAllAnimations() const {
    std::cout << "\n=== Animation List ===\n";
    for (const auto& [targetId, animationList] : animations_) {
        std::cout << "Target ID: " << targetId << "\n";
        for (const auto& animation : animationList) {
            std::cout << "  Animation ID: " << animation.animationId
                << " | Name: " << animation.animationName
                << " | Duration: " << animation.duration.count() << " seconds\n";
        }
    }
    std::cout << "======================\n\n";
}

