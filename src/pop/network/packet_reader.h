/**
 * @file packet_reader.h
 * @brief Utility class for reading data from packets
 */
#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include "packet.h"

namespace network {

/**
 * @brief Exception thrown when packet reading operations fail
 */
class PacketReadException : public std::runtime_error {
public:
    explicit PacketReadException(const std::string& message) 
        : std::runtime_error(message) {}
};

/**
 * @brief Utility class for reading structured data from packets
 * 
 * Provides easy-to-use methods for extracting different data types
 * from a packet with position tracking and bounds checking.
 */
class PacketReader {
public:
    /**
     * @brief Construct a reader for a packet
     * @param packet The packet to read from
     */
    explicit PacketReader(const Packet& packet);
    
    /**
     * @brief Construct a reader from a vector of bytes
     * @param data The vector containing packet data
     */
    explicit PacketReader(const std::vector<unsigned char>& data);
    
    /**
     * @brief Construct a reader from a raw byte array
     * @param data Pointer to the packet data
     * @param length Length of the packet data
     */
    PacketReader(const uint8_t* data, size_t length);
    
    /**
     * @brief Read a single byte from the current position
     * @return Byte value
     * @throws PacketReadException if reading beyond packet boundary
     */
    std::uint8_t readByte();
    
    /**
     * @brief Read a value of specified type from the current position
     * @tparam T Type of value to read
     * @return Value read from packet
     * @throws PacketReadException if reading beyond packet boundary
     */
    template <typename T>
    T read() {
        // Check if we have enough data for this type
        if (position_ + sizeof(T) > packet_.size()) {
            throw PacketReadException("Attempt to read beyond packet boundary");
        }
        
        // Read value
        T value = *reinterpret_cast<const T*>(packet_.data() + position_);
        
        // Advance position
        position_ += sizeof(T);
        
        return value;
    }
    
    /**
     * @brief Read a string with 8-bit length prefix
     * @return String value
     * @throws PacketReadException if reading beyond packet boundary
     */
    std::string readString8();
    
    /**
     * @brief Read a string with 16-bit length prefix
     * @return String value
     * @throws PacketReadException if reading beyond packet boundary
     */
    std::string readString16();
    
    /**
     * @brief Read a fixed-length string
     * @param length Number of bytes to read
     * @return String value
     * @throws PacketReadException if reading beyond packet boundary
     */
    std::string readString(std::size_t length);
    
    /**
     * @brief Read a specified number of bytes
     * @param length Number of bytes to read
     * @return Vector containing the read bytes
     * @throws PacketReadException if reading beyond packet boundary
     */
    std::vector<std::uint8_t> readBytes(std::size_t length);
    
    /**
     * @brief Skip specified number of bytes
     * @param bytes Number of bytes to skip
     * @throws PacketReadException if skipping beyond packet boundary
     */
    void skip(std::size_t bytes);
    
    /**
     * @brief Set the current read position
     * @param position New position
     * @throws PacketReadException if position is beyond packet boundary
     */
    void setPosition(std::size_t position);
    
    /**
     * @brief Get the current read position
     * @return Current position
     */
    [[nodiscard]] std::size_t getPosition() const;
    
    /**
     * @brief Get the remaining bytes in the packet
     * @return Number of unread bytes
     */
    [[nodiscard]] std::size_t getRemainingBytes() const;
    
    /**
     * @brief Get the packet being read
     * @return Reference to the packet
     */
    [[nodiscard]] const Packet& getPacket() const;
    
    /**
     * @brief Peek at the next byte without advancing position
     * @return Next byte value
     * @throws PacketReadException if peeking beyond packet boundary
     */
    [[nodiscard]] std::uint8_t peekByte() const;
    
    /**
     * @brief Peek at a byte at a specific offset
     * @param offset Offset into the packet
     * @return Byte value at offset
     * @throws PacketReadException if offset is beyond packet boundary
     */
    [[nodiscard]] std::uint8_t peekByteAt(std::size_t offset) const;
    
    /**
     * @brief Check if there is more data to read
     * @return True if unread data remains
     */
    [[nodiscard]] bool hasMoreData() const;

private:
    Packet packet_;      // Copy of the packet being read
    std::size_t position_;  // Current read position
};

} // namespace network
