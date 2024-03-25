#include "pch.h"
#include "GameStateManager.h"
#include "network_functions.h"
#include "packet_structures.h"
#include "obj_managers.h"
#include "packet_reader.h"
#include "structures.h"
#include <iostream>
#include <exception>

void ProcessSpritePacket(const std::vector<unsigned char> &packetData)
{
    try
    {
        PacketReader msg(packetData);
        msg.readByte(); // Skip packet ID

        auto count = msg.read<uint16_t>();
        for (uint16_t i = 0; i < count; ++i)
        {
            auto xCoord = msg.read<uint16_t>();
            auto yCoord = msg.read<uint16_t>();
            auto serial = msg.read<uint32_t>();
            auto image = msg.read<uint16_t>();
            auto color = msg.read<uint8_t>();
            auto display = msg.read<uint16_t>();

            if (image >= 0x4000 && image <= 0x8000)
            {
                // Monster or NPC
                auto spriteOffset = image - 0x4000;
                auto unknown1 = msg.read<uint8_t>();
                auto num4 = msg.read<uint8_t>(); // Potentially unused, consider removing if not needed
                auto unknown2 = msg.read<uint8_t>();
                auto type = msg.read<uint8_t>();

                if (type == 0x2)
                {
                    // NPC
                    auto name = msg.readString8();
                    std::cout << "NPC: [" << name << "]: " << xCoord << "," << yCoord << ": ID: " << serial << " Image: " << image << std::endl;
                }
                else
                {
                    // Monster
                    std::cout << "Monster: [" << spriteOffset << "]" << xCoord << "," << yCoord << ": ID: " << serial << " Image: " << image << std::endl;
                }
            }
            else if (image >= 0x8000)
            {
                // Item
                auto sprite = image - 0x8000;
                std::cout << "Item: [" << sprite << "]" << xCoord << "," << yCoord << ": ID: " << serial << " Image: " << image << std::endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught in ProcessSpritePacket: " << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught in ProcessSpritePacket." << '\n';
    }
}