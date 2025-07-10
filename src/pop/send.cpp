#include "pch.h"

#include "gamestate_manager.h"
#include "network_communicator.h"
#include "network_functions.h"

HANDLE g_h_shared_memory = nullptr;
void **g_p_shared_memory = nullptr;

int network_communicator::send_packet(BYTE *packet, int length)
{
	int cave = reinterpret_cast<int>(VirtualAllocEx(GetCurrentProcess(), nullptr, length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
	memcpy(reinterpret_cast<void *>(cave), (void *)packet, length); // NOLINT(performance-no-int-to-ptr)

	auto send_function = reinterpret_cast<send_function_type>(0x00563E00);
	int len = length;

	__asm
	{
		pushfd
		pushad
	}

	__asm
	{
		mov edx,len
		push edx

		mov eax,cave
		push eax

		mov ecx, [this_pointer]
		call send_function
	}

	__asm
	{
		popad
		popfd
	}

	VirtualFreeEx(GetCurrentProcess(), reinterpret_cast<void *>(cave), 0, MEM_RELEASE); // NOLINT(performance-no-int-to-ptr)

	return 0;
}

extern void packet_send(const packet &p)
{
	auto start = std::chrono::high_resolution_clock::now();

	game_function::send_to_server(p.data, p.length);

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	std::cout << "Time taken by packet_send: " << duration.count() << " microseconds" << std::endl;
}

BOOL initialize_shared_memory()
{
	g_h_shared_memory = CreateFileMapping(
		INVALID_HANDLE_VALUE, // Use paging file
		nullptr,			  // Default security
		PAGE_READWRITE,		  // Read/write access
		0,					  // Maximum object size (high-order DWORD)
		sizeof(void *),		  // Maximum object size (low-order DWORD)
		TEXT("Local\\SendFunctionAddress"));

	if (g_h_shared_memory == nullptr)
	{
		return FALSE;
	}

	g_p_shared_memory = static_cast<void **>(MapViewOfFile(
		g_h_shared_memory,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(void *)));

	if (g_p_shared_memory == nullptr)
	{
		CloseHandle(g_h_shared_memory);
		return FALSE;
	}
	*g_p_shared_memory = reinterpret_cast<void *>(&packet_send);

	return TRUE;
}

extern "C" __declspec(dllexport) BOOL get_send_function_address()
{
	if (g_p_shared_memory == nullptr)
	{
		if (!initialize_shared_memory())
		{
			return FALSE;
		}
	}
	return TRUE;
}

void cleanup_shared_memory()
{
	if (g_p_shared_memory != nullptr)
	{
		UnmapViewOfFile(g_p_shared_memory);
		g_p_shared_memory = nullptr;
	}
	if (g_h_shared_memory != nullptr)
	{
		CloseHandle(g_h_shared_memory);
		g_h_shared_memory = nullptr;
	}
}
