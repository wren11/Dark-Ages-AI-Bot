// ReSharper disable CppClangTidyPerformanceNoIntToPtr
// ReSharper disable CppClangTidyClangDiagnosticLanguageExtensionToken
// ReSharper disable CppDeclarationSpecifierWithoutDeclarators

#pragma once
#include "pch.h"
#include "constants.h"
#include "sprite.h"

static class game_function
{
public:
	static int this_pointer()
	{

		const auto valuePtr = reinterpret_cast<int *>(senderOffset);

		if (valuePtr == nullptr)
		{
			return -1;
		}

		try
		{
			return *valuePtr;
		}
		catch (...)
		{
			return -1;
		}
	}

	using HookFunctionType = void(__cdecl *)(int *, BYTE);

	static void walk(const BYTE &direction)
	{
		const auto this_ptr = reinterpret_cast<int *>(0x00882E68);
		const auto hookFunction = reinterpret_cast<HookFunctionType>(0x005F0C40);

		if (!this_ptr || !hookFunction)
		{
			return;
		}

		try
		{
			hookFunction(this_ptr, direction);
		}
		catch (...)
		{
		}
	}

	static void follow_object(const uint32_t id)
	{
		const auto this_ptr_address = reinterpret_cast<int *>(0x00882E68);

		if (this_ptr_address == nullptr)
		{
			return;
		}

		const int this_ptr = *this_ptr_address;

		if (this_ptr <= 0)
			return;

		const auto hook = reinterpret_cast<void(__thiscall *)(int *, int)>(0x005F4A70);

		if (hook == nullptr)
			return;

		if (id <= 0)
			return;

		__try
		{
			hook(reinterpret_cast<int *>(this_ptr), static_cast<int>(id));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	static void set_movement_locked()
	{
		const auto address = reinterpret_cast<uint8_t *>(0x5F0ADE);
		address[1] = 0x74;
	}

	static void set_movement_unlocked()
	{
		const auto address = reinterpret_cast<uint8_t *>(0x5F0ADE);
		address[1] = 0x75;
	}

	static uint8_t movement_state()
	{
		const uint8_t *address = reinterpret_cast<uint8_t *>(0x5F0ADE);
		const uint8_t value = *address;
		return value;
	}

	static void open_menu_raw(const uint32_t id)
	{
		int this_ptr = *reinterpret_cast<int *>(0x00882E68);
		int hook = 0x005F4730;
		void *memory = malloc(sizeof(char));
		memory = reinterpret_cast<void *>(id);

		__try
		{
			__asm
			{
				pushfd
				pushad

				mov eax, memory
				push eax
				mov ecx, [this_ptr]
				call hook

				popad
				popfd
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	using DA_OPEN_MENU = void(__cdecl *)(int *, int);

	static void open_menu(const uint32_t id)
	{
		const auto this_ptr = reinterpret_cast<int *>(0x00882E68);
		const DA_OPEN_MENU hookFunction = reinterpret_cast<DA_OPEN_MENU>(0x005F4730);

		if (!this_ptr || !hookFunction)
		{
			return;
		}

		try
		{
			hookFunction(this_ptr, static_cast<int>(id));
		}
		catch (...)
		{
		}
	}

	static int send_to_client(BYTE *packet, int length)
	{
		if (packet == nullptr)
			return 0;

		if (length <= 0)
			return 0;

		if (length > 0)
		{
			__try
			{
				__asm
				{
					pushfd
					pushad
				}

				int cave;
				cave = reinterpret_cast<int>(
					VirtualAllocEx(GetCurrentProcess(),
								   nullptr,
								   length,
								   MEM_RESERVE | MEM_COMMIT,
								   PAGE_READWRITE));

				memcpy(
					reinterpret_cast<void *>(cave),
					packet,
					length);

				int packet_in = recvPacketin;
				int packet_length = length;

				__asm
				{
					pop ecx

					mov edx, packet_length
					push edx

					mov eax, cave
					push eax

					call packet_in
					lea ecx, [ebp - 00000405]
					push ecx
				}

				__asm {
					popad
						popfd}

				VirtualFreeEx(GetCurrentProcess(), reinterpret_cast<void *>(cave), 0, MEM_RELEASE);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}

			return 0;
		}
	}

	static int send_to_server(BYTE *packet, int length)
	{
		if (packet == nullptr)
			return 0;

		if (length <= 0)
			return 0;

		__try
		{
			int cave = reinterpret_cast<int>(
				VirtualAllocEx(GetCurrentProcess(),
							   nullptr,
							   length,
							   MEM_RESERVE | MEM_COMMIT,
							   PAGE_READWRITE));

			memcpy(
				reinterpret_cast<void *>(cave),
				packet,
				length);

			__asm
			{
				pushfd
				pushad
			}

			int sender_id = this_pointer();

			if (sender_id <= 0)
				return -1;

			int send = sendOffset;
			int packet_length = length;

			__asm
			{
				mov edx, packet_length
				push edx

				mov eax, cave
				push eax

				mov ecx, [sender_id]
				call send
			}

			__asm {
				popad
					popfd}

			VirtualFreeEx(
				GetCurrentProcess(),
				reinterpret_cast<void *>(cave),
				0,
				MEM_RELEASE);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

		return 0;
	}

};

#define FACE(direction) game_function::send_to_server(new BYTE[]{0x11, direction, 0x00}, 3);
#define ASSAIL game_function::send_to_server(new BYTE[]{0x13, 0x01}, 3);
#define F5 game_function::send_to_server(new BYTE[]{0x38, 0x01}, 3);
#define ITEM(slot) game_function::send_to_server(new BYTE[]{0x1C, slot}, 3);
#define SKILL(slot) game_function::send_to_server(new BYTE[]{0x3E, slot, 0x00}, 3);
#define SPELL(slot) game_function::send_to_server(new BYTE[3]{0x0F, slot, 0x00}, 3);
#define ITEM_OFF(slot) game_function::send_to_server(new BYTE[3]{0x44, slot, 0x00}, 3);
#define CLICK(id) game_function::click_object(id);
#define CAN_MOVE game_function::movement_state() == 0x75;
#define CANNOT_MOVE game_function::movement_state() == 0x74;
