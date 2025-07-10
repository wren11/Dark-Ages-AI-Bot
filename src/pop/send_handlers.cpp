// ReSharper disable CppClangTidyClangDiagnosticSwitchEnum
#include "pch.h"
#include "gamestate_manager.h"
#include "packet_reader.h"
#include "packet_structures.h"
#include "structures.h"
#include "spell.h"

extern void send_handle_packet_x0E(const packet &packet)
{
}

extern void send_handle_packet_x0C(const packet &packet)
{
}

extern void send_handle_packet_x38(const packet &packet)
{
    game_state.refresh_game_state();
}

extern void send_handle_packet_x1C(const packet &packet)
{
    std::string initialWeaponName = game_state.CurrentWeaponName();
    std::string weaponName;

    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);
    bool weaponNameChanged = false;

    while (std::chrono::steady_clock::now() - startTime < timeout)
    {
        weaponName = game_state.CurrentWeaponName();
        if (weaponName != initialWeaponName && weaponName != "")
        {
            weaponNameChanged = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (weaponNameChanged)
    {
        game_state.spells_manager.OnWeaponChange(weaponName);
    }
}

extern void send_handle_packet_x13(const packet &packet)
{
    if (!game_state.spellbar.HasDion())
    {
        game_state.spells_manager.cast_spell("ard cradh");
    }
}

extern void send_handle_packet_x10(const packet &packet)
{
}

extern void send_handle_packet_x0F(const packet &packet)
{
}

extern void send_handle_packet_x06(const packet &packet)
{
    PacketReader msg(packet);
    msg.readByte();

    Location location = game_state.get_player_location();

    switch (static_cast<Direction>(msg.readByte()))
    {
    case Direction::North:
        location.Y--;
        break;
    case Direction::South:
        location.Y++;
        break;
    case Direction::West:
        location.X--;
        break;
    case Direction::East:
        location.X++;
        break;
    default:;
    }

    game_state.update_player_location(location);
}
