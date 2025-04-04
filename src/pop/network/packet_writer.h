/**
 * @file packet_writer.h
 * @brief Utility class for writing data to packets
 */
#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include "packet.h"

namespace network {

/**
 * @brief Exception thrown when packet writing operations fail
 */
class PacketWriteException : public std::runtime_error {
public:
    explicit PacketWriteException(const std::string& message) 
        : std::runtime_error(message) {}
};

/**
 * @brief Utility class for writing structured data to packets
 * 
 * Provides methods for building packets with different data types.
 * Uses a buffer that automatically grows as needed.
 */
class PacketWriter {
public:
    /**
     * @brief Create a new packet writer with default capacity
     */
    PacketWriter();
    
    /**
     * @brief Create a new packet writer with specified capacity
     * @param initialCapacity Initial buffer capacity in bytes
     */
    explicit PacketWriter(std::size_t initialCapacity);
    
    /**
     * @brief Create a new packet writer with default capacity and initialize with packet type
     * @param packetType The packet type to initialize with
     */
    explicit PacketWriter(std::uint8_t packetType);
    
    /**
     * @brief Create a new packet writer with specified capacity and initialize with packet type
     * @param packetType The packet type to initialize with
     * @param initialCapacity Initial buffer capacity in bytes
     */
    PacketWriter(std::uint8_t packetType, std::size_t initialCapacity);
    
    /**
     * @brief Write a single byte
     * @param value Byte value to write
     * @return Reference to this writer for chaining
     */
    PacketWriter& writeByte(std::uint8_t value);
    
    /**
     * @brief Write a value of specified type
     * @tparam T Type of value to write
     * @param value Value to write
     * @return Reference to this writer for chaining
     */
    template <typename T>
    PacketWriter& write(T value) {
        // Ensure we have enough space
        ensureCapacity(position_ + sizeof(T));
        
        // Write the value
        *reinterpret_cast<T*>(buffer_.data() + position_) = value;
        
        // Advance position
        position_ += sizeof(T);
        
        return *this;
    }
    
    /**
     * @brief Write a string with 8-bit length prefix
     * @param value String to write
     * @return Reference to this writer for chaining
     */
    PacketWriter& writeString8(const std::string& value);
    
    /**
     * @brief Write a string with 16-bit length prefix
     * @param value String to write
     * @return Reference to this writer for chaining
     */
    PacketWriter& writeString16(const std::string& value);
    
    /**
     * @brief Write a fixed-length string, truncating or padding as needed
     * @param value String to write
     * @param length Fixed length to write
     * @param padByte Byte to use for padding if string is shorter than length
     * @return Reference to this writer for chaining
     */
    PacketWriter& writeString(const std::string& value, std::size_t length, std::uint8_t padByte = 0);
    
    /**
     * @brief Write bytes from another buffer
     * @param data Pointer to data to write
     * @param length Number of bytes to write
     * @return Reference to this writer for chaining
     */
    PacketWriter& writeBytes(const std::uint8_t* data, std::size_t length);
    
    /**
     * @brief Create a packet from the current buffer
     * @return Newly created packet
     */
    [[nodiscard]] Packet createPacket() const;
    
    /**
     * @brief Send the packet to the server using NetworkInterface
     * @return True if packet was sent successfully
     */
    [[nodiscard]] bool sendToServer() const;
    
    /**
     * @brief Send the packet to the client using NetworkInterface
     * @return True if packet was sent successfully
     */
    [[nodiscard]] bool sendToClient() const;
    
    /**
     * @brief Get the raw data from the packet buffer
     * @return Vector containing a copy of the packet data
     */
    [[nodiscard]] std::vector<std::uint8_t> getData() const;
    
    /**
     * @brief Get the current size of the buffer
     * @return Buffer size in bytes
     */
    [[nodiscard]] std::size_t size() const;
    
    /**
     * @brief Reset the writer to start a new packet
     */
    void reset();
    
    /**
     * @brief Reset the writer and initialize with a new packet type
     * @param packetType The packet type to initialize with
     */
    void resetWithType(std::uint8_t packetType);

private:
    /**
     * @brief Ensure the buffer has enough capacity
     * @param requiredCapacity Minimum required capacity
     */
    void ensureCapacity(std::size_t requiredCapacity);

    std::vector<std::uint8_t> buffer_;  // Buffer for packet data
    std::size_t position_;              // Current write position
};

} // namespace network
