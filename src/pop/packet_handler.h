#pragma once
#include "pch.h"
#include "packet_structures.h"

using PacketHandlerFunc = void (*)(const packet &);

void send_handle_packet_x1C(const packet &packet);
void send_handle_packet_x38(const packet &packet);
void send_handle_packet_x10(const packet &packet);
void send_handle_packet_x0F(const packet &packet);
void send_handle_packet_x13(const packet &packet);
void send_handle_packet_x06(const packet &packet);

void recv_handle_packet_x04(const packet &packet);
void recv_handle_packet_x0B(const packet &packet);
void recv_handle_packet_x0C(const packet &packet);
void recv_handle_packet_x17(const packet &packet);
void recv_handle_packet_x0E(const packet &packet);
void recv_handle_packet_x3A(const packet &packet);
void recv_handle_packet_x33(const packet &packet);
void recv_handle_packet_x07(const packet &packet);
void recv_handle_packet_x29(const packet &packet);
void recv_handle_packet_x15(const packet &packet);
void recv_handle_packet_x39(const packet &packet);
void recv_handle_packet_x10(const packet &packet);
void recv_handle_packet_x18(const packet &packet);
void recv_handle_packet_x0F(const packet &packet);
