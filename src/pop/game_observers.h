#pragma once
#include "pch.h"

class IWeaponChangeObserver
{
public:
    virtual ~IWeaponChangeObserver() = default;
    virtual void OnWeaponChange(const std::string &newWeaponName) = 0;
};
