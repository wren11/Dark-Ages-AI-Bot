#include "pch.h"
#include "gamestate_manager.h"
#include "packet_reader.h"
#include "packet_structures.h"
#include "structures.h"
#include <regex>
#include "spell.h"
#include "spell_manager.h"

extern void recv_handle_packet_x07(const packet &packet)
{
    try
    {
        PacketReader msg(packet);
        msg.readByte();

        auto count = msg.read<uint16_t>();
        for (uint16_t i = 0; i < count; ++i)
        {
            auto xCoord = msg.read<uint16_t>();
            auto yCoord = msg.read<uint16_t>();
            auto serial = msg.read<uint32_t>();
            auto image = msg.read<uint16_t>();
            auto color = msg.read<uint8_t>();
            auto display = msg.read<uint16_t>();

            std::shared_ptr<Sprite> s = std::make_shared<Sprite>(xCoord, yCoord, serial, image);

            if (image >= 0x4000 && image <= 0x8000)
            {
                auto spriteOffset = image - 0x4000;
                auto unknown1 = msg.read<uint8_t>();
                auto num4 = msg.read<uint8_t>();
                auto unknown2 = msg.read<uint8_t>();
                auto type = msg.read<uint8_t>();

                if (type == 0x2)
                {
                    // NPC
                    auto name = msg.readString8();
                }
                else
                {
                    // MONSTER
                }
            }
            else if (image >= 0x8000)
            {
                // Item
                auto sprite = image - 0x8000;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught in recv_handle_packet_x07: " << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught in recv_handle_packet_x07." << '\n';
    }
}

extern void recv_handle_packet_x3A(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    USHORT icon = msg.read<unsigned short>();
    byte color = msg.readByte();

    if (color > 0)
    {
        game_state.spellbar.AddSpellIcon(icon, color);
    }
    else
    {
        game_state.spellbar.RemoveSpellIcon(icon);
    }
}

extern void recv_handle_packet_x39(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    Legend legendData;
    legendData.Nation = msg.readByte();
    legendData.GuildRank = msg.readString(msg.readByte());
    legendData.Title = msg.readString(msg.readByte());
    legendData.GroupList = msg.readString(msg.readByte());
    msg.readByte(); // Skip the byte that's always 1
    legendData.Notes = msg.readString(msg.readByte());
    legendData.Path = msg.readByte();
    legendData.Medenian = msg.readByte() == 1;
    legendData.Master = msg.readByte() == 1;
    legendData.Class = msg.readString(msg.readByte());
    legendData.Guild = msg.readString(msg.readByte());

    auto marksCount = msg.readByte();
    for (int i = 0; i < marksCount; ++i)
    {
        LegendMark mark;
        mark.Icon = msg.readByte();
        mark.TextColor = msg.readByte();
        mark.MarkID = msg.readString(msg.readByte());
        mark.Mark = msg.readString(msg.readByte());
        legendData.LegendMarks.push_back(mark);
    }
}

extern void recv_handle_packet_x04(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();
    game_state.update_player_location(Location(
        msg.read<unsigned short>(),
        msg.read<unsigned short>()));
}

extern void recv_handle_packet_x0B(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    const auto direction = static_cast<Direction>(msg.readByte());

    auto oldX = msg.read<unsigned short>();
    auto oldY = msg.read<unsigned short>();
    auto newX = oldX, newY = oldY;

    switch (direction)
    {
    case Direction::North:
        newY--;
        break;
    case Direction::South:
        newY++;
        break;
    case Direction::West:
        newX--;
        break;
    case Direction::East:
        newX++;
        break;
    default:;
    }

    game_state.update_player_direction(direction);
    game_state.update_player_location(Location(newX, newY));
}

extern void recv_handle_packet_x0C(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    unsigned int id = msg.read<unsigned int>();
    auto oldX = msg.read<unsigned short>();
    auto oldY = msg.read<unsigned short>();
    auto newX = oldX, newY = oldY;

    auto const direction = static_cast<Direction>(msg.readByte());

    switch (direction)
    {
    case Direction::North:
        newY--;
        break;
    case Direction::South:
        newY++;
        break;
    case Direction::West:
        newX--;
        break;
    case Direction::East:
        newX++;
        break;
    default:;
    }

    if (id == game_state.get_serial())
    {
        game_state.update_player_direction(direction);
        game_state.update_player_location(Location(newX, newY));
    }
    else
    {
        game_state.player_manager.GetAndApplyAction(
            id,
            [newX, newY, direction](Player *player)
            {
                if (player != nullptr)
                {
                    player->SetLocation(Location(newX, newY));
                    player->SetDirection(direction);
                }
            });
    }
}

extern void recv_handle_packet_x08(const packet &pkt)
{
    PacketReader packet(pkt);

    if (packet.readByte() == 0x08)
    {
        char bitmask = packet.read<char>();

        StatsSnapshot newStats{};
        newStats.BitMask = bitmask;

        if (bitmask & 0x20)
        {
            packet.setPosition(packet.getPosition() + 3);
            newStats.Level = packet.read<char>();
            newStats.Ability = packet.read<char>();
            newStats.MaximumHP = packet.read<unsigned int>();
            newStats.MaximumMP = packet.read<unsigned int>();
            newStats.Str = packet.read<char>();
            newStats.Int = packet.read<char>();
            newStats.Wis = packet.read<char>();
            newStats.Con = packet.read<char>();
            newStats.Dex = packet.read<char>();
            newStats.availablePoints = packet.read<bool>();
            newStats.AvailablePoints = packet.read<char>();
            newStats.MaximumWeight = packet.read<unsigned short>();
            newStats.CurrentWeight = packet.read<unsigned short>();
            packet.setPosition(packet.getPosition() + 4);
        }
        if (bitmask & 0x10)
        {
            newStats.CurrentHP = packet.read<unsigned int>();
            newStats.CurrentMP = packet.read<unsigned int>();
        }
        if (bitmask & 0x08)
        {
            newStats.Experience = packet.read<unsigned int>();
            newStats.ToNextLevel = packet.read<unsigned int>();
            newStats.AbilityExp = packet.read<unsigned int>();
            newStats.ToNextAbility = packet.read<unsigned int>();
            packet.setPosition(packet.getPosition() + 4);
            newStats.Gold = packet.read<unsigned int>();
        }
        if (bitmask & 0x04)
        {
            newStats.BitMask = packet.read<unsigned short>();
            packet.setPosition(packet.getPosition() + 1);
            newStats.AttackElement2 = packet.read<char>();
            newStats.DefenseElement2 = packet.read<char>();
            newStats.MailAndParcel = packet.read<char>();
            newStats.AttackElement = static_cast<Elements>(packet.read<char>());
            newStats.DefenseElement = static_cast<Elements>(packet.read<char>());
            newStats.MagicResistance = packet.read<char>();
            packet.setPosition(packet.getPosition() + 1);
            newStats.ArmorClass = static_cast<signed char>(packet.read<char>());
            newStats.Damage = packet.read<char>();
            newStats.Hit = packet.read<char>();
        }

        game_state.statistics_observer.updateStats(newStats);
    }
}

extern void recv_handle_packet_x29(const packet &pkt)
{
    PacketReader msg(pkt);
    msg.readByte();

    int fromId = 0;
    int targetId = 0;
    int fromEffect = 0;
    int targetEffect = 0;
    int delay = 0;
    int x = 0;
    int y = 0;

    targetId = msg.read<unsigned int>();

    if (targetId != 0)
    {
        fromId = msg.read<unsigned int>();
        targetEffect = msg.read<unsigned short>();
        fromEffect = msg.read<unsigned short>();
        delay = msg.read<unsigned short>();
    }
    else
    {
        targetEffect = msg.read<unsigned short>();
        delay = msg.read<unsigned short>();
        x = msg.read<unsigned short>();
        y = msg.read<unsigned short>();
        msg.readByte();
    }

    Animation
        animation(
            fromId,
            targetId,
            fromEffect,
            targetEffect,
            delay,
            x,
            y);

    game_state.animations_manager.addAnimation(animation);
}

extern void recv_handle_packet_x0E(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    const unsigned int id = msg.read<unsigned int>();

    if (!game_state.player_manager.DeleteBySerial(id))
        game_state.sprite_manager.DeleteBySerial(id);
}

extern void recv_handle_packet_x17(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    spell s;
    s.slot = msg.readByte();
    s.icon = msg.read<unsigned short>();
    s.type = msg.readByte();
    s.name = msg.readString(msg.readByte());
    s.prompt = msg.readString(msg.readByte());
    s.castLines = msg.readByte();

    std::regex spellNameRegex("(.*?)( \\(Lev:)(\\d+)(\\/)(\\d+)(\\))");
    std::smatch match;
    if (std::regex_search(s.name, match, spellNameRegex) && match.size() > 5)
    {
        s.name = match[1].str();
    }

    game_state.spells_manager.add_spell(s);
}

extern void recv_handle_packet_x18(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    int slot = msg.readByte();
    if (slot > 0)
    {
        game_state.spells_manager.remove_spell(slot);
    }
}

extern void recv_handle_packet_x10(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    int slot = msg.readByte();
    game_state.inventory_manager.RemoveItem(slot);
}

extern void recv_handle_packet_x0F(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    Item item;
    item.InventorySlot = msg.readByte();
    item.Icon = msg.read<unsigned short>();
    item.IconPal = msg.readByte();
    item.Name = msg.readString8();
    item.Amount = msg.read<unsigned int>();
    item.Stackable = msg.readByte();
    item.MaximumDurability = msg.read<unsigned int>();
    item.CurrentDurability = msg.read<unsigned int>();

    game_state.inventory_manager.AddItem(item);
}
