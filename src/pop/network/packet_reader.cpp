/**
 * @file packet_reader.cpp
 * @brief Implementation of the PacketReader class
 */
#include "packet_reader.h"
#include "../utils/logging.h"

namespace network {

PacketReader::PacketReader(const Packet& packet)
    : packet_(packet), position_(0) {
}

PacketReader::PacketReader(const std::vector<unsigned char>& data)
    : position_(0) {
    packet_ = Packet(const_cast<uint8_t*>(data.data()), data.size());
}

PacketReader::PacketReader(const uint8_t* data, size_t length)
    : position_(0) {
    packet_ = Packet(const_cast<uint8_t*>(data), length);
}

std::uint8_t PacketReader::readByte() {
    if (position_ >= packet_.size()) {
        utils::Logging::error("Attempt to read byte beyond packet boundary");
        throw PacketReadException("Attempt to read byte beyond packet boundary");
    }
    
    return packet_[position_++];
}

std::string PacketReader::readString8() {
    try {
        // Read length byte
        std::uint8_t length = readByte();
        
        // Read string with that length
        return readString(length);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error reading string8: " + std::string(e.what()));
        throw;
    }
}

std::string PacketReader::readString16() {
    try {
        // Read length word
        std::uint16_t length = read<std::uint16_t>();
        
        // Read string with that length
        return readString(length);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Error reading string16: " + std::string(e.what()));
        throw;
    }
}

std::string PacketReader::readString(std::size_t length) {
    // Check if we have enough data
    if (position_ + length > packet_.size()) {
        utils::Logging::error("Attempt to read string beyond packet boundary");
        throw PacketReadException("Attempt to read string beyond packet boundary");
    }
    
    // Extract the string
    std::string result;
    result.reserve(length);
    
    for (std::size_t i = 0; i < length; ++i) {
        char ch = static_cast<char>(packet_[position_ + i]);
        // Stop at null terminator if found
        if (ch == '\0') {
            position_ += i + 1;
            return result;
        }
        result.push_back(ch);
    }
    
    // Advance position
    position_ += length;
    
    return result;
}

std::vector<std::uint8_t> PacketReader::readBytes(std::size_t length) {
    // Check if we have enough data
    if (position_ + length > packet_.size()) {
        utils::Logging::error("Attempt to read bytes beyond packet boundary");
        throw PacketReadException("Attempt to read bytes beyond packet boundary");
    }
    
    // Extract the bytes
    std::vector<std::uint8_t> result;
    result.reserve(length);
    
    for (std::size_t i = 0; i < length; ++i) {
        result.push_back(packet_[position_ + i]);
    }
    
    // Advance position
    position_ += length;
    
    return result;
}

void PacketReader::skip(std::size_t bytes) {
    if (position_ + bytes > packet_.size()) {
        utils::Logging::error("Attempt to skip beyond packet boundary");
        throw PacketReadException("Attempt to skip beyond packet boundary");
    }
    
    position_ += bytes;
}

void PacketReader::setPosition(std::size_t position) {
    if (position > packet_.size()) {
        utils::Logging::error("Attempt to set position beyond packet boundary");
        throw PacketReadException("Attempt to set position beyond packet boundary");
    }
    
    position_ = position;
}

std::size_t PacketReader::getPosition() const {
    return position_;
}

std::size_t PacketReader::getRemainingBytes() const {
    return packet_.size() - position_;
}

const Packet& PacketReader::getPacket() const {
    return packet_;
}

std::uint8_t PacketReader::peekByte() const {
    if (position_ >= packet_.size()) {
        utils::Logging::error("Attempt to peek byte beyond packet boundary");
        throw PacketReadException("Attempt to peek byte beyond packet boundary");
    }
    
    return packet_[position_];
}

std::uint8_t PacketReader::peekByteAt(std::size_t offset) const {
    if (offset >= packet_.size()) {
        utils::Logging::error("Attempt to peek byte at offset beyond packet boundary");
        throw PacketReadException("Attempt to peek byte at offset beyond packet boundary");
    }
    
    return packet_[offset];
}

bool PacketReader::hasMoreData() const {
    return position_ < packet_.size();
}

} // namespace network
