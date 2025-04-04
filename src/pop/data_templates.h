#pragma once
#include "pch.h"
#include <string>
#include <stdexcept>
#include "packet_structures.h"

using BYTE = unsigned char;
using USHORT = unsigned short;

template <typename T>
T ReadData(const packet &pkt, size_t &index)
{
	if (index + sizeof(T) > pkt.size())
	{
		throw std::out_of_range("Attempt to read beyond packet length");
	}

	T value = 0;
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		value |= static_cast<T>(pkt[index + i]) << (8 * (sizeof(T) - 1 - i));
	}
	index += sizeof(T);
	return value;
}

template <>
inline std::string ReadData<std::string>(const packet &pkt, size_t &index)
{
	BYTE length = ReadData<BYTE>(pkt, index);

	if (index + length > pkt.size())
	{
		throw std::out_of_range("Attempt to read beyond packet length for string");
	}

	std::string str(reinterpret_cast<const char *>(&pkt.data[index]), length);
	index += length;
	return str;
}
