/**
 * @file network_interface.cpp
 * @brief Implementation of the NetworkInterface class
 */
#include "pch.h"
#include "network_interface.h"
#include "../utils/logging.h"

#include <Windows.h>
#include <iostream>

// These were previously included from pch.h and constants.h
#define sendOffset 0x00567FB0
#define recvPacketin 0x00467060
#define senderOffset 0x00833880

namespace game {

NetworkInterface& NetworkInterface::getInstance() {
    static NetworkInterface instance;
    return instance;
}

NetworkInterface::NetworkInterface() {
    // Nothing to initialize yet
}

NetworkInterface::~NetworkInterface() {
    // Clean up resources if needed
}

bool NetworkInterface::initialize() {
    utils::Logging::info("Initializing NetworkInterface");
    
    // Validate that we can access key memory addresses
    try {
        auto thisPtr = getThisPointer();
        if (thisPtr <= 0) {
            utils::Logging::warning("Failed to get valid this pointer, but continuing initialization");
        } else {
            utils::Logging::debug("This pointer validated: " + std::to_string(thisPtr));
        }
        
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception during NetworkInterface initialization: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception during NetworkInterface initialization");
        return false;
    }
}

template <typename T>
T NetworkInterface::readMemory(std::uintptr_t address) const {
    try {
        return *reinterpret_cast<T*>(address);
    }
    catch (...) {
        utils::Logging::error("Failed to read memory at address 0x" + 
                              utils::Logging::hexString(static_cast<std::uint8_t>(address >> 24)) +
                              utils::Logging::hexString(static_cast<std::uint8_t>(address >> 16)) +
                              utils::Logging::hexString(static_cast<std::uint8_t>(address >> 8)) +
                              utils::Logging::hexString(static_cast<std::uint8_t>(address)));
        return T{};
    }
}

template <typename T>
void NetworkInterface::writeMemory(std::uintptr_t address, T value) const {
    try {
        *reinterpret_cast<T*>(address) = value;
    }
    catch (...) {
        utils::Logging::error("Failed to write memory at address 0x" + 
                             utils::Logging::hexString(static_cast<std::uint8_t>(address >> 24)) +
                             utils::Logging::hexString(static_cast<std::uint8_t>(address >> 16)) +
                             utils::Logging::hexString(static_cast<std::uint8_t>(address >> 8)) +
                             utils::Logging::hexString(static_cast<std::uint8_t>(address)));
    }
}

std::int32_t NetworkInterface::getThisPointer() const {
    try {
        auto valuePtr = reinterpret_cast<int*>(senderOffset);
        if (valuePtr == nullptr) {
            return -1;
        }
        return *valuePtr;
    }
    catch (...) {
        return -1;
    }
}

bool NetworkInterface::sendToServer(const network::Packet& packet) {
    if (packet.size() <= 0) {
        utils::Logging::error("Attempted to send empty packet to server");
        return false;
    }
    
    try {
        // Allocate memory for the packet
        void* memory = VirtualAlloc(nullptr, packet.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!memory) {
            utils::Logging::error("Failed to allocate memory for packet");
            return false;
        }
        
        // Copy packet data to allocated memory
        for (size_t i = 0; i < packet.size(); ++i) {
            reinterpret_cast<std::uint8_t*>(memory)[i] = packet[i];
        }
        
        // Get this pointer
        int thisPtr = getThisPointer();
        if (thisPtr <= 0) {
            utils::Logging::error("Invalid this pointer for send operation");
            VirtualFree(memory, 0, MEM_RELEASE);
            return false;
        }
        
        // Create safe function pointer to the send function
        using SendFunctionType = int(__thiscall*)(void*, void*, int);
        auto sendFunction = reinterpret_cast<SendFunctionType>(sendOffset);
        
        // Call the send function
        __try {
            sendFunction(reinterpret_cast<void*>(thisPtr), memory, static_cast<int>(packet.size()));
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            utils::Logging::error("Exception while sending packet to server");
            VirtualFree(memory, 0, MEM_RELEASE);
            return false;
        }
        
        // Free allocated memory
        VirtualFree(memory, 0, MEM_RELEASE);
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in sendToServer: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception in sendToServer");
        return false;
    }
}

bool NetworkInterface::sendToClient(const network::Packet& packet) {
    if (packet.size() <= 0) {
        utils::Logging::error("Attempted to send empty packet to client");
        return false;
    }
    
    try {
        // Allocate memory for the packet
        void* memory = VirtualAlloc(nullptr, packet.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!memory) {
            utils::Logging::error("Failed to allocate memory for packet");
            return false;
        }
        
        // Copy packet data to allocated memory
        for (size_t i = 0; i < packet.size(); ++i) {
            reinterpret_cast<std::uint8_t*>(memory)[i] = packet[i];
        }
        
        // Create safe function pointer to the receive function
        using RecvFunctionType = int(__cdecl*)(void*, int);
        auto recvFunction = reinterpret_cast<RecvFunctionType>(recvPacketin);
        
        // Call the receive function
        __try {
            recvFunction(memory, static_cast<int>(packet.size()));
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            utils::Logging::error("Exception while sending packet to client");
            VirtualFree(memory, 0, MEM_RELEASE);
            return false;
        }
        
        // Free allocated memory
        VirtualFree(memory, 0, MEM_RELEASE);
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in sendToClient: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception in sendToClient");
        return false;
    }
}

void NetworkInterface::walk(std::uint8_t direction) {
    try {
        auto thisPtr = reinterpret_cast<int*>(THIS_POINTER_ADDRESS);
        if (!thisPtr) {
            utils::Logging::error("Invalid this pointer for walk operation");
            return;
        }
        
        using WalkFunctionType = void(__cdecl*)(int*, std::uint8_t);
        auto walkFunction = reinterpret_cast<WalkFunctionType>(WALK_FUNCTION_ADDRESS);
        
        if (!walkFunction) {
            utils::Logging::error("Invalid walk function pointer");
            return;
        }
        
        __try {
            walkFunction(thisPtr, direction);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            utils::Logging::error("Exception in walk function");
        }
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in walk: " + std::string(e.what()));
    }
}

void NetworkInterface::followObject(std::uint32_t objectId) {
    try {
        auto thisPtr = readMemory<int>(THIS_POINTER_ADDRESS);
        if (thisPtr <= 0) {
            utils::Logging::error("Invalid this pointer for follow operation");
            return;
        }
        
        if (objectId <= 0) {
            utils::Logging::error("Invalid object ID for follow operation");
            return;
        }
        
        using FollowFunctionType = void(__thiscall*)(int*, int);
        auto followFunction = reinterpret_cast<FollowFunctionType>(FOLLOW_FUNCTION_ADDRESS);
        
        if (!followFunction) {
            utils::Logging::error("Invalid follow function pointer");
            return;
        }
        
        __try {
            followFunction(reinterpret_cast<int*>(thisPtr), static_cast<int>(objectId));
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            utils::Logging::error("Exception in follow function");
        }
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in followObject: " + std::string(e.what()));
    }
}

void NetworkInterface::lockMovement() {
    try {
        writeMemory<std::uint8_t>(MOVEMENT_STATE_ADDRESS + 1, 0x74);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in lockMovement: " + std::string(e.what()));
    }
}

void NetworkInterface::unlockMovement() {
    try {
        writeMemory<std::uint8_t>(MOVEMENT_STATE_ADDRESS + 1, 0x75);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in unlockMovement: " + std::string(e.what()));
    }
}

bool NetworkInterface::canMove() {
    try {
        return readMemory<std::uint8_t>(MOVEMENT_STATE_ADDRESS + 1) == 0x75;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in canMove: " + std::string(e.what()));
        return false;
    }
}

void NetworkInterface::openMenu(std::uint32_t objectId) {
    try {
        auto thisPtr = readMemory<int*>(THIS_POINTER_ADDRESS);
        if (!thisPtr) {
            utils::Logging::error("Invalid this pointer for open menu operation");
            return;
        }
        
        using OpenMenuFunctionType = void(__cdecl*)(int*, int);
        auto openMenuFunction = reinterpret_cast<OpenMenuFunctionType>(OPEN_MENU_FUNCTION_ADDRESS);
        
        if (!openMenuFunction) {
            utils::Logging::error("Invalid open menu function pointer");
            return;
        }
        
        __try {
            openMenuFunction(thisPtr, static_cast<int>(objectId));
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            utils::Logging::error("Exception in open menu function");
        }
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in openMenu: " + std::string(e.what()));
    }
}

void NetworkInterface::castSpell(std::uint8_t spellSlot, std::uint32_t targetId, const std::string& spellName) {
    try {
        // Get base address
        int baseAddress = readMemory<int>(THIS_POINTER_ADDRESS);
        if (baseAddress <= 0) {
            utils::Logging::error("Invalid base address for cast spell operation");
            return;
        }
        
        // Get intermediate address
        DWORD intermediateAddress = readMemory<DWORD>(baseAddress + 0x2CC);
        if (intermediateAddress == 0) {
            utils::Logging::error("Invalid intermediate address for cast spell operation");
            return;
        }
        
        // Calculate this pointer for spell casting
        unsigned __int8* thisPtr = reinterpret_cast<unsigned __int8*>(intermediateAddress + 0x4DFA + 0x05);
        
        // Set spell information
        writeMemory<std::uint8_t>(reinterpret_cast<std::uintptr_t>(thisPtr) + 400, spellSlot);
        writeMemory<std::uint8_t>(reinterpret_cast<std::uintptr_t>(thisPtr) + 661, 1);
        
        // Set spell name (with safety)
        const size_t maxNameLength = 99; // Leave room for null terminator in 100-byte buffer
        size_t nameLength = spellName.length() < maxNameLength ? spellName.length() : maxNameLength;
        for (size_t i = 0; i < nameLength; ++i) {
            writeMemory<char>(reinterpret_cast<std::uintptr_t>(thisPtr) + 405 + i, spellName[i]);
        }
        writeMemory<char>(reinterpret_cast<std::uintptr_t>(thisPtr) + 405 + nameLength, '\0');
        
        // Set up parameters
        unsigned int contextId = 15; // Example value from original code
        unsigned int additionalData = 4; // Example value from original code
        
        // Create function pointer
        using CastSpellFunctionType = void(__thiscall*)(void*, unsigned int, unsigned int, unsigned int);
        auto castSpellFunction = reinterpret_cast<CastSpellFunctionType>(CAST_SPELL_FUNCTION_ADDRESS);
        
        // Call the function
        __try {
            castSpellFunction(thisPtr, targetId, additionalData, contextId);
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            utils::Logging::error("Exception in cast spell function");
        }
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in castSpell: " + std::string(e.what()));
    }
}

void NetworkInterface::faceDirection(std::uint8_t direction) {
    // Create packet data
    std::uint8_t packetData[3] = {0x11, direction, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::attack() {
    // Create packet data
    std::uint8_t packetData[3] = {0x13, 0x01, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::pressF5Key() {
    // Create packet data
    std::uint8_t packetData[3] = {0x38, 0x01, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::useItem(std::uint8_t slot) {
    // Create packet data
    std::uint8_t packetData[3] = {0x1C, slot, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::useSkill(std::uint8_t slot) {
    // Create packet data
    std::uint8_t packetData[3] = {0x3E, slot, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::castSpellBySlot(std::uint8_t slot) {
    // Create packet data
    std::uint8_t packetData[3] = {0x0F, slot, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::removeItem(std::uint8_t slot) {
    // Create packet data
    std::uint8_t packetData[3] = {0x44, slot, 0x00};
    network::Packet packet(packetData, 3);
    sendToServer(packet);
}

void NetworkInterface::equipItem(std::uint8_t inventorySlot, std::uint8_t equipSlot) {
    try {
        // Create packet data for equipping an item
        // Packet format: [0x25][inventory slot][equip slot]
        std::uint8_t packetData[3] = {0x25, inventorySlot, equipSlot};
        network::Packet packet(packetData, 3);
        
        utils::Logging::debug("Equipping item from slot " + std::to_string(inventorySlot) + 
                             " to equip slot " + std::to_string(equipSlot));
        
        sendToServer(packet);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in equipItem: " + std::string(e.what()));
    }
}

void NetworkInterface::dropItem(std::uint8_t slot, std::uint16_t amount) {
    try {
        // Create packet data for dropping an item
        // Packet format: [0x1F][slot][amount_low][amount_high]
        std::uint8_t packetData[4] = {
            0x1F, 
            slot, 
            static_cast<uint8_t>(amount & 0xFF),  // low byte
            static_cast<uint8_t>((amount >> 8) & 0xFF)  // high byte
        };
        network::Packet packet(packetData, 4);
        
        utils::Logging::debug("Dropping " + std::to_string(amount) + 
                             " items from slot " + std::to_string(slot));
        
        sendToServer(packet);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in dropItem: " + std::string(e.what()));
    }
}

} // namespace game
