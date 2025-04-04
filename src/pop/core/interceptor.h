/**
 * @file interceptor.h
 * @brief Core packet interception class for the bot
 */
#pragma once

#include <mutex>
#include <atomic>
#include <string>
#include <windows.h>

#include "../network/packet.h"

namespace core {

// Function pointer types for the hooked functions
using SendFunctionPtr = int(__stdcall*)(BYTE*, int, int, char);
using RecvFunctionPtr = DWORD(__stdcall*)(DWORD*, BYTE*);

// Forward declarations of stub functions that will be used by Detours
int __stdcall SendFunctionStub(BYTE* data, int arg1, int arg2, char arg3) noexcept;
DWORD __stdcall RecvFunctionStub(DWORD* arg1, BYTE* data) noexcept;

/**
 * @brief Interface for classes that handle network packets
 * 
 * Defines the contract for packet handlers used by the Interceptor.
 */
class IPacketHandler {
public:
    virtual ~IPacketHandler() = default;

    /**
     * @brief Handle an outgoing packet (client to server)
     * @param packet The packet being sent
     * @return True if handling was successful, false otherwise
     */
    virtual bool handleOutgoingPacket(const network::Packet& packet) = 0;

    /**
     * @brief Handle an incoming packet (server to client)
     * @param packet The packet being received
     * @return True if handling was successful, false otherwise
     */
    virtual bool handleIncomingPacket(const network::Packet& packet) = 0;
};

/**
 * @brief Core class managing packet interception
 * 
 * Implements a thread-safe singleton pattern to provide global access
 * to packet interception functionality. Uses the Detours library
 * to hook into the game's network functions.
 */
class Interceptor {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static Interceptor& getInstance();
    
    /**
     * @brief Destroy the singleton instance
     * 
     * This should be called during DLL unload to ensure proper cleanup.
     */
    static void destroyInstance();

    /**
     * @brief Prevent copying
     */
    Interceptor(const Interceptor&) = delete;
    
    /**
     * @brief Prevent assignment
     */
    Interceptor& operator=(const Interceptor&) = delete;

    /**
     * @brief Initialize the interceptor
     * 
     * Sets up the interceptor with function pointers and initializes
     * related systems like packet handlers.
     * 
     * @param sendFunction Pointer to the game's send function
     * @param recvFunction Pointer to the game's receive function
     * @return True if initialization succeeded
     */
    bool initialize(FARPROC sendFunction, FARPROC recvFunction);
    
    /**
     * @brief Attach function hooks
     * 
     * Uses Detours to hook into the game's network functions.
     * 
     * @return True if hooks were attached successfully
     */
    bool attachHooks();
    
    /**
     * @brief Detach function hooks
     * 
     * Removes hooks from the game's network functions.
     * 
     * @return True if hooks were detached successfully
     */
    bool detachHooks();
    
    /**
     * @brief Clean up resources
     * 
     * Detaches hooks and releases resources.
     */
    void cleanup();

    /**
     * @brief Set whether to show outgoing packets
     * @param show True to show outgoing packets
     */
    void setShowOutgoing(bool show);
    
    /**
     * @brief Set whether to show incoming packets
     * @param show True to show incoming packets
     */
    void setShowIncoming(bool show);
    
    /**
     * @brief Get whether to show outgoing packets
     * @return True if outgoing packets should be shown
     */
    bool getShowOutgoing() const;
    
    /**
     * @brief Get whether to show incoming packets
     * @return True if incoming packets should be shown
     */
    bool getShowIncoming() const;

    /**
     * @brief Process an outgoing packet
     * 
     * This is called by the SendFunctionStub.
     * 
     * @param packet The packet to process
     */
    void processOutgoingPacket(const network::Packet& packet);
    
    /**
     * @brief Process an incoming packet
     * 
     * This is called by the RecvFunctionStub.
     * 
     * @param packet The packet to process
     */
    void processIncomingPacket(const network::Packet& packet);

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Interceptor();
    
    /**
     * @brief Private destructor
     */
    ~Interceptor();

    /**
     * @brief Convert a packet to a hex string for logging
     * @param packet The packet to convert
     * @return Hex string representation of the packet
     */
    std::string packetToHexString(const network::Packet& packet);

    bool initialized_;                    // Whether the interceptor is initialized
    bool hooked_;                         // Whether hooks are currently applied
    std::atomic<bool> show_outgoing_;     // Whether to show outgoing packets
    std::atomic<bool> show_incoming_;     // Whether to show incoming packets
    
    std::mutex mutex_;                    // Mutex for thread safety
    
    // Static singleton instance and mutex
    static Interceptor* instance_;
    static std::mutex singleton_mutex_;
    
    // Friends for access to private members
    friend int __stdcall SendFunctionStub(BYTE*, int, int, char) noexcept;
    friend DWORD __stdcall RecvFunctionStub(DWORD*, BYTE*) noexcept;
};

} // namespace core
