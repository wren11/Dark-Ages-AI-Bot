/**
 * @file network_interface.h
 * @brief Interface for native game network functions
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "../network/packet.h"

namespace game {

/**
 * @brief Interface for native game functions
 * 
 * This class provides a clean interface for interacting with
 * the native game functions while hiding the implementation
 * details of memory addresses and assembly code.
 */
class NetworkInterface {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static NetworkInterface& getInstance();
    
    /**
     * @brief Initialize the network interface
     * @return True if initialization succeeded
     */
    bool initialize();

    /**
     * @brief Send a packet to the server
     * @param packet Packet to send
     * @return True if sending succeeded
     */
    bool sendToServer(const network::Packet& packet);
    
    /**
     * @brief Send a packet to the client
     * @param packet Packet to send
     * @return True if sending succeeded
     */
    bool sendToClient(const network::Packet& packet);
    
    /**
     * @brief Walk in a direction
     * @param direction Direction to walk (0-3)
     */
    void walk(std::uint8_t direction);
    
    /**
     * @brief Follow an object
     * @param objectId ID of object to follow
     */
    void followObject(std::uint32_t objectId);
    
    /**
     * @brief Lock player movement
     */
    void lockMovement();
    
    /**
     * @brief Unlock player movement
     */
    void unlockMovement();
    
    /**
     * @brief Check if player can move
     * @return True if player can move
     */
    bool canMove();
    
    /**
     * @brief Open menu for an object
     * @param objectId ID of object to open menu for
     */
    void openMenu(std::uint32_t objectId);
    
    /**
     * @brief Cast a spell
     * @param spellSlot Slot of spell to cast
     * @param targetId ID of target
     * @param spellName Name of spell
     */
    void castSpell(std::uint8_t spellSlot, std::uint32_t targetId, const std::string& spellName);
    
    /**
     * @brief Send face direction command
     * @param direction Direction to face
     */
    void faceDirection(std::uint8_t direction);
    
    /**
     * @brief Send attack command
     */
    void attack();
    
    /**
     * @brief Send F5 key command
     */
    void pressF5Key();
    
    /**
     * @brief Use item from inventory
     * @param slot Slot of item
     */
    void useItem(std::uint8_t slot);
    
    /**
     * @brief Use skill
     * @param slot Slot of skill
     */
    void useSkill(std::uint8_t slot);
    
    /**
     * @brief Cast spell by slot
     * @param slot Slot of spell
     */
    void castSpellBySlot(std::uint8_t slot);
    
    /**
     * @brief Remove item from slot
     * @param slot Slot of item
     */
    void removeItem(std::uint8_t slot);
    
    /**
     * @brief Equip item to a specific slot
     * @param inventorySlot Slot of item in inventory
     * @param equipSlot Slot to equip to
     */
    void equipItem(std::uint8_t inventorySlot, std::uint8_t equipSlot);
    
    /**
     * @brief Drop item from inventory
     * @param slot Slot of item
     * @param amount Amount to drop
     */
    void dropItem(std::uint8_t slot, std::uint16_t amount);

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    NetworkInterface();
    
    /**
     * @brief Private destructor
     */
    ~NetworkInterface();
    
    /**
     * @brief Read memory from game process
     * @tparam T Type to read
     * @param address Memory address to read from
     * @return Value read from memory
     */
    template <typename T>
    T readMemory(std::uintptr_t address) const {
        try {
            return *reinterpret_cast<T*>(address);
        }
        catch (...) {
            // Log error in a way that doesn't depend on the implementation
            return T{};
        }
    }
    
    /**
     * @brief Write memory to game process
     * @tparam T Type to write
     * @param address Memory address to write to
     * @param value Value to write
     */
    template <typename T>
    void writeMemory(std::uintptr_t address, T value) const {
        try {
            *reinterpret_cast<T*>(address) = value;
        }
        catch (...) {
            // Log error in a way that doesn't depend on the implementation
        }
    }
    
    /**
     * @brief Get the 'this' pointer for the game context
     * @return This pointer value
     */
    std::int32_t getThisPointer() const;
    
    /**
     * @brief Call a native game function safely
     * @tparam ReturnT Return type of function
     * @tparam Args Argument types
     * @param functionAddress Address of function to call
     * @param args Arguments to pass to function
     * @return Return value from function
     */
    template <typename ReturnT, typename... Args>
    ReturnT callNativeFunction(std::uintptr_t functionAddress, Args... args) const {
        // Type-define the function pointer type
        using FunctionType = ReturnT(__cdecl*)(Args...);
        auto function = reinterpret_cast<FunctionType>(functionAddress);
        
        if (!function) {
            return ReturnT{};
        }
        
        try {
            return function(args...);
        }
        catch (...) {
            return ReturnT{};
        }
    }

    // Memory addresses for game functions and data
    static constexpr std::uintptr_t THIS_POINTER_ADDRESS = 0x00882E68;
    static constexpr std::uintptr_t WALK_FUNCTION_ADDRESS = 0x005F0C40;
    static constexpr std::uintptr_t FOLLOW_FUNCTION_ADDRESS = 0x005F4A70;
    static constexpr std::uintptr_t MOVEMENT_STATE_ADDRESS = 0x5F0ADE;
    static constexpr std::uintptr_t OPEN_MENU_FUNCTION_ADDRESS = 0x005F4730;
    static constexpr std::uintptr_t CAST_SPELL_FUNCTION_ADDRESS = 0x0049A3B0;
    static constexpr std::uintptr_t SENDER_OFFSET_ADDRESS = 0x00833880; // From constants.h
    static constexpr std::uintptr_t SEND_FUNCTION_ADDRESS = 0x00567FB0; // From constants.h
    static constexpr std::uintptr_t RECV_FUNCTION_ADDRESS = 0x00467060; // From constants.h
};

} // namespace game
