#include "pch.h"
#include "packet_structures.h"
#include "gamestate_manager.h"
#include "packet_reader.h"

constexpr bool isFirstByte33(const packet& pkt) {
    return pkt.data[0] == 0x33;
}

template<bool Condition>
struct PacketProcessor {
    static void process(const packet& pkt) {
    }
};

template<>
struct PacketProcessor<true> {
    static void process(const packet& pkt) {
        try {
            Player p;
            PacketReader msg(pkt);

            msg.readByte();

            p.Position.X = msg.read<unsigned short>();
            p.Position.Y = msg.read<unsigned short>();
            p.Position.FacingDirection = static_cast<Direction>(msg.read<unsigned char>());
            p.Serial = msg.read<unsigned int>();
            p.Head = msg.read<unsigned short>();

            if (p.Head == 0xFFFF) {
                p.Form = msg.read<unsigned short>();
                p.Body = 0;
                p.Arms = msg.read<unsigned char>();
                p.Boots = msg.read<unsigned char>();
                p.Armor = msg.read<unsigned short>();
                p.Shield = msg.read<unsigned char>();
                p.Weapon = msg.read<unsigned short>();
                msg.readBytes(1); 
            }
            else {
                p.Body = msg.read<unsigned char>();
                p.Arms = msg.read<unsigned short>();
                p.Boots = msg.read<unsigned char>();
                p.Armor = msg.read<unsigned short>();
                p.Shield = msg.read<unsigned char>();
                p.Weapon = msg.read<unsigned short>();
                p.HeadColor = msg.read<unsigned char>();
                p.BootColor = msg.read<unsigned char>();
                p.Acc1Color = msg.read<unsigned char>();
                p.Acc1 = msg.read<unsigned short>();
                p.Acc2Color = msg.read<unsigned char>();
                p.Acc2 = msg.read<unsigned short>();
                p.Unknown = msg.read<unsigned char>();
                p.Acc3 = msg.read<unsigned short>();
                p.Unknown2 = msg.read<unsigned char>();
                p.RestCloak = msg.read<unsigned char>();
                p.Overcoat = msg.read<unsigned short>();
                p.OvercoatColor = msg.read<unsigned char>();
                p.SkinColor = msg.read<unsigned char>();
                p.HideBool = msg.read<unsigned char>();
                p.FaceShape = msg.read<unsigned char>();
            }

            msg.readByte();
            p.Name = msg.readString8();
            p.Hostile = p.IsHostile(game_state.hostile_players);

            if (p.Name == game_state.get_username()) {
                game_state.update_player_serial(p.Serial);
                game_state.update_player_direction(p.Position.FacingDirection);
                game_state.update_player_location(p.Position);
            }
            else 
            {
                game_state.player_manager.AddOrUpdate(p.Serial, p);
            }
        }
        catch (const std::exception& e) {
            std::cout << "Exception caught in ProcessPlayerPacket: " << e.what() << '\n';
        }
        catch (...) {
            std::cout << "Unknown exception caught in ProcessPlayerPacket." << '\n';
        }
    }
};

std::optional<packet> getPacketIfFirstByte33(const packet& pkt) {
    if (isFirstByte33(pkt)) {
        return pkt;
    }
    return std::nullopt;
}

extern void recv_handle_packet_x33(const packet& packet) {
    auto optionalPacket = getPacketIfFirstByte33(packet);
    optionalPacket.has_value()
		? PacketProcessor<true>::process(optionalPacket.value())
		: PacketProcessor<false>::process(packet);
}
