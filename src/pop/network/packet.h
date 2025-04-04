/**
 * @file packet.h
 * @brief Defines the Packet class for network packet representation and handling
 */
#pragma once

#include <cstddef>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <memory>
#include <vector>
#include <iostream>
#include <type_traits>

namespace network {

/**
 * @brief Represents a network packet with data and length
 * 
 * Provides RAII semantics for packet data management with move semantics
 * for efficient handling of packet data.
 */
class Packet {
public:
    // Type definitions for clarity
    using ByteType = std::uint8_t;
    using BytePointer = std::unique_ptr<ByteType[]>;
    
    /**
     * @brief Constructs a packet from raw byte array
     * @param data Pointer to raw byte data (ownership not transferred)
     * @param length Number of bytes in the packet
     */
    Packet(ByteType* data, size_t length);
    
    /**
     * @brief Copy constructor
     * @param other Packet to copy from
     */
    Packet(const Packet& other);
    
    /**
     * @brief Copy assignment operator
     * @param other Packet to copy from
     * @return Reference to this packet
     */
    Packet& operator=(const Packet& other);
    
    /**
     * @brief Move constructor
     */
    Packet(Packet&& other) noexcept = default;
    
    /**
     * @brief Move assignment operator
     */
    Packet& operator=(Packet&& other) noexcept = default;
    
    /**
     * @brief Access packet data at specified index
     * @param index Zero-based index into packet data
     * @return Byte value at specified index
     * @throws std::out_of_range if index is out of bounds
     */
    [[nodiscard]] ByteType operator[](size_t index) const;
    
    /**
     * @brief Get size of packet in bytes
     * @return Number of bytes in packet
     */
    [[nodiscard]] size_t size() const noexcept;
    
    /**
     * @brief Output packet contents as hexadecimal string
     */
    void printHex() const;
    
    /**
     * @brief Get raw pointer to packet data
     * @return Const pointer to beginning of packet data
     */
    [[nodiscard]] const ByteType* data() const noexcept;

private:
    BytePointer data_;  // Packet data buffer
    size_t length_;     // Length of packet in bytes
};

/**
 * @brief Send a packet to the network
 * @param packet Packet to send
 */
void sendPacket(const Packet& packet);

/**
 * @brief Create a packet from individual bytes
 * @tparam Bytes Variadic template for bytes
 * @param bytes Individual bytes to include in packet
 * @return Constructed packet
 */
template <typename... Bytes>
Packet createPacket(Bytes... bytes) {
    static_assert((... && std::is_integral_v<Bytes>), "All arguments must be integral types");
    std::vector<Packet::ByteType> data = {static_cast<Packet::ByteType>(bytes)...};
    return Packet(data.data(), data.size());
}

} // namespace network
