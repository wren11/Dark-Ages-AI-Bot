#pragma once
#include "pch.h"
#include "player.h"
#include "structures.h"
#include "object_manager.h"
#include "statistics.h"
#include "constants.h"
#include "datafile.h"
#include "spellicons.h"
#include "sprite.h"
#include "inventory_manager.h"
#include "game_observers.h"
#include "spell_manager.h"
#include "spell.h"

class game_state_manager
{

public:
	game_state_manager() = default;

	bool initialize();

	spell_manager SpellContext() const
	{
		return spells_manager;
	}

	static std::string get_username()
	{
		std::string name;

		if (const auto raw_name = reinterpret_cast<char *>(userNameoffset); raw_name != nullptr)
		{
			name = raw_name;
		}

		if (name.length() > 20)
		{
			name = name.substr(0, 20);
		}

		return name;
	}

	unsigned int get_serial() const
	{
		return serial_;
	}

	Location get_player_location() const
	{
		return player_location_;
	}

	void set_player_info(std::string user, const Location loc, const Direction direction)
	{
		username_ = std::move(user);
		player_location_ = loc;
		player_location_.FacingDirection = direction;
	}

	void update_player_location(const Location &loc)
	{
		player_location_ = loc;

		stepsTaken_++;

		if (stepsTaken_ >= 4)
		{
			stepsTaken_ = 0;
		}
	}

	void update_player_direction(const Direction direction)
	{
		player_location_.FacingDirection = direction;
	}

	void update_player_serial(const unsigned int id)
	{
		serial_ = id;
	}

	void refresh_game_state()
	{
		block = false;
		stepsTaken_ = 0;

		const std::vector<std::shared_ptr<Player>> newPlayers = player_manager.GetObjectsWithinRange(player_location_);
		player_manager.RemoveObjectsOutsideRange(player_location_);
		player_manager.MergeOrPrune(newPlayers);
	}

	void update_game_states();
	void update(double deltaTime);
	static void refresh();

	std::vector<std::string> hostile_players;
	GenericObjectManager<Player, unsigned int> player_manager;
	GenericObjectManager<Sprite, unsigned int> sprite_manager;
	Spell_Icons spellbar;
	StatisticsManager statistics_observer;
	datafile storage_manager;
	AnimationsManager animations_manager;
	inventory_manager inventory_manager;
	spell_manager spells_manager;

	bool block = false;

	std::string CurrentWeaponName()
	{
		const DWORD baseAddress = 0x085118C;
		const DWORD offset1 = 0x588;
		const DWORD offset2 = 0x670;
		DWORD ptr;
		std::string weaponNameResult;

		try
		{
			ptr = *reinterpret_cast<DWORD *>(baseAddress);
			ptr += offset1;
			ptr = *reinterpret_cast<DWORD *>(ptr);
			ptr += offset2;

			LPCSTR weaponName = reinterpret_cast<LPCSTR>(ptr);

			if (weaponName != nullptr && IsBadStringPtrA(weaponName, 256) == 0)
			{
				weaponNameResult = std::string(weaponName);
			}
			else
			{
				return "";
			}
		}
		catch (...)
		{
			return "";
		}

		return weaponNameResult;
	}

private:
	std::string username_;
	unsigned int serial_{};
	Location player_location_;
	int stepsTaken_ = 0;
};

extern game_state_manager game_state;
