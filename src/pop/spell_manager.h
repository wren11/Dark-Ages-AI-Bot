#pragma once
#include "gamestate_manager.h"
#include "item.h"
#include "game_observers.h"
#include "SpellData.h"
#include "SpellEffect.h"
#include "gamestate_manager.h"
#include "spell.h"

class spell_manager : public IWeaponChangeObserver
{
private:
    std::shared_ptr<std::string> current_weapon_;
    std::vector<spell> spells_;
    std::map<std::string, std::shared_ptr<SpellEffect>> staff_effects_ = {
        {"Skylight Staff", std::make_shared<ReduceEffect>(2)},
        {"Dragon Infused Staff", std::make_shared<SetEffect>(1)}};

    std::map<std::string, std::pair<std::string, int>> best_staff_cache_;

public:
    spell_manager() : current_weapon_(std::make_shared<std::string>(""))
    {
        spells_.resize(90);
    }

    void OnWeaponChange(const std::string &newWeaponName) override
    {
        Update(newWeaponName);
    }

    std::map<std::string, std::string> determine_best_staff_for_spells() const
    {
        std::map<std::string, std::string> bestStaffForSpell;

        for (const auto &[spellName, baseLine] : SpellData::baseSpellLines)
        {
            std::optional<std::pair<std::string, int>> bestStaff;
            int lowestLine = 100;

            for (const auto &[staffName, effect] : staff_effects_)
            {
                int adjustedLine = baseLine;
                if (effect->GetType() == StaffEffectType::Reduce)
                {
                    adjustedLine -= effect->GetEffectValue();
                }
                else if (effect->GetType() == StaffEffectType::Set)
                {
                    adjustedLine = effect->GetEffectValue();
                }

                if (adjustedLine < lowestLine)
                {
                    lowestLine = adjustedLine;
                    bestStaff = std::make_pair(staffName, adjustedLine);
                }
            }

            if (bestStaff && lowestLine < baseLine)
            {
                bestStaffForSpell[spellName] = bestStaff->first;
            }
        }

        return bestStaffForSpell;
    }

    int lookup_spell_base(const std::string &spell_name) const
    {
        std::string lower_spell_name = to_lower(spell_name);
        for (const auto &pair : SpellData::baseSpellLines)
        { // Updated to use SpellData class
            if (to_lower(pair.first) == lower_spell_name)
            {
                return pair.second;
            }
        }
        return -1;
    }

    spell *find_spell_by_slot(BYTE slot)
    {
        auto it = std::find_if(spells_.begin(), spells_.end(), [slot](const spell &sp)
                               { return sp.slot == slot; });

        if (it != spells_.end())
        {
            return &(*it);
        }
        else
        {
            return nullptr;
        }
    }

    spell* find_spell_by_name(const std::string& spellName)
    {
        auto it = std::find_if(spells_.begin(), spells_.end(), [&spellName](const spell& sp)
            { return sp.name == spellName; });

        if (it != spells_.end())
        {
            return &(*it);
        }
        else
        {
            return nullptr;
        }
    }

    void apply_staff_effects_to_spell(spell &sp)
    {
        if (current_weapon_ && !current_weapon_->empty())
        {
            auto effectIt = staff_effects_.find(*current_weapon_);
            if (effectIt != staff_effects_.end())
            {
                auto &effect = effectIt->second;
                if (effect->GetType() == StaffEffectType::Reduce)
                {
                    sp.castLines = abs(sp.castLines - effect->GetEffectValue());
                }
                else if (effect->GetType() == StaffEffectType::Set)
                {
                    sp.castLines = effect->GetEffectValue();
                }
            }
        }
    }

    void cast(const std::string spellstr);

    void Update(const std::string &equipped_weapon)
    {
        try
        {
            *current_weapon_ = equipped_weapon;
            auto bestStaffMap = determine_best_staff_for_spells();

            for (const auto &entry : bestStaffMap)
            {
                std::cout << "Best staff for " << entry.first << " is " << entry.second << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception caught in spell_manager::Update: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Unknown exception caught in spell_manager::Update." << std::endl;
        }
    }

    void clear_best_staff_cache()
    {
        best_staff_cache_.clear();
    }

    // Helper method to convert spell name to lowercase
    static std::string to_lower(const std::string &str)
    {
        std::string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });
        return lower_str;
    }

    void add_spell(spell &spell)
    {
        std::string spellName(spell.name);
        size_t pos = spellName.find(" (");

        if (pos != std::string::npos)
        {
            spellName = spellName.substr(0, pos);
        }

        spell.name = std::string(spellName);

        spells_[spell.slot] = spell;
    }

    void add_spells(std::vector<spell> spells)
    {
        for (auto &sp : spells)
        {
            add_spell(sp);
        }
    }

    void remove_spell(const byte slot)
    {
        auto it = std::find_if(spells_.begin(), spells_.end(), [&slot](const spell &sp)
                               { return sp.slot == slot - 1; });

        if (it != spells_.end())
        {
            spells_.erase(it);
        }
        else
        {
            std::cout << "Spell to remove not found." << std::endl;
        }
    }

    void cast_spell(const std::string &spell_name);
};