#pragma once
#include "pch.h"
#include <iostream>
#include <deque>
#include <chrono>
#include <algorithm>
#include <memory>

enum class Elements
{
    None,
    Fire,
    Water,
    Wind,
    Earth,
    Light,
    Dark
};

struct StatsSnapshot
{
    char Level, Ability, Str, Int, Wis, Con, Dex, AvailablePoints, AttackElement2, DefenseElement2, MailAndParcel, Damage, Hit;
    bool availablePoints;
    unsigned int MaximumHP, MaximumMP, CurrentHP, CurrentMP, Experience, ToNextLevel, AbilityExp, ToNextAbility, Gold;
    unsigned short MaximumWeight, CurrentWeight, BitMask;
    signed char ArmorClass;
    Elements AttackElement, DefenseElement;
    int oldgold;
    char MagicResistance;
    std::chrono::steady_clock::time_point timestamp;

    StatsSnapshot() : Level(0), Ability(0), Str(0), Int(0), Wis(0), Con(0), Dex(0), AvailablePoints(0), AttackElement2(0), DefenseElement2(0), MailAndParcel(0),
                      Damage(0), Hit(0), availablePoints(false), MaximumHP(0), MaximumMP(0), CurrentHP(0), CurrentMP(0), Experience(0), ToNextLevel(0), AbilityExp(0), ToNextAbility(0), Gold(0),
                      MaximumWeight(0), CurrentWeight(0), BitMask(0), ArmorClass(0), AttackElement(Elements::None), DefenseElement(Elements::None), oldgold(0), MagicResistance(0), timestamp(std::chrono::steady_clock::now()) {}
};

class StatisticsManager
{
private:
    std::shared_ptr<StatsSnapshot> currentStats;
    std::deque<std::shared_ptr<StatsSnapshot>> history;

    void trimHistory()
    {
        auto now = std::chrono::steady_clock::now();
        history.erase(std::remove_if(history.begin(), history.end(), [now](const std::shared_ptr<StatsSnapshot> &snapshot)
                                     { return std::chrono::duration_cast<std::chrono::seconds>(now - snapshot->timestamp).count() > 10; }),
                      history.end());
    }

public:
    StatisticsManager() : currentStats(std::make_shared<StatsSnapshot>()) {}

    void updateStats(const StatsSnapshot &newStats)
    {
        currentStats = std::make_shared<StatsSnapshot>(newStats);
        currentStats->timestamp = std::chrono::steady_clock::now();
        history.push_back(currentStats);
        trimHistory();
    }

    void printCurrentStats() const
    {
        std::cout << "---------------------------------------------\n";
        std::cout << "| Field            | Current Value         |\n";
        std::cout << "---------------------------------------------\n";
        auto printField = [](const std::string &name, auto value)
        {
            std::cout << "| " << std::left << std::setw(17) << name << "| " << value << "\n";
        };

        printField("Level", static_cast<int>(currentStats->Level));
        printField("Ability", static_cast<int>(currentStats->Ability));
        printField("MaximumHP", currentStats->MaximumHP);
        printField("MaximumMP", currentStats->MaximumMP);
        printField("Str", static_cast<int>(currentStats->Str));
        printField("Int", static_cast<int>(currentStats->Int));
        printField("Wis", static_cast<int>(currentStats->Wis));
        printField("Con", static_cast<int>(currentStats->Con));
        printField("Dex", static_cast<int>(currentStats->Dex));
        printField("AvailablePoints", static_cast<int>(currentStats->AvailablePoints));
        printField("MaximumWeight", currentStats->MaximumWeight);
        printField("CurrentWeight", currentStats->CurrentWeight);
        printField("CurrentHP", currentStats->CurrentHP);
        printField("CurrentMP", currentStats->CurrentMP);
        printField("Experience", currentStats->Experience);
        printField("ToNextLevel", currentStats->ToNextLevel);
        printField("AbilityExp", currentStats->AbilityExp);
        printField("ToNextAbility", currentStats->ToNextAbility);
        printField("Gold", currentStats->Gold);
        printField("BitMask", static_cast<int>(currentStats->BitMask));
        printField("AttackElement2", static_cast<int>(currentStats->AttackElement2));
        printField("DefenseElement2", static_cast<int>(currentStats->DefenseElement2));
        printField("MailAndParcel", static_cast<int>(currentStats->MailAndParcel));
        printField("AttackElement", static_cast<int>(currentStats->AttackElement));
        printField("DefenseElement", static_cast<int>(currentStats->DefenseElement));
        printField("MagicResistance", static_cast<int>(currentStats->MagicResistance));
        printField("ArmorClass", static_cast<int>(currentStats->ArmorClass));
        printField("Damage", static_cast<int>(currentStats->Damage));
        printField("Hit", static_cast<int>(currentStats->Hit));
        std::cout << "---------------------------------------------\n";
    }

    void printHistory() const
    {
        for (const auto &snapshot : history)
        {
            std::cout << "Timestamp: " << std::chrono::duration_cast<std::chrono::seconds>(snapshot->timestamp.time_since_epoch()).count() << " seconds since epoch\n";
        }
    }
};