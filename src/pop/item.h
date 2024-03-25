#pragma once
#include "pch.h"

class Item
{
public:
    std::string Name;
    int InventorySlot;
    std::chrono::time_point<std::chrono::system_clock> NextUse;
    uint16_t Icon;
    uint8_t IconPal;
    uint32_t Amount = 1;
    uint8_t Stackable;
    uint32_t CurrentDurability;
    uint32_t MaximumDurability;
    bool Gone = false;
    bool IsIdentified = false;

    Item() : NextUse(std::chrono::system_clock::now()) {}

    std::string ToString() const
    {
        return Name;
    }
};