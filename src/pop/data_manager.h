#pragma once
#include "pch.h"

// Define position and direction
struct Position {
    unsigned short X;
    unsigned short Y;
    unsigned char FacingDirection;

    Position() : X(0), Y(0), FacingDirection(0) {}
    Position(unsigned short x, unsigned short y, unsigned char dir) : X(x), Y(y), FacingDirection(dir) {}
};

enum class Direction : unsigned char {
    North = 0,
    East = 1,
    South = 2,
    West = 3,
    Unknown = 4
};

// Define player structure
struct Player {
    unsigned int Serial;
    std::string Name;
    Position Position;
    unsigned short Head;
    unsigned short Body;
    unsigned short Arms;
    unsigned short Boots;
    unsigned short Armor;
    unsigned short Shield;
    unsigned short Weapon;
    unsigned short Acc1;
    unsigned short Acc2;
    unsigned short Acc3;
    unsigned short RestCloak;
    unsigned short Overcoat;
    unsigned char HeadColor;
    unsigned char BootColor;
    unsigned char Acc1Color;
    unsigned char Acc2Color;
    unsigned char SkinColor;
    unsigned char OvercoatColor;
    unsigned char FaceShape;
    unsigned char HideBool;
    unsigned char Unknown;
    unsigned char Unknown2;
    bool Hostile = false;
};

struct Animation {
    unsigned int targetId;
    unsigned short animationId;
    std::chrono::steady_clock::time_point startTime;
    std::chrono::seconds duration;
    std::string animationName;
    Animation() : targetId(0), animationId(0), startTime(std::chrono::steady_clock::now()), duration(0), animationName("") {}
};

constexpr std::chrono::seconds DionDuration = std::chrono::seconds(60);

class GameDataManager {
public:
    // Function to add or update a player's information
    void AddOrUpdatePlayer(unsigned int serial, const Player& player);

    // Function to get a player's information
    std::optional<Player> GetPlayer(unsigned int serial);

    // Function to add or update an animation
    void AddOrUpdateAnimation(unsigned int targetId, const Animation& animation);

    // Function to check if an animation is still active
    bool IsAnimationActive(unsigned int targetId, unsigned short animationId);

    // Function to remove an animation if it has expired
    void RemoveExpiredAnimations();

    // Function to check if a player has a specific animation
    bool HasAnimation(unsigned int targetId, unsigned short animationId);

    // Function to get all active animations
    std::vector<Animation> GetActiveAnimations(unsigned int targetId);

    // Function to log game events
    void LogEvent(const std::string& event);

    // Print all players for debugging
    void PrintAllPlayers() const;

    // Print all animations for debugging
    void PrintAllAnimations() const;

private:
    // Map to store player data by serial ID
    std::unordered_map<unsigned int, Player> players_;

    // Map to store animations by target ID
    std::unordered_map<unsigned int, std::vector<Animation>> animations_;

    // Log for game events
    std::vector<std::string> eventLog_;
};
