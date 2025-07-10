#pragma once
#include "pch.h"
#include "network_functions.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <array>
#include <functional>
#include <memory>

using BYTE = unsigned char;
using USHORT = unsigned short;

enum class Direction : BYTE
{
    North,
    East,
    South,
    West,
    None
};

struct LegendMark
{
    byte Icon;
    byte TextColor;
    std::string MarkID;
    std::string Mark;
};

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
        summary << "Marks Count: " << LegendMarks.size() << "\n";

        for (const auto &mark : LegendMarks)
        {
            summary << "Mark - ID: " << mark.MarkID << ", Mark: " << mark.Mark << "\n";
        }

        return summary.str();
    }
};

class Location
{
public:
    USHORT X;
    USHORT Y;
    Direction FacingDirection;

    Location(USHORT x = 0, USHORT y = 0, Direction facing = Direction::None) : X(x), Y(y), FacingDirection(facing) {}

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
        return (Y == target.Y && std::abs(X - target.X) <= 4) || (X == target.X && std::abs(Y - target.Y) <= 4);
    }

    Location predictLocation(int steps) const
    {
        Location result(*this);
        std::array<std::function<void(Location &, int)>, 4> moveFuncs = {{[](Location &loc, int s)
                                                                          { loc.Y -= s; },
                                                                          [](Location &loc, int s)
                                                                          { loc.Y += s; },
                                                                          [](Location &loc, int s)
                                                                          { loc.X += s; },
                                                                          [](Location &loc, int s)
                                                                          { loc.X -= s; }}};
        if (static_cast<int>(FacingDirection) < moveFuncs.size())
        {
            moveFuncs[static_cast<int>(FacingDirection)](result, steps);
        }
        return result;
    }

    bool isPredictedInLineOfSight(const Location &other, int steps) const
    {
        Location predictedLocation = other.predictLocation(steps);
        return isWithinFourSpacesStraight(predictedLocation);
    }

    Direction determineFacingDirection(const Location &target) const
    {
        if (X == target.X)
        {
            return Y > target.Y ? Direction::North : Direction::South;
        }
        else if (Y == target.Y)
        {
            return X > target.X ? Direction::West : Direction::East;
        }
        return FacingDirection;
    }

    void faceAndAmbush(const Location &other, int steps)
    {
        if (isPredictedInLineOfSight(other, steps))
        {
            Location predictedLocation = other.predictLocation(steps);
            FacingDirection = determineFacingDirection(predictedLocation);
            // Assuming game_function::send_to_server exists and takes a unique_ptr to a BYTE array
            auto message = std::make_unique<BYTE[]>(3);
            message[0] = 0x11;
            message[1] = static_cast<BYTE>(FacingDirection);
            message[2] = 0x00;
            game_function::send_to_server(std::move(message).get(), 3);
            // Assuming AMBUSH is a macro or function that performs the ambush
        }
    }

    Direction determineEscapeDirection(const Location &other, int steps) const
    {
        Location predictedLocation = other.predictLocation(steps);
        int deltaX = predictedLocation.X - X;
        int deltaY = predictedLocation.Y - Y;
        if (std::abs(deltaX) > std::abs(deltaY))
        {
            return deltaX > 0 ? Direction::West : Direction::East;
        }
        else if (std::abs(deltaY) > std::abs(deltaX))
        {
            return deltaY > 0 ? Direction::North : Direction::South;
        }
        return Direction::None;
    }

    Location move(Direction direction, int steps = 1) const
    {
        Location result(*this);
        std::array<std::function<void(Location &, int)>, 4> moveFuncs = {{[](Location &loc, int s)
                                                                          { loc.Y -= s; },
                                                                          [](Location &loc, int s)
                                                                          { loc.Y += s; },
                                                                          [](Location &loc, int s)
                                                                          { loc.X += s; },
                                                                          [](Location &loc, int s)
                                                                          { loc.X -= s; }}};
        if (static_cast<int>(direction) < moveFuncs.size())
        {
            moveFuncs[static_cast<int>(direction)](result, steps);
        }
        return result;
    }

    Direction approachWithoutLOS(const Location &target, const Location &threat, int steps)
    {
        Location predictedThreatLocation = threat.predictLocation(steps);
        std::vector<Direction> possibleDirections = {Direction::North, Direction::South, Direction::East, Direction::West};
        std::vector<Location> possibleMoves;
        std::transform(possibleDirections.begin(), possibleDirections.end(), std::back_inserter(possibleMoves), [&](Direction d)
                       { return this->move(d); });

        possibleMoves.erase(std::remove_if(possibleMoves.begin(), possibleMoves.end(), [&](const Location &loc)
                                           { return predictedThreatLocation.isPredictedInLineOfSight(loc, steps); }),
                            possibleMoves.end());

        auto closestMove = std::min_element(possibleMoves.begin(), possibleMoves.end(), [&](const Location &a, const Location &b)
                                            { return Location::Distance(a, target) < Location::Distance(b, target); });

        if (closestMove == possibleMoves.end())
        {
            return Direction::None;
        }

        if (closestMove->X > this->X)
            return Direction::East;
        if (closestMove->X < this->X)
            return Direction::West;
        if (closestMove->Y > this->Y)
            return Direction::South;
        if (closestMove->Y < this->Y)
            return Direction::North;

        return Direction::None;
    }

    Location canCharge() const
    {
        return predictLocation(5);
    }

    bool canAmbush(const Location &target) const
    {
        return isPredictedInLineOfSight(target, 3) && Distance(*this, target) <= 3;
    }

    std::string reachTargetWithSkills(const Location &target)
    {
        Location chargeLocation = canCharge();
        bool canCharge = Distance(chargeLocation, target) < Distance(*this, target);

        if (canAmbush(target))
        {
            return "Use Ambush to reach the target.";
        }
        else if (canCharge)
        {
            return "Use Charge to get closer to the target.";
        }

        return "Cannot reach the target with skills from the current position.";
    }

    std::string strategicMove(const Location &target, const Location &opponent)
    {
        Location myPredictedCharge = canCharge();
        Location opponentPredictedCharge = opponent.canCharge();

        bool canIAmbush = canAmbush(target);
        bool opponentCanAmbushMe = opponent.canAmbush(*this);

        bool safeFromCharge = !opponentPredictedCharge.isPredictedInLineOfSight(*this, 5);
        bool safeFromAmbush = !opponentCanAmbushMe;

        if (canIAmbush && safeFromCharge && safeFromAmbush)
        {
            return "Ambush to reach the target safely, avoiding the opponent's LOS, charge, and ambush.";
        }
        else if (Distance(myPredictedCharge, target) < Distance(*this, target) && safeFromCharge && safeFromAmbush)
        {
            return "Charge to get closer to the target safely, ensuring we're out of the opponent's LOS and safe from their ambush.";
        }

        Direction moveDirection = determineEscapeDirection(opponent, 1);
        if (moveDirection != Direction::None && safeFromAmbush)
        {
            return "Move " + directionToString(moveDirection) + " to safely approach the target, staying vigilant of the opponent's potential actions.";
        }

        return "Stay put to carefully avoid the opponent's LOS, charge, and ambush. Wait for a more advantageous position or for the opponent to make a move.";
    }

    bool isInFront(const Location &target, Direction direction) const
    {
        switch (direction)
        {
        case Direction::North:
            return target.Y < Y;
        case Direction::South:
            return target.Y > Y;
        case Direction::East:
            return target.X > X;
        case Direction::West:
            return target.X < X;
        case Direction::None:
            break;
        default:
            return false;
        }
    }
};