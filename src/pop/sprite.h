#pragma once
#include "pch.h"
#include "structures.h"
#include "animations.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

enum class Direction : BYTE;
class Sprite;

enum class NpcType
{
    Mundane,
    Item
};

class SpriteObserver
{
public:
    virtual ~SpriteObserver() = default;
    virtual void OnSpriteMoved(const Sprite &sprite);
    virtual void OnSpriteChanged(const Sprite &sprite);
};

inline void SpriteObserver::OnSpriteMoved(const Sprite &sprite)
{
}

inline void SpriteObserver::OnSpriteChanged(const Sprite &sprite)
{
}

class ObserverManager
{
    std::vector<std::weak_ptr<SpriteObserver>> observers;

public:
    void AttachObserver(const std::shared_ptr<SpriteObserver> &observer)
    {
        observers.emplace_back(observer);
    }

    void DetachObserver(const std::shared_ptr<SpriteObserver> &observer)
    {
        observers.erase(std::remove_if(observers.begin(), observers.end(),
                                       [&observer](const std::weak_ptr<SpriteObserver> &wptr)
                                       {
                                           return wptr.expired() || wptr.lock() == observer;
                                       }),
                        observers.end());
    }

    template <typename Action>
    void NotifyObservers(Action action) const
    {
        for (const auto &wptr : observers)
        {
            if (auto sptr = wptr.lock())
            {
                action(sptr);
            }
        }
    }
};

class Sprite
{
    USHORT xCoord;
    USHORT yCoord;
    UINT32 serial;
    USHORT image;
    BYTE color;
    Direction direction;
    NpcType type;
    std::string name;
    AnimationsManager animations;
    ObserverManager observerManager;

public:
    Sprite(USHORT x, USHORT y, UINT32 id, USHORT img)
        : xCoord(x), yCoord(y), serial(id), image(img), color(0), direction(), type(NpcType::Mundane) {}

    Sprite(const Sprite &) = delete;
    Sprite &operator=(const Sprite &) = delete;

    Sprite(Sprite &&) noexcept = default;
    Sprite &operator=(Sprite &&) noexcept = default;

    UINT32 GetSerial() const { return serial; }
    USHORT GetXCoord() const { return xCoord; }
    USHORT GetYCoord() const { return yCoord; }

    void Update()
    {
        animations.Update();
        observerManager.NotifyObservers([this](const std::shared_ptr<SpriteObserver> &observer)
                                        { observer->OnSpriteChanged(*this); });
    }

    void Refresh()
    {
        animations.Clear();
    }

    void Move(USHORT newX, USHORT newY)
    {
        xCoord = newX;
        yCoord = newY;
        observerManager.NotifyObservers([this](const std::shared_ptr<SpriteObserver> &observer)
                                        { observer->OnSpriteMoved(*this); });
    }

    void AttachObserver(const std::shared_ptr<SpriteObserver> &observer)
    {
        observerManager.AttachObserver(observer);
    }

    void DetachObserver(const std::shared_ptr<SpriteObserver> &observer)
    {
        observerManager.DetachObserver(observer);
    }

    void MergeUpdates(const Sprite &updatedSprite)
    {
        this->xCoord = updatedSprite.xCoord;
        this->yCoord = updatedSprite.yCoord;
        this->direction = updatedSprite.direction;
    }
};