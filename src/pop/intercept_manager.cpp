#include "pch.h"
#include "intercept_manager.h"
#include <future>
#include <random>

// Use the new packet system
#include "network/packet.h"
#include "network/packet_reader.h"
#include "network/packet_writer.h"
#include "network/packet_handler_registry.h"
#include "game/network_interface.h"
#include "game/sprite_manager.h"
#include "utils/logging.h"

// Static member initialization
intercept_manager::PFN_ORIGINAL_SEND intercept_manager::TrueSendFunction = nullptr;
intercept_manager::PFN_ORIGINAL_RECV intercept_manager::TrueRecvFunction = nullptr;
std::atomic<bool> intercept_manager::hooks_applied_(false);
std::mutex intercept_manager::hook_mutex_;
std::future<void> intercept_manager::hook_future_ = std::future<void>();

// Memory addresses for hooking
const DWORD intercept_manager::sendPacketOutgoing = 0x00567FB0;
const DWORD intercept_manager::recvPacketIncoming = 0x00467060;

// Exception handler for safer operation
LONG WINAPI VectoredExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    // Log the exception information with improved logging
    utils::Logging::error("Exception caught in hook: 0x" + 
                         utils::Logging::hexString(pExceptionInfo->ExceptionRecord->ExceptionCode) +
                         " at address 0x" + 
                         utils::Logging::hexString(reinterpret_cast<std::uintptr_t>(pExceptionInfo->ExceptionRecord->ExceptionAddress)));
    return EXCEPTION_CONTINUE_EXECUTION;
}

// Helper math functions with bounds checking
namespace {
    // Safe random engine
    std::mt19937& getRandomEngine() {
        static std::mt19937 engine(static_cast<unsigned long>(std::time(nullptr)));
        return engine;
    }
    
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    
    double multiply(double a, double b) {
        return a * b;
    }
    
    double divide(double a, double b) {
        if (std::abs(b) < 1e-10) {
            utils::Logging::warning("Division by near-zero value");
            return 0.0;
        }
        return a / b;
    }
    
    std::string formatValue(double value, int precision = 4) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
}

int __stdcall intercept_manager::SendFunctionStub(BYTE *data, int arg1, int arg2, char arg3) noexcept
{
    try {
        if (data == nullptr || arg1 < 2) {
            utils::Logging::warning("SendFunctionStub: Invalid packet data or length");
            return 0;
        }
        
        // Create a network::Packet from the raw data for modern interface
        network::Packet packet(data, static_cast<size_t>(arg1));
        
        // Process the packet with the PacketHandlerRegistry (outgoing = true)
        network::PacketHandlerRegistry::getInstance().processPacket(packet, true);
        
        // Process with sprite manager if needed
        if (data[0] == 0x33) {
            game::SpriteManager::getInstance().processSpritePacket(packet);
        }
        
        return TrueSendFunction(data, arg1, arg2, arg3);
    } 
    catch (const std::exception& e) {
        utils::Logging::error("Exception in SendFunctionStub: " + std::string(e.what()));
        return 0;
    }
    catch (...) {
        utils::Logging::error("Unknown exception in SendFunctionStub");
        return 0;
    }
}

int __stdcall intercept_manager::RecvFunctionStub(BYTE *data, int arg1) noexcept
{
    try {
        if (data == nullptr || arg1 < 2) {
            utils::Logging::warning("RecvFunctionStub: Invalid packet data or length");
            return 0;
        }
        
        // Create a network::Packet from the raw data for modern interface
        network::Packet packet(data, static_cast<size_t>(arg1));
        
        // Process the packet with the PacketHandlerRegistry (outgoing = false)
        network::PacketHandlerRegistry::getInstance().processPacket(packet, false);
        
        // Process with sprite manager if needed
        if (data[0] == 0x33 || data[0] == 0x34 || data[0] == 0x35 || data[0] == 0x36) {
            game::SpriteManager::getInstance().processSpritePacket(packet);
        }
        
        return TrueRecvFunction(data, arg1);
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in RecvFunctionStub: " + std::string(e.what()));
        return 0;
    }
    catch (...) {
        utils::Logging::error("Unknown exception in RecvFunctionStub");
        return 0;
    }
}

void intercept_manager::on_packet_send(const network::Packet& packet)
{
    try {
        std::uint8_t packetType = packet.size() > 0 ? packet[0] : 0;
        utils::Logging::trace("Sending packet type: 0x" + utils::Logging::hexString(packetType) + 
                            " (" + network::PacketHandlerRegistry::getInstance().getPacketName(packetType, true) + ")");
        
        // Additional packet processing can be done here
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in on_packet_send: " + std::string(e.what()));
    }
}

void intercept_manager::on_packet_recv(const network::Packet& packet)
{
    try {
        std::uint8_t packetType = packet.size() > 0 ? packet[0] : 0;
        utils::Logging::trace("Received packet type: 0x" + utils::Logging::hexString(packetType) + 
                            " (" + network::PacketHandlerRegistry::getInstance().getPacketName(packetType, false) + ")");
        
        // Additional packet processing can be done here
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception in on_packet_recv: " + std::string(e.what()));
    }
}

void intercept_manager::initialize_game_state()
{
    try {
        utils::Logging::info("Initializing game state...");
        
        // Initialize the NetworkInterface
        game::NetworkInterface::getInstance().initialize();
        
        // Initialize the SpriteManager
        game::SpriteManager::getInstance().initialize();
        
        utils::Logging::info("Game state initialized successfully");
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to initialize game state: " + std::string(e.what()));
    }
}

