#include "pch.h"
#include "network/packet_handler.h"
#include "packet_registry.h"
#include "packet_structures.h"

std::unordered_map<uint8_t, PacketHandlerFunc> PacketHandlerRegistry::recv_handlers_;
std::unordered_map<uint8_t, PacketHandlerFunc> PacketHandlerRegistry::send_handlers_;

void PacketHandlerRegistry::register_send_handlers(
    const uint8_t opcode,
    const PacketHandlerFunc handler)
{
    send_handlers_[opcode] = handler;
}

void PacketHandlerRegistry::register_recv_handlers(
    const uint8_t opcode,
    const PacketHandlerFunc handler)
{
    recv_handlers_[opcode] = handler;
}

void PacketHandlerRegistry::handle_outgoing_data(const packet &pkt)
{
    if (pkt.length < 2)
        return;

    if (const auto it = send_handlers_.find(pkt.data[0]); it != send_handlers_.end())
    {
        it->second(pkt);
    }
}

void PacketHandlerRegistry::handle_incoming_data(const packet &pkt)
{
    if (pkt.length < 2)
        return;

    if (const auto it = recv_handlers_.find(pkt.data[0]); it != recv_handlers_.end())
    {
        it->second(pkt);
    }
}
