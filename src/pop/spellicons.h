#pragma once
#include "pch.h"

enum class SpellIcon
{
    Disease = 1,
    Mezmerize_WormSkull_Shout = 2,
    Blind = 3,
    BeagCradh = 5,
    EisdCreutair = 7,
    DoubleExpApBonus = 8,
    Hide = 10,
    NaomhAite = 11,
    CreagNeart = 13,
    Beannaich = 16,
    PerfectDefense_WeatherProtection = 19,
    Slan_Silence = 26,
    Poison = 35,
    Burn_Pause = 40,
    Sleep = 50,
    FasDeireas = 52,
    Dion = 53,
    DeireasFaileas_AsgallFaileas = 54,
    Mist = 55,
    Cradh = 82,
    MorCradh = 83,
    ArdCradh = 84,
    Skull = 89,
    Pramh = 90,
    Armachd_AegisSphere = 94,
    BeagSuain = 97,
    Dragon = 98,
    ClawFist = 99,
    WolfFangFist = 101,
    MantidScent = 113,
    FasNadur = 119,
    CatsHearing = 124,
    InnerFire = 126,
    DarkSeal = 133,
    Decay = 141,
    Purify_SummonerSilence = 143,
    Regeneration = 146,
    SkillBonus = 147,
    GreaterExpBonus = 148,
    CounterAttack = 150,
    FeralForm = 183,
    KaruraForm = 184,
    KomodasForm = 185,
    RestCloak = 203,
};

class Spell_Icons
{
public:
    Spell_Icons() = default;

    void AddSpellIcon(unsigned short icon, unsigned char color)
    {
        if (color > 0)
        {
	        auto spellIcon = static_cast<SpellIcon>(icon);
            messages[spellIcon] = "Has received spell Icon: " + std::to_string(icon);
        }
    }

    void RemoveSpellIcon(unsigned short icon)
    {
	    auto spellIcon = static_cast<SpellIcon>(icon);
        messages.erase(spellIcon);
    }

    bool HasSpellIcon(SpellIcon icon) const
    {
        return messages.find(icon) != messages.end();
    }

    bool HasDisease() const { return HasSpellIcon(SpellIcon::Disease); }
    bool HasMezmerizeWormSkullShout() const { return HasSpellIcon(SpellIcon::Mezmerize_WormSkull_Shout); }
    bool HasBlind() const { return HasSpellIcon(SpellIcon::Blind); }
    bool HasBeagCradh() const { return HasSpellIcon(SpellIcon::BeagCradh); }
    bool HasEisdCreutair() const { return HasSpellIcon(SpellIcon::EisdCreutair); }
    bool HasDoubleExpApBonus() const { return HasSpellIcon(SpellIcon::DoubleExpApBonus); }
    bool HasHide() const { return HasSpellIcon(SpellIcon::Hide); }
    bool HasNaomhAite() const { return HasSpellIcon(SpellIcon::NaomhAite); }
    bool HasCreagNeart() const { return HasSpellIcon(SpellIcon::CreagNeart); }
    bool HasBeannaich() const { return HasSpellIcon(SpellIcon::Beannaich); }
    bool HasPerfectDefenseWeatherProtection() const { return HasSpellIcon(SpellIcon::PerfectDefense_WeatherProtection); }
    bool HasSlanSilence() const { return HasSpellIcon(SpellIcon::Slan_Silence); }
    bool HasPoison() const { return HasSpellIcon(SpellIcon::Poison); }
    bool HasBurnPause() const { return HasSpellIcon(SpellIcon::Burn_Pause); }
    bool HasSleep() const { return HasSpellIcon(SpellIcon::Sleep); }
    bool HasFasDeireas() const { return HasSpellIcon(SpellIcon::FasDeireas); }
    bool HasDion() const { return HasSpellIcon(SpellIcon::Dion); }
    bool HasDeireasFaileas_AsgallFaileas() const { return HasSpellIcon(SpellIcon::DeireasFaileas_AsgallFaileas); }
    bool HasMist() const { return HasSpellIcon(SpellIcon::Mist); }
    bool HasCradh() const { return HasSpellIcon(SpellIcon::Cradh); }
    bool HasMorCradh() const { return HasSpellIcon(SpellIcon::MorCradh); }
    bool HasArdCradh() const { return HasSpellIcon(SpellIcon::ArdCradh); }
    bool HasSkull() const { return HasSpellIcon(SpellIcon::Skull); }
    bool HasPramh() const { return HasSpellIcon(SpellIcon::Pramh); }
    bool HasArmachd_AegisSphere() const { return HasSpellIcon(SpellIcon::Armachd_AegisSphere); }
    bool HasBeagSuain() const { return HasSpellIcon(SpellIcon::BeagSuain); }
    bool HasDragon() const { return HasSpellIcon(SpellIcon::Dragon); }
    bool HasClawFist() const { return HasSpellIcon(SpellIcon::ClawFist); }
    bool HasWolfFangFist() const { return HasSpellIcon(SpellIcon::WolfFangFist); }
    bool HasMantidScent() const { return HasSpellIcon(SpellIcon::MantidScent); }
    bool HasFasNadur() const { return HasSpellIcon(SpellIcon::FasNadur); }
    bool HasCatsHearing() const { return HasSpellIcon(SpellIcon::CatsHearing); }
    bool HasInnerFire() const { return HasSpellIcon(SpellIcon::InnerFire); }
    bool HasDarkSeal() const { return HasSpellIcon(SpellIcon::DarkSeal); }
    bool HasDecay() const { return HasSpellIcon(SpellIcon::Decay); }
    bool HasPurify_SummonerSilence() const { return HasSpellIcon(SpellIcon::Purify_SummonerSilence); }
    bool HasRegeneration() const { return HasSpellIcon(SpellIcon::Regeneration); }
    bool HasSkillBonus() const { return HasSpellIcon(SpellIcon::SkillBonus); }
    bool HasGreaterExpBonus() const { return HasSpellIcon(SpellIcon::GreaterExpBonus); }
    bool HasCounterAttack() const { return HasSpellIcon(SpellIcon::CounterAttack); }
    bool HasFeralForm() const { return HasSpellIcon(SpellIcon::FeralForm); }
    bool HasKaruraForm() const { return HasSpellIcon(SpellIcon::KaruraForm); }
    bool HasKomodasForm() const { return HasSpellIcon(SpellIcon::KomodasForm); }
    bool HasRestCloak() const { return HasSpellIcon(SpellIcon::RestCloak); }

private:
    std::map<SpellIcon, std::string> messages;
};
