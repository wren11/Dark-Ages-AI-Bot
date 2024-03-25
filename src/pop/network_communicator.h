#pragma once
#include "pch.h"
#include "packet_structures.h"

void cleanup_shared_memory();
BOOL initialize_shared_memory();

constexpr int this_pointer = 0x0073D958;
typedef int(__thiscall *send_function_type)(int this_ptr, const void *a2, __int16 a3);

class network_communicator
{
public:
	static int send_packet(BYTE *packet, int length);
};

void packet_send(const packet &p);
