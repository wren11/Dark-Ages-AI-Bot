#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <sstream>

using byte = unsigned char;
using uint16_t = unsigned short;

enum class Direction : byte
{
    North,
    East,
    South,
    West,
    None
};

// Simple legend mark for player info
struct LegendMark
{
    byte Icon;
    byte TextColor;
    std::string MarkID;
    std::string Mark;
};

// Player legend/bio information
struct Legend
{
    byte Nation;
    std::string GuildRank;
    std::string Title;
    std::string GroupList;
    std::string Notes;
    byte Path;
    bool Medenian;
    bool Master;
    std::string Class;
    std::string Guild;
    std::vector<LegendMark> LegendMarks;

    std::string formatSummary() const
    {
        std::ostringstream summary;
        summary << "Nation: " << static_cast<int>(Nation) << "\n";
        summary << "Guild Rank: " << GuildRank << "\n";
        summary << "Title: " << Title << "\n";
        summary << "Group List: " << GroupList << "\n";
        summary << "Notes: " << Notes << "\n";
        summary << "Path: " << static_cast<int>(Path) << "\n";
        summary << "Medenian: " << (Medenian ? "Yes" : "No") << "\n";
        summary << "Master: " << (Master ? "Yes" : "No") << "\n";
        summary << "Class: " << Class << "\n";
        summary << "Guild: " << Guild << "\n";
        return summary.str();
    }
};

// Core location class for game positioning
class Location
{
public:
    uint16_t X;
    uint16_t Y;
    Direction FacingDirection;

    Location(uint16_t x = 0, uint16_t y = 0, Direction facing = Direction::None) : X(x), Y(y), FacingDirection(facing) {}

    std::string directionToString(Direction direction) const
    {
        static const std::array<std::pair<Direction, std::string>, 5> directionMap = {{{Direction::North, "North"},
                                                                                       {Direction::East, "East"},
                                                                                       {Direction::South, "South"},
                                                                                       {Direction::West, "West"},
                                                                                       {Direction::None, "None"}}};
        auto it = std::find_if(directionMap.begin(), directionMap.end(), [direction](const auto &pair)
                               { return pair.first == direction; });
        return it != directionMap.end() ? it->second : "Unknown Direction";
    }

    static double Distance(const Location &a, const Location &b)
    {
        return std::hypot(a.X - b.X, a.Y - b.Y);
    }

    bool isWithinFourSpacesStraight(const Location &target) const
    {
        return (Y == target.Y && std::abs(static_cast<int>(X) - static_cast<int>(target.X)) <= 4) || 
               (X == target.X && std::abs(static_cast<int>(Y) - static_cast<int>(target.Y)) <= 4);
    }

    Direction determineFacingDirection(const Location &target) const
    {
        int deltaX = static_cast<int>(target.X) - static_cast<int>(X);
        int deltaY = static_cast<int>(target.Y) - static_cast<int>(Y);

        if (std::abs(deltaX) > std::abs(deltaY))
        {
            return (deltaX > 0) ? Direction::East : Direction::West;
        }
        else
        {
            return (deltaY > 0) ? Direction::South : Direction::North;
        }
    }

    Location move(Direction direction, int steps = 1) const
    {
        Location newLocation(*this);
        
        switch (direction)
        {
            case Direction::North:
                newLocation.Y = static_cast<uint16_t>(std::max(0, static_cast<int>(Y) - steps));
                break;
            case Direction::South:
                newLocation.Y = static_cast<uint16_t>(Y + steps);
                break;
            case Direction::East:
                newLocation.X = static_cast<uint16_t>(X + steps);
                break;
            case Direction::West:
                newLocation.X = static_cast<uint16_t>(std::max(0, static_cast<int>(X) - steps));
                break;
            default:
                break;
        }
        
        newLocation.FacingDirection = direction;
        return newLocation;
    }
};