#pragma once
#include <mutex>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <shared_mutex>
#include "packet_structures.h"

class PacketReader
{
private:
    const BYTE *bodyData; // Pointer to the packet data
    size_t length;        // Length of the packet data
    mutable std::shared_mutex mutex;
    size_t position = 0;

public:
    explicit PacketReader(const packet &pkt) : bodyData(pkt.data), length(pkt.size()), position(0) {}

    PacketReader(const PacketReader &other)
    {
        std::unique_lock<std::shared_mutex> lock(other.mutex);
        bodyData = other.bodyData;
        length = other.length;
        position = other.position;
    }

    PacketReader &operator=(const PacketReader &other)
    {
        if (this != &other)
        {
            std::unique_lock<std::shared_mutex> lhs_lock(mutex, std::defer_lock);
            std::unique_lock<std::shared_mutex> rhs_lock(other.mutex, std::defer_lock);
            std::lock(lhs_lock, rhs_lock);

            bodyData = other.bodyData;
            length = other.length;
            position = other.position;
        }
        return *this;
    }

    size_t getPosition() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return position;
    }

    void setPosition(size_t newPos)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        if (newPos > length)
            throw std::out_of_range("Position out of range");
        position = newPos;
    }

    unsigned char readByte()
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        if (position >= length)
            throw std::out_of_range("Index out of range");
        return bodyData[position++];
    }

    template <typename T>
    T read()
    {
        static_assert(std::is_arithmetic<T>::value, "Read type must be arithmetic");
        std::unique_lock<std::shared_mutex> lock(mutex);
        size_t typeLength = sizeof(T);
        if (position + typeLength > length)
            throw std::out_of_range("Index out of range");

        T value = 0;
        for (size_t i = 0; i < typeLength; ++i)
        {
            value |= static_cast<T>(bodyData[position + i]) << (8 * (typeLength - 1 - i));
        }
        position += typeLength;
        if constexpr (std::is_same_v<T, bool>)
        {
            return value != 0;
        }
        return value;
    }

    std::vector<unsigned char> readBytes(size_t len)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        if (position + len > length)
            throw std::out_of_range("Index out of range");

        std::vector<unsigned char> buffer(bodyData + position, bodyData + position + len);
        position += len;
        return buffer;
    }

    std::string readString(size_t len)
    {
        auto buffer = readBytes(len);
        return std::string(buffer.begin(), buffer.end());
    }

    std::string readString8()
    {
        auto len = readByte();
        return readString(len);
    }

    bool canReadMore() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return position < length;
    }

    void reset()
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        position = 0;
    }
};