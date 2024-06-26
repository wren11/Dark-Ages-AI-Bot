#include "pch.h"
#include "SpellData.h"

const std::map<std::string, int> SpellData::baseSpellLines = {
    {"ao ard cradh", 1},
    {"ao beag cradh", 1},
    {"ao cradh", 1},
    {"ao mor cradh", 1},
    {"ao dall", 1},
    {"ao puinsein", 1},
    {"ao suain", 1},
    {"ard athar", 4},
    {"ard athar nadur", 0},
    {"ard creag nadur", 0},
    {"ard cradh", 3},
    {"ard creag", 4},
    {"ard fas nadur", 4},
    {"ard ioc", 2},
    {"ard naomh aite", 4},
    {"ard puinneag spiorad", 0},
    {"ard sal", 4},
    {"ard sal nadur", 0},
    {"ard srad", 4},
    {"ard srad nadur", 0},
    {"armachd", 2},
    {"athar", 2},
    {"athar gar", 4},
    {"athar lamh", 1},
    {"beag breisleich", 1},
    {"beag cradh", 2},
    {"beag fas nadur", 2},
    {"beag ioc", 1},
    {"beag ioc comlha", 1},
    {"beag ioc fein", 1},
    {"beag naomh aite", 3},
    {"beag nochd", 5},
    {"beag pramh", 4},
    {"beag puinneag spiorad", 4},
    {"beag puinsein", 4},
    {"beag seun", 3},
    {"beag slan", 3},
    {"beannaich", 2},
    {"breisleich", 1},
    {"Bubble Block", 0},
    {"Bubble Shield", 1},
    {"Cat's Hearing", 5},
    {"Counter Attack 1", 2},
    {"Counter Attack 2", 2},
    {"Counter Attack 3", 2},
    {"Counter Attack 4", 2},
    {"Counter Attack 5", 2},
    {"Counter Attack 6", 2},
    {"Counter Attack 7", 2},
    {"Counter Attack 8", 2},
    {"cradh", 3},
    {"creag", 2},
    {"creag gar", 4},
    {"creag lamh", 1},
    {"creag neart", 0},
    {"Cursed Tune 1", 0},
    {"Cursed Tune 2", 0},
    {"Cursed Tune 3", 0},
    {"Cursed Tune 4", 0},
    {"Cursed Tune 5", 0},
    {"Cursed Tune 6", 0},
    {"Cursed Tune 7", 0},
    {"Cyclone", 0},
    {"dachaidh", 2},
    {"dall", 4},
    {"Dark Seal", 6},
    {"Darker Seal", 6},
    {"deireas faileas", 3},
    {"deo lamh", 1},
    {"deo saighead", 3},
    {"deo searg", 4},
    {"deo searg gar", 4},
    {"dion", 0},
    {"Disenchanter", 4},
    {"Draco Stance", 0},
    {"eisd creutair", 0},
    {"fas deireas", 1},
    {"fas nadur", 3},
    {"fas spiorad", 0},
    {"Firey Defender", 4},
    {"Frost Arrow 1", 1},
    {"Frost Arrow 2", 1},
    {"Frost Arrow 3", 1},
    {"Frost Arrow 4", 1},
    {"Frost Arrow 5", 1},
    {"Frost Arrow 6", 1},
    {"Gentle Touch", 1},
    {"Great Blind Snare", 0},
    {"Great Poison Snare", 0},
    {"Great Sleep", 0},
    {"Groo 1", 1},
    {"Groo 2", 1},
    {"Groo 3", 1},
    {"Groo 4", 2},
    {"Groo 5", 2},
    {"Groo 6", 2},
    {"Groo 7", 2},
    {"Groo 8", 2},
    {"Groo 9", 2},
    {"Groo 10", 2},
    {"Groo 11", 2},
    {"Groo 12", 2},
    {"Hide", 0},
    {"Howl", 5},
    {"Inner Fire", 5},
    {"Insult", 2},
    {"ioc", 1},
    {"ioc comlha", 2},
    {"Iron Skin", 2},
    {"Keeter 1", 1},
    {"Keeter 2", 1},
    {"Keeter 3", 1},
    {"Keeter 4", 1},
    {"Keeter 5", 1},
    {"Keeter 6", 2},
    {"Keeter 7", 2},
    {"Keeter 8", 2},
    {"Keeter 9", 2},
    {"Keeter 10", 2},
    {"Keeter 11", 2},
    {"Keeter 12", 2},
    {"Kelberoth Stance", 0},
    {"Lyliac Plant", 0},
    {"Lyliac Vineyard", 0},
    {"Maiden Trap", 7},
    {"Mana Ward", 0},
    {"Master Feral Form", 0},
    {"Master Karura Form", 0},
    {"Master Komodas Form", 0},
    {"Mermaid 1", 1},
    {"Mermaid 2", 1},
    {"Mermaid 3", 1},
    {"Mermaid 4", 1},
    {"Mermaid 5", 1},
    {"Mermaid 6", 2},
    {"Mermaid 7", 2},
    {"Mermaid 8", 2},
    {"Mermaid 9", 2},
    {"Mermaid 10", 2},
    {"Mermaid 11", 2},
    {"Mermaid 12", 2},
    {"Mist", 0},
    {"mor athar", 4},
    {"mor beannaich", 2},
    {"mor breisleich", 1},
    {"mor cradh", 3},
    {"mor creag", 4},
    {"mor dion", 2},
    {"mor dion comlha", 4},
    {"mor fas nadur", 4},
    {"mor ioc", 2},
    {"mor ioc comlha", 2},
    {"mor naomh aite", 4},
    {"mor pian na dion", 1},
    {"mor puinneag spiorad", 1},
    {"mor sal", 4},
    {"mor slan", 0},
    {"mor srad", 4},
    {"mor strioch pian gar", 1},
    {"mor strioch bais", 4},
    {"mor strioch bais lamh", 0},
    {"mor strioch bais mealll", 0},
    {"Mud Wall", 0},
    {"naomh aite", 2},
    {"nuadhaich", 2},
    {"pian na dion", 1},
    {"pramh", 4},
    {"puinneag breatha", 0},
    {"puinneag spiorad", 0},
    {"puinsein", 3},
    {"Reflection", 8},
    {"Regeneration 1", 0},
    {"Regeneration 2", 0},
    {"Regeneration 3", 0},
    {"Regeneration 4", 0},
    {"Regeneration 5", 0},
    {"Regeneration 6", 0},
    {"Regeneration 7", 0},
    {"sal", 2},
    {"sal gar", 4},
    {"sal lamh", 1},
    {"seun", 2},
    {"Shock Arrow", 0},
    {"slan", 0},
    {"Snort", 1},
    {"sonruich nadur", 5},
    {"spion beathach", 4},
    {"spit", 1},
    {"srad", 2},
    {"srad gar", 4},
    {"srad lamh", 1},
    {"Star Arrow 1", 0},
    {"Star Arrow 2", 0},
    {"Star Arrow 3", 0},
    {"Star Arrow 4", 0},
    {"Star Arrow 5", 0},
    {"Star Arrow 6", 0},
    {"Star Arrow 7", 0},
    {"Star Arrow 8", 0},
    {"Star Arrow 9", 0},
    {"Stone Skin", 0},
    {"suain", 4},
    {"taunt", 4},
    {"Torch 1", 1},
    {"Torch 2", 1},
    {"Torch 3", 1},
    {"Torch 4", 1},
    {"Torch 5", 1},
    {"Torch 6", 2},
    {"Torch 7", 2},
    {"Torch 8", 2},
    {"Torch 9", 2},
    {"Torch 10", 2},
    {"Torch 11", 2},
    {"Torch 12", 2},
    {"White Bat Stance", 0},
    {"Wild Feral Form", 0},
    {"Wild Komodas Form", 0},
    {"Wild Karura Form", 0},
    {"Wings of Protection", 0},
    {"Wraith Touch", 4},
    {"Zombie Defender", 4},
    {"Fierce Karura Form", 0},
    {"Karura Form", 0},
    {"Fierce Feral Form", 0},
    {"Feral Form", 0},
    {"Fierce Komodas Form", 0},
    {"Komodas Form", 0},
    {"ard ioc comlha", 2}};
