/**
 * @file packet.cpp
 * @brief Implementation of the Packet class for network packet handling
 */
#include "packet.h"

namespace network {

Packet::Packet(ByteType* data, size_t length) 
    : data_(std::make_unique<ByteType[]>(length)), 
      length_(length) 
{
    std::copy(data, data + length, data_.get());
}

Packet::Packet(const Packet& other) 
    : data_(std::make_unique<ByteType[]>(other.length_)), 
      length_(other.length_) 
{
    std::copy(other.data_.get(), other.data_.get() + other.length_, data_.get());
}

Packet& Packet::operator=(const Packet& other) 
{
    if (this != &other) 
    {
        data_ = std::make_unique<ByteType[]>(other.length_);
        length_ = other.length_;
        std::copy(other.data_.get(), other.data_.get() + other.length_, data_.get());
    }
    return *this;
}

ByteType Packet::operator[](size_t index) const 
{
    if (index >= length_) 
    {
        throw std::out_of_range("Index out of range");
    }
    return data_[index];
}

size_t Packet::size() const noexcept 
{
    return length_;
}

void Packet::printHex() const 
{
    std::ostringstream oss;
    for (size_t i = 0; i < length_; ++i) 
    {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << static_cast<int>(data_[i]);
        if (i < length_ - 1) 
        {
            oss << " ";
        }
    }
    std::cout << oss.str() << '\n';
}

const ByteType* Packet::data() const noexcept 
{
    return data_.get();
}

void sendPacket(const Packet& packet) 
{
    // TODO: Implement actual network sending functionality
    // This function will use the game's networking API to send the packet
}

} // namespace network
