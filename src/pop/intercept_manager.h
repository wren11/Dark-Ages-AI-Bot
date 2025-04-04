#pragma once
#include "pch.h"
#include <atomic>
#include <mutex>
#include <future>

// Forward declarations
namespace network {
    class Packet;
}

/**
 * @brief Manager for network interception
 * 
 * Handles hooking of game network functions and routing of
 * packets to the appropriate handlers.
 */
class intercept_manager
{
public:
    /**
     * @brief Initialize the intercept manager
     */
    static void Initialize();
    
    /**
     * @brief Attach hooks to game functions
     * @return True if hooks were successfully attached
     */
    static bool AttachHook();
    
    /**
     * @brief Remove hooks from game functions
     * @return True if hooks were successfully removed
     */
    static bool RemoveHook();

    // Delete copy and move constructors/assignments to ensure singleton pattern
    intercept_manager(const intercept_manager&) = delete;
    intercept_manager& operator=(const intercept_manager&) = delete;
    intercept_manager(intercept_manager&&) = delete;
    intercept_manager& operator=(intercept_manager&&) = delete;

private:
    // Private constructor for singleton pattern
    intercept_manager() = default;
    ~intercept_manager() = default;
    
    // Safer hook management with atomic flags
    static std::atomic<bool> hooks_applied_;
    static std::mutex hook_mutex_;
    static std::future<void> hook_future_;

    // Stubs with exception safety
    static int __stdcall SendFunctionStub(BYTE* data, int arg1, int arg2, char arg3) noexcept;
    static int __stdcall RecvFunctionStub(BYTE* data, int arg1) noexcept;

    // Packet processing handlers
    static void on_packet_send(const network::Packet& packet);
    static void on_packet_recv(const network::Packet& packet);

    // Initialization helpers
    static void initialize_game_state();
    static void initialize_handlers();

    // Function pointer typedefs and storage
    using PFN_ORIGINAL_SEND = int(__stdcall*)(BYTE* data, int arg1, int arg2, char arg3);
    using PFN_ORIGINAL_RECV = int(__stdcall*)(BYTE* data, int arg1);

    static PFN_ORIGINAL_SEND TrueSendFunction;
    static PFN_ORIGINAL_RECV TrueRecvFunction;

    // Constants for function addresses
    static const DWORD sendPacketOutgoing;
    static const DWORD recvPacketIncoming;
};
