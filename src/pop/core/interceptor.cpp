/**
 * @file interceptor.cpp
 * @brief Implementation of the Interceptor class
 */
#include "interceptor.h"
#include "../utils/logging.h"
#include "../network/packet_handler_registry.h"
#include "../game/game_data_manager.h"

#include <detours.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace core {

// Original send/recv function pointers
static SendFunctionPtr original_send_function = nullptr;
static RecvFunctionPtr original_recv_function = nullptr;

// Singleton instance
Interceptor* Interceptor::instance_ = nullptr;
std::mutex Interceptor::singleton_mutex_;

Interceptor& Interceptor::getInstance() {
    std::lock_guard<std::mutex> lock(singleton_mutex_);
    if (instance_ == nullptr) {
        instance_ = new Interceptor();
    }
    return *instance_;
}

void Interceptor::destroyInstance() {
    std::lock_guard<std::mutex> lock(singleton_mutex_);
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
    }
}

Interceptor::Interceptor() 
    : initialized_(false), 
      hooked_(false), 
      show_outgoing_(true), 
      show_incoming_(true) {
}

Interceptor::~Interceptor() {
    cleanup();
}

bool Interceptor::initialize(FARPROC sendFunction, FARPROC recvFunction) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        utils::Logging::warning("Interceptor already initialized");
        return true;
    }

    utils::Logging::info("Initializing Interceptor");

    try {
        // Store original function pointers
        original_send_function = reinterpret_cast<SendFunctionPtr>(sendFunction);
        original_recv_function = reinterpret_cast<RecvFunctionPtr>(recvFunction);

        if (!original_send_function || !original_recv_function) {
            utils::Logging::error("Invalid function pointers");
            return false;
        }

        // Initialize the packet handler registry
        network::PacketHandlerRegistry::getInstance().initialize();

        // Initialize the game data manager
        game::GameDataManager::getInstance().initialize();

        initialized_ = true;
        utils::Logging::info("Interceptor initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception during Interceptor initialization: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception during Interceptor initialization");
        return false;
    }
}

bool Interceptor::attachHooks() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        utils::Logging::error("Cannot attach hooks: Interceptor not initialized");
        return false;
    }

    if (hooked_) {
        utils::Logging::warning("Hooks already attached");
        return true;
    }

    utils::Logging::info("Attaching hooks");

    try {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // Attach to send function
        DetourAttach(&(PVOID&)original_send_function, SendFunctionStub);
        
        // Attach to recv function
        DetourAttach(&(PVOID&)original_recv_function, RecvFunctionStub);

        LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            utils::Logging::error("Error attaching hooks: " + std::to_string(result));
            return false;
        }

        hooked_ = true;
        utils::Logging::info("Hooks attached successfully");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception attaching hooks: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception attaching hooks");
        return false;
    }
}

bool Interceptor::detachHooks() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!hooked_) {
        utils::Logging::warning("Hooks not attached");
        return true;
    }

    utils::Logging::info("Detaching hooks");

    try {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // Detach from send function
        DetourDetach(&(PVOID&)original_send_function, SendFunctionStub);
        
        // Detach from recv function
        DetourDetach(&(PVOID&)original_recv_function, RecvFunctionStub);

        LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            utils::Logging::error("Error detaching hooks: " + std::to_string(result));
            return false;
        }

        hooked_ = false;
        utils::Logging::info("Hooks detached successfully");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception detaching hooks: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception detaching hooks");
        return false;
    }
}

void Interceptor::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (hooked_) {
        detachHooks();
    }

    initialized_ = false;
    utils::Logging::info("Interceptor cleaned up");
}

void Interceptor::setShowOutgoing(bool show) {
    std::lock_guard<std::mutex> lock(mutex_);
    show_outgoing_ = show;
}

void Interceptor::setShowIncoming(bool show) {
    std::lock_guard<std::mutex> lock(mutex_);
    show_incoming_ = show;
}

bool Interceptor::getShowOutgoing() const {
    return show_outgoing_.load();
}

bool Interceptor::getShowIncoming() const {
    return show_incoming_.load();
}

void Interceptor::processOutgoingPacket(const network::Packet& packet) {
    if (getShowOutgoing()) {
        utils::Logging::debug("Outgoing packet: " + packetToHexString(packet));
    }

    // Process the packet with registered handlers
    network::PacketHandlerRegistry::getInstance().processPacket(packet, true);
}

void Interceptor::processIncomingPacket(const network::Packet& packet) {
    if (getShowIncoming()) {
        utils::Logging::debug("Incoming packet: " + packetToHexString(packet));
    }

    // Process the packet with registered handlers
    network::PacketHandlerRegistry::getInstance().processPacket(packet, false);
}

std::string Interceptor::packetToHexString(const network::Packet& packet) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase;

    for (size_t i = 0; i < packet.size(); ++i) {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(packet[i]);
        if (i < packet.size() - 1) {
            oss << " ";
        }
    }

    return oss.str();
}

// Static function stubs that will be used by Detours

int __stdcall SendFunctionStub(BYTE* data, int arg1, int arg2, char arg3) noexcept {
    try {
        // Create a packet from the data
        network::Packet packet(data, arg1);
        
        // Process the packet
        Interceptor::getInstance().processOutgoingPacket(packet);
        
        // Call the original function
        return original_send_function(data, arg1, arg2, arg3);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in SendFunctionStub: " + std::string(e.what()));
    }
    catch (...) {
        utils::Logging::error("Unknown exception in SendFunctionStub");
    }
    
    // Call the original function even if an exception occurs
    return original_send_function(data, arg1, arg2, arg3);
}

DWORD __stdcall RecvFunctionStub(DWORD* arg1, BYTE* data) noexcept {
    // Call the original function first
    DWORD result = original_recv_function(arg1, data);
    
    try {
        if (result > 0) {
            // Create a packet from the data
            network::Packet packet(data, result);
            
            // Process the packet
            Interceptor::getInstance().processIncomingPacket(packet);
        }
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in RecvFunctionStub: " + std::string(e.what()));
    }
    catch (...) {
        utils::Logging::error("Unknown exception in RecvFunctionStub");
    }
    
    return result;
}

} // namespace core
