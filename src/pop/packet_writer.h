#pragma once
#include "pch.h"
#include "network_functions.h"

class PacketWriter
{
private:
    std::vector<BYTE> data;
    mutable std::shared_mutex mutex;

public:
    PacketWriter() = default;

    PacketWriter(const PacketWriter& other)
    {
        std::shared_lock<std::shared_mutex> lock(other.mutex);
        data = other.data;
    }

    PacketWriter& operator=(const PacketWriter& other)
    {
        if (this != &other)
        {
            std::unique_lock<std::shared_mutex> lhs_lock(mutex, std::defer_lock);
            std::shared_lock<std::shared_mutex> rhs_lock(other.mutex, std::defer_lock);
            std::lock(lhs_lock, rhs_lock);
            data = other.data;
        }
        return *this;
    }

    template <typename T>
    void write(const T& value)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "Write type must be arithmetic or enum");
        std::unique_lock<std::shared_mutex> lock(mutex);
        size_t typeLength = sizeof(T);
        BYTE valueBytes[sizeof(T)];

        // Convert value to bytes in network byte order (big-endian)
        for (size_t i = 0; i < typeLength; ++i)
        {
            valueBytes[i] = static_cast<BYTE>((value >> (8 * (typeLength - 1 - i))) & 0xFF);
        }

        // Append the bytes to the packet data
        data.insert(data.end(), valueBytes, valueBytes + typeLength);
    }

    void writeBytes(const std::vector<BYTE>& bytes)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        data.insert(data.end(), bytes.begin(), bytes.end());
    }

    void writeString(const std::string& str)
    {
        writeBytes(std::vector<BYTE>(str.begin(), str.end()));
    }

    void writeString8(const std::string& str)
    {
        if (str.length() > 255)
            throw std::out_of_range("String length exceeds maximum for writeString8");
        write<BYTE>(static_cast<BYTE>(str.length()));
        writeString(str);
    }

    void sendToServer()
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        if (!data.empty())
        {
            printBytesHex();
            game_function::send_to_server(data.data(), static_cast<int>(data.size()));
        }
    }

    void printBytesHex() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        for (size_t i = 0; i < data.size(); ++i)
        {
            std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
            if (i < data.size() - 1)
            {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }

    void reset()
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        data.clear();
    }

    size_t getSize() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return data.size();
    }
};