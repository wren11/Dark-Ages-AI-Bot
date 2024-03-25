#pragma once
#include "pch.h"
#include "Item.h"

class inventory_manager
{
private:
    std::vector<Item> items;

public:
    void AddItem(const Item &item)
    {
        items.push_back(item);
    }

    void RemoveItem(const byte slot)
    {
        auto it = std::find_if(items.begin(), items.end(), [slot](const Item &item)
                               { return item.InventorySlot == slot; });
        if (it != items.end())
        {
            items.erase(it);
        }
    }

    std::optional<Item> FindItemByName(const std::string &name) const
    {
        auto it = std::find_if(items.begin(), items.end(), [&name](const Item &item)
                               { return item.Name == name; });
        if (it != items.end())
        {
            return *it;
        }
        return std::nullopt;
    }

    std::optional<Item> FindItemBySlot(int slot) const
    {
        auto it = std::find_if(items.begin(), items.end(), [slot](const Item &item)
                               { return item.InventorySlot == slot; });
        if (it != items.end())
        {
            return *it;
        }
        return std::nullopt;
    }
};