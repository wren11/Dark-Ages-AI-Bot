/**
 * @file packet_writer.cpp
 * @brief Implementation of the PacketWriter class
 */
#include "packet_writer.h"
#include "../utils/logging.h"
#include "../game/network_interface.h"
#include <algorithm>

namespace network {

// Default initial capacity (256 bytes)
constexpr std::size_t DEFAULT_CAPACITY = 256;

PacketWriter::PacketWriter() 
    : buffer_(DEFAULT_CAPACITY), position_(0) {
}

PacketWriter::PacketWriter(std::size_t initialCapacity)
    : buffer_(initialCapacity > 0 ? initialCapacity : DEFAULT_CAPACITY), position_(0) {
}

PacketWriter::PacketWriter(std::uint8_t packetType)
    : buffer_(DEFAULT_CAPACITY), position_(0) {
    // Initialize with packet type
    writeByte(packetType);
}

PacketWriter::PacketWriter(std::uint8_t packetType, std::size_t initialCapacity)
    : buffer_(initialCapacity > 0 ? initialCapacity : DEFAULT_CAPACITY), position_(0) {
    // Initialize with packet type
    writeByte(packetType);
}

PacketWriter& PacketWriter::writeByte(std::uint8_t value) {
    try {
        ensureCapacity(position_ + 1);
        buffer_[position_++] = value;
        return *this;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in writeByte: " + std::string(e.what()));
        throw PacketWriteException("Error in writeByte: " + std::string(e.what()));
    }
}

PacketWriter& PacketWriter::writeString8(const std::string& value) {
    try {
        // Check if string is too long for 8-bit length
        if (value.length() > 255) {
            utils::Logging::warning("String too long for 8-bit length prefix, truncating");
            std::string truncated = value.substr(0, 255);
            
            // Write length byte
            writeByte(static_cast<std::uint8_t>(truncated.length()));
            
            // Write string data
            return writeBytes(reinterpret_cast<const std::uint8_t*>(truncated.data()), truncated.length());
        }
        
        // Write length byte
        writeByte(static_cast<std::uint8_t>(value.length()));
        
        // Write string data
        return writeBytes(reinterpret_cast<const std::uint8_t*>(value.data()), value.length());
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in writeString8: " + std::string(e.what()));
        throw PacketWriteException("Error in writeString8: " + std::string(e.what()));
    }
}

PacketWriter& PacketWriter::writeString16(const std::string& value) {
    try {
        // Check if string is too long for 16-bit length
        if (value.length() > 65535) {
            utils::Logging::warning("String too long for 16-bit length prefix, truncating");
            std::string truncated = value.substr(0, 65535);
            
            // Write length word
            write<std::uint16_t>(static_cast<std::uint16_t>(truncated.length()));
            
            // Write string data
            return writeBytes(reinterpret_cast<const std::uint8_t*>(truncated.data()), truncated.length());
        }
        
        // Write length word
        write<std::uint16_t>(static_cast<std::uint16_t>(value.length()));
        
        // Write string data
        return writeBytes(reinterpret_cast<const std::uint8_t*>(value.data()), value.length());
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in writeString16: " + std::string(e.what()));
        throw PacketWriteException("Error in writeString16: " + std::string(e.what()));
    }
}

PacketWriter& PacketWriter::writeString(const std::string& value, std::size_t length, std::uint8_t padByte) {
    try {
        ensureCapacity(position_ + length);
        
        // Copy either the entire string or as much as fits in the fixed length
        const std::size_t bytesToCopy = std::min(value.length(), length);
        std::copy_n(value.data(), bytesToCopy, reinterpret_cast<char*>(buffer_.data() + position_));
        
        // Pad remaining space if needed
        if (bytesToCopy < length) {
            std::fill_n(buffer_.data() + position_ + bytesToCopy, length - bytesToCopy, padByte);
        }
        
        position_ += length;
        return *this;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in writeString: " + std::string(e.what()));
        throw PacketWriteException("Error in writeString: " + std::string(e.what()));
    }
}

PacketWriter& PacketWriter::writeBytes(const std::uint8_t* data, std::size_t length) {
    try {
        if (data == nullptr) {
            utils::Logging::error("Null pointer provided to writeBytes");
            throw PacketWriteException("Null pointer provided to writeBytes");
        }
        
        ensureCapacity(position_ + length);
        std::copy_n(data, length, buffer_.data() + position_);
        position_ += length;
        
        return *this;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in writeBytes: " + std::string(e.what()));
        throw PacketWriteException("Error in writeBytes: " + std::string(e.what()));
    }
}

Packet PacketWriter::createPacket() const {
    try {
        // Create a packet from the buffer up to the current position
        return Packet(buffer_.data(), position_);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in createPacket: " + std::string(e.what()));
        throw PacketWriteException("Error in createPacket: " + std::string(e.what()));
    }
}

bool PacketWriter::sendToServer() const {
    try {
        Packet packet = createPacket();
        return game::NetworkInterface::getInstance().sendToServer(packet);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in sendToServer: " + std::string(e.what()));
        return false;
    }
}

bool PacketWriter::sendToClient() const {
    try {
        Packet packet = createPacket();
        return game::NetworkInterface::getInstance().sendToClient(packet);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error in sendToClient: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::uint8_t> PacketWriter::getData() const {
    std::vector<std::uint8_t> result(buffer_.begin(), buffer_.begin() + position_);
    return result;
}

std::size_t PacketWriter::size() const {
    return position_;
}

void PacketWriter::reset() {
    // Reset position, but keep buffer capacity
    position_ = 0;
}

void PacketWriter::resetWithType(std::uint8_t packetType) {
    position_ = 0;
    writeByte(packetType);
}

void PacketWriter::ensureCapacity(std::size_t requiredCapacity) {
    if (requiredCapacity > buffer_.size()) {
        // Grow by doubling capacity, but at least to the required size
        std::size_t newCapacity = std::max(buffer_.size() * 2, requiredCapacity);
        buffer_.resize(newCapacity);
    }
}

} // namespace network
