#ifndef SPELL_H
#define SPELL_H

#include "pch.h"

class spell
{
public:
    std::string name;
    BYTE slot;
    unsigned short icon;
    BYTE type;
    std::string prompt;
    BYTE castLines;

    spell() : slot(0), icon(0), type(0), castLines(0)
    {
    }

    spell(const std::string &name, const BYTE &slot, unsigned short icon, const BYTE &type, const std::string &prompt, const BYTE &castLines)
        : name(name), slot(slot), icon(icon), type(type), prompt(prompt), castLines(castLines)
    {
    }
};
#endif
