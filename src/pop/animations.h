#pragma once
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <functional>

class Animation
{
public:
    int fromId;
    int targetId;
    int fromEffect;
    int targetEffect;
    int delay;
    int x;
    int y;

    Animation(int fromId, int targetId, int fromEffect, int targetEffect, int delay, int x, int y) : fromId(fromId), targetId(targetId),
                                                                                                     fromEffect(fromEffect),
                                                                                                     targetEffect(targetEffect),
                                                                                                     delay(delay), x(x),
                                                                                                     y(x)
    {
    }
};

class AnimationTiming
{
public:
    double longTimer;
    double shortTimer;
    bool longTimerActive;
    bool shortTimerActive;
    unsigned int targetId;

    AnimationTiming() : longTimer(20.5), shortTimer(8), longTimerActive(true), shortTimerActive(true), targetId(0) {}

    void update(double deltaTime)
    {
        if (longTimerActive)
        {
            longTimer -= deltaTime;
            if (longTimer <= 0)
            {
                longTimerActive = false;
                // Long timer expired logic here (if needed)
                std::cout << "Long timer expired." << std::endl;
            }
        }

        if (shortTimerActive)
        {
            shortTimer -= deltaTime;
            if (shortTimer <= 0)
            {
                shortTimerActive = false;
                // Short timer expired logic here (if needed)
                std::cout << "Short timer expired." << std::endl;
            }
        }
    }

    void resetLongTimer()
    {
        longTimer = 20.5;
        longTimerActive = true;
    }

    void resetShortTimer()
    {
        shortTimer = 8;
        shortTimerActive = true;
    }

    bool hasLongTimerExpired() const
    {
        return !longTimerActive;
    }

    bool hasShortTimerExpired() const
    {
        return !shortTimerActive;
    }

    double getLongTimer() const
    {
        return longTimer;
    }

    double getShortTimer() const
    {
        return shortTimer;
    }

    unsigned int getTarget() const
    {
        return targetId;
    }
};

class AnimationsManager
{
private:
    std::mutex animationsMutex;
    std::unordered_map<int, AnimationTiming> animations;

public:
    void addAnimation(const Animation &animation)
    {
        std::lock_guard<std::mutex> lock(animationsMutex);

        if (animation.targetEffect == 244)
        {
            auto &timing = animations[animation.targetId];
            timing.targetId = animation.targetId;

            timing.resetLongTimer();
            timing.resetShortTimer();
        }
    }

    auto ForEach(std::function<void(int targetId, const AnimationTiming &)> func) -> void
    {
        std::lock_guard<std::mutex> lock(animationsMutex);
        for (const auto &pair : animations)
        {
            func(pair.first, pair.second);
        }
    }

    void update(double deltaTime)
    {
        std::lock_guard<std::mutex> lock(animationsMutex);

        for (auto it = animations.begin(); it != animations.end();)
        {
            it->second.update(deltaTime);

            if (it->second.hasLongTimerExpired() && it->second.hasShortTimerExpired())
            {
                it = animations.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Update();
    void Clear();
};
