#pragma once
#include <vector>
#include <mutex>
#include <algorithm>
#include <optional>
#include <memory>
#include <functional>

#include "sprite.h"
#include "structures.h"

template <typename T, typename SerialType>
class GenericObjectManager
{
private:
    std::vector<std::shared_ptr<T>> objects;
    mutable std::mutex objectsMutex;

public:
    GenericObjectManager() = default;

    void AddOrUpdate(const SerialType &serial, const T &newData)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto it = std::find_if(objects.begin(), objects.end(),
                               [&serial](const std::shared_ptr<T> &obj)
                               { return obj->GetSerial() == serial; });

        if (it != objects.end())
        {
            (*it)->MergeUpdates(newData);
        }
        else
        {
            auto newObj = std::make_shared<T>(newData);
            objects.push_back(newObj);
        }
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        objects.clear();
    }

    std::optional<std::shared_ptr<T>> GetBySerial(const SerialType &serial)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto it = std::find_if(objects.begin(), objects.end(),
                               [&serial](const std::shared_ptr<T> &obj)
                               { return obj->GetSerial() == serial; });
        if (it != objects.end())
        {
            return *it;
        }
        return std::nullopt;
    }

    bool DeleteBySerial(const SerialType &serial)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto it = std::remove_if(objects.begin(), objects.end(),
                                 [&serial](const std::shared_ptr<T> &obj)
                                 { return obj->GetSerial() == serial; });
        if (it != objects.end())
        {
            objects.erase(it, objects.end());
            return true;
        }
        return false;
    }

    void ForEach(std::function<void(std::shared_ptr<T>)> action)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        for (auto &object : objects)
        {
            action(object);
        }
    }

    void MergeOrPrune(const std::vector<std::shared_ptr<T>> &updatedObjects)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        std::vector<std::shared_ptr<T>> tempObjects;

        for (const auto &updatedObject : updatedObjects)
        {
            auto it = std::find_if(objects.begin(), objects.end(),
                                   [&updatedObject](const std::shared_ptr<T> &obj)
                                   { return obj->GetSerial() == updatedObject->GetSerial(); });
            if (it != objects.end())
            {
                (*it)->MergeUpdates(*updatedObject);
                tempObjects.push_back(*it);
            }
            else
            {
                tempObjects.push_back(updatedObject);
            }
        }

        objects.swap(tempObjects);
    }

    size_t GetTotalCount()
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        return objects.size();
    }

    size_t GetTotalWithinRange(const Location &center, double range) const
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        return std::count_if(objects.begin(), objects.end(), [&center, range](const std::shared_ptr<T> &obj)
                             {
            double distance = std::sqrt(std::pow(obj->GetLocation().X - center.X, 2) + std::pow(obj->GetLocation().Y - center.Y, 2));
            return distance <= range; });
    }

    std::vector<std::shared_ptr<T>> GetObjectsWithinRange(const Location &center, double range = 12.0)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        std::vector<std::shared_ptr<T>> withinRange;

        std::copy_if(objects.begin(), objects.end(), std::back_inserter(withinRange),
                     [&center, range](const std::shared_ptr<T> &obj)
                     {
                         double distance = std::sqrt(std::pow(obj->GetLocation().X - center.X, 2) +
                                                     std::pow(obj->GetLocation().Y - center.Y, 2));
                         return distance <= range;
                     });
        return withinRange;
    }

    void RemoveObjectsOutsideRange(const Location &center, double range = 12.0)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto newEnd = std::remove_if(objects.begin(), objects.end(),
                                     [&center, range](const std::shared_ptr<T> &obj)
                                     {
                                         double distance = std::sqrt(std::pow(obj->GetLocation().X - center.X, 2) +
                                                                     std::pow(obj->GetLocation().Y - center.Y, 2));
                                         return distance > range;
                                     });
        objects.erase(newEnd, objects.end());
    }

    size_t GetTotalNextToLocation(const Location &location) const
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        return std::count_if(objects.begin(), objects.end(), [&location](const std::shared_ptr<T> &obj)
                             {
            auto objLoc = obj->GetLocation();
            return (std::abs(objLoc.X - location.X) <= 1 && std::abs(objLoc.Y - location.Y) <= 1) && !(objLoc.X == location.X && objLoc.Y == location.Y); });
    }

    std::optional<std::shared_ptr<T>> GetNearestFromLocation(const Location &location)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto nearestIt = std::min_element(objects.begin(), objects.end(), [&location](const std::shared_ptr<T> &a, const std::shared_ptr<T> &b)
                                          { return Distance(a->GetLocation(), location) < Distance(b->GetLocation(), location); });

        if (nearestIt != objects.end())
        {
            return *nearestIt;
        }
        return std::nullopt;
    }

    std::optional<std::shared_ptr<T>> GetFurthestFromLocation(const Location &location) const
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto furthestIt = std::max_element(objects.begin(), objects.end(), [&location](const std::shared_ptr<T> &a, const std::shared_ptr<T> &b)
                                           { return Distance(a->GetLocation(), location) < Distance(b->GetLocation(), location); });

        if (furthestIt != objects.end())
        {
            return *furthestIt;
        }
        return std::nullopt;
    }

    static double Distance(const Location &a, const Location &b)
    {
        return std::sqrt(std::pow(a.X - b.X, 2) + std::pow(a.Y - b.Y, 2));
    }

    bool GetAndApplyAction(const SerialType &serial, const std::function<void(T *)> &action)
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto it = std::find_if(objects.begin(), objects.end(),
                               [&serial](const std::shared_ptr<T> &obj)
                               { return obj->GetSerial() == serial; });
        if (it != objects.end())
        {
            T *rawPtr = it->get(); // Get the raw pointer from the shared_ptr
            action(rawPtr);        // Apply the action
            return true;
        }
        return false;
    }

    size_t GetObjectCount() const
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        return objects.size();
    }
};
