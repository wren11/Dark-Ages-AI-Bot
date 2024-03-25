#pragma once
#include "pch.h"
#include "packet_structures.h"
#include "worker.h"

class intercept_manager
{
public:
	static void Initialize();
	static void AttachHook();
	static void RemoveHook();

	static int __stdcall SendFunctionStub(BYTE *data, int arg1, int arg2, char arg3);
	static int __stdcall RecvFunctionStub(BYTE *data, int arg1);

	static void __stdcall on_packet_send(packet *packet);
	static void __stdcall on_packet_recv(packet *packet);

	static void initialize_game_state();
	static void initialize_drawing_manager();
	static void initialize_handlers();
	static void initialize_assets();

private:
	using PFN_ORIGINAL_SEND = int(__stdcall *)(BYTE *data, int arg1, int arg2, char arg3);
	using PFN_ORIGINAL_RECV = int(__stdcall *)(BYTE *data, int arg1);

	static PFN_ORIGINAL_SEND TrueSendFunction;
	static PFN_ORIGINAL_RECV TrueRecvFunction;

	static const DWORD sendPacketOutgoing;
	static const DWORD recvPacketIncoming;
};
