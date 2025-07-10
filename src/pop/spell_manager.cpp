#include "pch.h"
#include "spell.h"
#include "gamestate_manager.h"
#include "network_functions.h"
#include "spell_manager.h"
#include "packet_writer.h"


void spell_manager::cast(const std::string spellstr)
{
    spell* sp = this->find_spell_by_name(spellstr);
    if (sp)
    {
        this->apply_staff_effects_to_spell(*sp);

        PacketWriter packet1;
        packet1.write<byte>(0x4D);
        packet1.write<byte>(sp->castLines);
        packet1.write<byte>(0x00);
        packet1.sendToServer();

        if (sp->castLines > 0)
        {
            PacketWriter packet2;
            packet2.write<byte>(0x4E);
            packet2.writeString8(sp->name);
            packet2.write<byte>(0x00);
            packet2.sendToServer();

            std::this_thread::sleep_for(std::chrono::milliseconds(sp->castLines * 900));
            PacketWriter packet3;
            packet3.write<byte>(0x4E);
            packet3.writeString8(sp->name);
            packet3.write<byte>(0x00);
            packet3.sendToServer();
        }

        PacketWriter packet4;
        packet4.write<byte>(0x0F);
        packet4.write<byte>(sp->slot);
        packet4.write<unsigned int>(game_state.get_serial());
        packet4.write<unsigned short>(game_state.get_player_location().X);
        packet4.write<unsigned short>(game_state.get_player_location().Y);
        packet4.sendToServer();
    }
}


void spell_manager::cast_spell(const std::string &spell_name)
{
    const auto lower_spell_name = to_lower(spell_name);
    const auto &bestStaffMap = determine_best_staff_for_spells();

    std::string bestStaff;
    std::optional<Item> staff;

    auto cache_it = best_staff_cache_.find(lower_spell_name);
    if (cache_it != best_staff_cache_.end())
    {
        bestStaff = cache_it->second.first;
        staff = game_state.inventory_manager.FindItemByName(bestStaff);
    }
    else
    {
        auto it = bestStaffMap.find(lower_spell_name);
        if (it != bestStaffMap.end())
        {
            bestStaff = it->second;
            staff = game_state.inventory_manager.FindItemByName(bestStaff);
            if (staff.has_value())
            {
                best_staff_cache_[lower_spell_name] = std::make_pair(bestStaff, static_cast<byte>(staff.value().InventorySlot));
            }
        }
    }

    if (!bestStaff.empty() && *current_weapon_ != bestStaff)
    {
        std::cout << "Casting Spell '" << spell_name << "', Swapping to '" << bestStaff << "' for optimal performance." << std::endl;
        *current_weapon_ = bestStaff;

        if (staff.has_value() && !game_state.spellbar.HasPramh() && !game_state.spellbar.HasWolfFangFist() && game_state.CurrentWeaponName() != *current_weapon_)
        {
            *current_weapon_ = bestStaff;
            ITEM(static_cast<byte>(staff.value().InventorySlot));
        }
    }
    else if (!bestStaff.empty())
    {
        std::cout << "Casting Spell '" << spell_name << "', using the optimal Staff: '" << bestStaff << "'." << std::endl;
    }
    else
    {
        std::cout << "No specific best staff found for spell '" << spell_name << "'. Use default casting method." << std::endl;
    }

    this->cast(spell_name);
}
