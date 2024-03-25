#include "pch.h"
#include "intercept_manager.h"
#include <future>
#include "packet_reader.h"
#include "gamestate_manager.h"
#include "overlay_manager.h"
#include "packet_processor.h"
#include "packet_registry.h"

intercept_manager::PFN_ORIGINAL_SEND
	intercept_manager::TrueSendFunction = nullptr;
intercept_manager::PFN_ORIGINAL_RECV
	intercept_manager::TrueRecvFunction = nullptr;

const DWORD intercept_manager::sendPacketOutgoing = 0x00567FB0;
const DWORD intercept_manager::recvPacketIncoming = 0x00467060;

LONG WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
	return EXCEPTION_CONTINUE_EXECUTION;
}

PacketProcessor packetProcessor;

int __stdcall intercept_manager::SendFunctionStub(BYTE *data, int arg1, int arg2, char arg3)
{
	if (data == nullptr || arg1 < 2)
		return 0;
	packetProcessor.enqueueSend(std::make_shared<packet>(data, arg1));
	return TrueSendFunction(data, arg1, arg2, arg3);
}

int __stdcall intercept_manager::RecvFunctionStub(BYTE *data, int arg1)
{
	if (data == nullptr || arg1 < 2)
		return 0;
	packetProcessor.enqueueRecv(std::make_shared<packet>(data, arg1));
	return TrueRecvFunction(data, arg1);
}

void __stdcall intercept_manager::on_packet_send(packet *packet)
{
	if (packet != nullptr && packet->length >= 2)
	{
		packet->print_hex();
		PacketHandlerRegistry::handle_outgoing_data(*packet);
	}
}

void intercept_manager::on_packet_recv(packet *packet)
{
	if (packet != nullptr && packet->length >= 2)
	{
		PacketHandlerRegistry::handle_incoming_data(*packet);
	}
}

void intercept_manager::initialize_game_state()
{
	game_state.initialize();
	game_state.set_player_info(
		game_state_manager::get_username(),
		Location(0, 0),
		Direction::North);
}

void intercept_manager::initialize_drawing_manager()
{
	std::thread overlayThread([&]()
		{
			drawing_manager.initialize();
			drawing_manager.run();
		});
	overlayThread.detach();
}

void intercept_manager::initialize_handlers()
{
	PacketHandlerRegistry::register_send_handlers(0x1C, send_handle_packet_x1C);
	PacketHandlerRegistry::register_send_handlers(0x38, send_handle_packet_x38);
	PacketHandlerRegistry::register_send_handlers(0x10, send_handle_packet_x10);
	PacketHandlerRegistry::register_send_handlers(0x0F, send_handle_packet_x0F);
	PacketHandlerRegistry::register_send_handlers(0x13, send_handle_packet_x13);
	PacketHandlerRegistry::register_send_handlers(0x06, send_handle_packet_x06);

	PacketHandlerRegistry::register_recv_handlers(0x3A, recv_handle_packet_x3A);
	PacketHandlerRegistry::register_recv_handlers(0x04, recv_handle_packet_x04);
	PacketHandlerRegistry::register_recv_handlers(0x0B, recv_handle_packet_x0B);
	PacketHandlerRegistry::register_recv_handlers(0x0C, recv_handle_packet_x0C);
	PacketHandlerRegistry::register_recv_handlers(0x17, recv_handle_packet_x17);
	PacketHandlerRegistry::register_recv_handlers(0x0E, recv_handle_packet_x0E);
	PacketHandlerRegistry::register_recv_handlers(0x07, recv_handle_packet_x07);
	PacketHandlerRegistry::register_recv_handlers(0x33, recv_handle_packet_x33);
	PacketHandlerRegistry::register_recv_handlers(0x29, recv_handle_packet_x29);
	PacketHandlerRegistry::register_recv_handlers(0x39, recv_handle_packet_x39);
	PacketHandlerRegistry::register_recv_handlers(0x18, recv_handle_packet_x18);
	PacketHandlerRegistry::register_recv_handlers(0x10, recv_handle_packet_x10);
	PacketHandlerRegistry::register_recv_handlers(0x0F, recv_handle_packet_x0F);
}

void intercept_manager::initialize_assets()
{
}

void intercept_manager::Initialize()
{
	initialize_assets();
	initialize_handlers();
	initialize_drawing_manager();
	initialize_game_state();
}

void intercept_manager::AttachHook()
{
	TrueSendFunction = reinterpret_cast<PFN_ORIGINAL_SEND>(DetourFunction(reinterpret_cast<PBYTE>(sendPacketOutgoing),
																		  reinterpret_cast<PBYTE>(SendFunctionStub)));

	TrueRecvFunction = reinterpret_cast<PFN_ORIGINAL_RECV>(DetourFunction(reinterpret_cast<PBYTE>(recvPacketIncoming),
																		  reinterpret_cast<PBYTE>(RecvFunctionStub)));
}

void intercept_manager::RemoveHook()
{
	DetourRemove(reinterpret_cast<PBYTE>(TrueSendFunction), reinterpret_cast<PBYTE>(SendFunctionStub));
	DetourRemove(reinterpret_cast<PBYTE>(TrueRecvFunction), reinterpret_cast<PBYTE>(RecvFunctionStub));

	drawing_manager.cleanup();
}
