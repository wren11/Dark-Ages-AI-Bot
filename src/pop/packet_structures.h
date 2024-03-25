#pragma once
#include "pch.h"
#include <cstddef>
#include <stdexcept>
#include <iomanip>
#include <sstream>

// Assuming BYTE is defined as:
using BYTE = unsigned char;

class packet
{
public:
	BYTE *data;
	size_t length;

	packet(BYTE *d, size_t len) : data(new BYTE[len]), length(len)
	{
		std::copy(d, d + len, data);
	}

	~packet()
	{
		delete[] data;
	}

	packet(const packet &other) : data(new BYTE[other.length]), length(other.length)
	{
		std::copy(other.data, other.data + other.length, data);
	}

	packet &operator=(const packet &other)
	{
		if (this != &other)
		{
			delete[] data;
			length = other.length;
			data = new BYTE[length];
			std::copy(other.data, other.data + other.length, data);
		}
		return *this;
	}

	BYTE operator[](size_t index) const
	{
		if (index >= length)
		{
			throw std::out_of_range("Index out of range");
		}
		return data[index];
	}

	size_t size() const { return length; }

	void print_hex() const
	{
		std::ostringstream oss;
		for (size_t i = 0; i < length; ++i)
		{
			oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
			if (i < length - 1)
			{
				oss << " ";
			}
		}
		std::cout << oss.str() << '\n';
	}
};

void packet_send(const packet &p);

// Variadic template function to create a packet
template <typename... Bytes>
packet create_packet(Bytes... bytes)
{
	static_assert((... && std::is_integral_v<Bytes>), "All arguments must be integral types.");
	std::vector<BYTE> data = {static_cast<BYTE>(bytes)...};
	return packet(data.data(), data.size());
}