void intercept_manager::initialize_handlers()
{
    try {
        utils::Logging::info("Initializing packet handlers...");
        
        // Initialize the PacketHandlerRegistry
        network::PacketHandlerRegistry::getInstance().initialize();
        
        // Register additional packet names for better logging
        auto& registry = network::PacketHandlerRegistry::getInstance();
        
        // Incoming packets (server to client)
        registry.registerPacketName(0x01, "LoginResult", false);
        registry.registerPacketName(0x02, "Redirect", false);
        registry.registerPacketName(0x03, "CharacterList", false);
        registry.registerPacketName(0x04, "MapChange", false);
        registry.registerPacketName(0x0A, "HealthUpdate", false);
        registry.registerPacketName(0x0B, "ManaUpdate", false);
        registry.registerPacketName(0x0F, "MapData", false);
        registry.registerPacketName(0x10, "ChatMessage", false);
        registry.registerPacketName(0x11, "SystemMessage", false);
        registry.registerPacketName(0x12, "WarpEffect", false);
        registry.registerPacketName(0x1A, "CombatResult", false);
        registry.registerPacketName(0x33, "NewSprite", false);
        registry.registerPacketName(0x34, "SpriteUpdate", false);
        registry.registerPacketName(0x35, "SpriteAppearance", false);
        registry.registerPacketName(0x36, "RemoveSprite", false);
        
        // Outgoing packets (client to server)
        registry.registerPacketName(0x02, "LoginRequest", true);
        registry.registerPacketName(0x03, "SelectCharacter", true);
        registry.registerPacketName(0x04, "CreateCharacter", true);
        registry.registerPacketName(0x10, "ChatMessage", true);
        registry.registerPacketName(0x11, "FaceDirection", true);
        registry.registerPacketName(0x13, "Attack", true);
        registry.registerPacketName(0x15, "Walk", true);
        registry.registerPacketName(0x18, "DropGold", true);
        registry.registerPacketName(0x19, "CastSpell", true);
        registry.registerPacketName(0x1C, "UseItem", true);
        registry.registerPacketName(0x1E, "ClickObject", true);
        registry.registerPacketName(0x1F, "DropItem", true);
        registry.registerPacketName(0x25, "EquipItem", true);
        registry.registerPacketName(0x38, "PressF5", true);
        registry.registerPacketName(0x3E, "UseSkill", true);
        registry.registerPacketName(0x44, "RemoveItem", true);
        
        utils::Logging::info("Packet handlers initialized successfully");
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to initialize handlers: " + std::string(e.what()));
    }
}

void intercept_manager::Initialize()
{
    try {
        // Add vectored exception handler for increased stability
        AddVectoredExceptionHandler(1, VectoredExceptionHandler);
        
        // Initialize handlers and game state
        initialize_handlers();
        initialize_game_state();
        
        utils::Logging::info("Intercept manager initialized successfully");
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to initialize intercept manager: " + std::string(e.what()));
    }
}

bool intercept_manager::AttachHook()
{
    // Use lock to ensure thread safety during hook operations
    std::lock_guard<std::mutex> lock(hook_mutex_);
    
    // Prevent double attachment
    if (hooks_applied_) {
        utils::Logging::warning("Hooks already applied, skipping attachment");
        return false;
    }
    
    try {
        // Handle any previous hook thread
        if (hook_future_.valid()) {
            hook_future_.wait();
        }
        
        // Launch the hook operation asynchronously
        hook_future_ = std::async(std::launch::async, []() {
            try {
                // Hook the functions
                TrueSendFunction = reinterpret_cast<PFN_ORIGINAL_SEND>(DetourFunction(
                    reinterpret_cast<PBYTE>(sendPacketOutgoing),
                    reinterpret_cast<PBYTE>(SendFunctionStub)));

                TrueRecvFunction = reinterpret_cast<PFN_ORIGINAL_RECV>(DetourFunction(
                    reinterpret_cast<PBYTE>(recvPacketIncoming),
                    reinterpret_cast<PBYTE>(RecvFunctionStub)));
                    
                hooks_applied_ = true;
                utils::Logging::info("Hooks successfully applied");
            }
            catch (const std::exception& e) {
                utils::Logging::error("Failed to attach hooks: " + std::string(e.what()));
                hooks_applied_ = false;
            }
            catch (...) {
                utils::Logging::error("Unknown exception while attaching hooks");
                hooks_applied_ = false;
            }
        });
        
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to start hook thread: " + std::string(e.what()));
        return false;
    }
}

bool intercept_manager::RemoveHook()
{
    // Use lock to ensure thread safety during hook operations
    std::lock_guard<std::mutex> lock(hook_mutex_);
    
    // Check if hooks are applied
    if (!hooks_applied_) {
        utils::Logging::warning("No hooks applied, skipping removal");
        return false;
    }
    
    try {
        // Handle any previous hook thread
        if (hook_future_.valid()) {
            hook_future_.wait();
        }
        
        // Launch the unhook operation asynchronously
        hook_future_ = std::async(std::launch::async, []() {
            try {
                // Remove the hooks if they were applied
                if (TrueSendFunction) {
                    DetourRemove(reinterpret_cast<PBYTE>(TrueSendFunction), 
                                 reinterpret_cast<PBYTE>(SendFunctionStub));
                }
                
                if (TrueRecvFunction) {
                    DetourRemove(reinterpret_cast<PBYTE>(TrueRecvFunction), 
                                 reinterpret_cast<PBYTE>(RecvFunctionStub));
                }
                
                hooks_applied_ = false;
                utils::Logging::info("Hooks successfully removed");
            }
            catch (const std::exception& e) {
                utils::Logging::error("Failed to remove hooks: " + std::string(e.what()));
            }
            catch (...) {
                utils::Logging::error("Unknown exception while removing hooks");
            }
        });
        
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to start unhook thread: " + std::string(e.what()));
        return false;
    }
}
