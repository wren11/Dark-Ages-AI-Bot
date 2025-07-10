#pragma once
#include "pch.h"
#include "packet_handler.h"

class PacketHandlerRegistry
{
public:
	static void register_send_handlers(uint8_t opcode, PacketHandlerFunc handler);
	static void register_recv_handlers(uint8_t opcode, PacketHandlerFunc handler);
	static void handle_outgoing_data(const packet &pkt);
	static void handle_incoming_data(const packet &pkt);

private:
	static std::unordered_map<uint8_t, PacketHandlerFunc> recv_handlers_;
	static std::unordered_map<uint8_t, PacketHandlerFunc> send_handlers_;
};

inline PacketHandlerRegistry handler_registry;
