/**
 * @file dll_main.cpp
 * @brief Main entry point for the Dark Ages AI Bot DLL
 */
#include <windows.h>
#include <memory>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>

#include "core/interceptor.h"
#include "network/packet_handler_registry.h"
#include "game/game_data_manager.h"
#include "utils/logging.h"
#include "utils/input_handler.h"

// Game-specific function addresses - these will need to be updated for each game version
constexpr DWORD SEND_FUNCTION_ADDRESS = 0x00567FB0;
constexpr DWORD RECV_FUNCTION_ADDRESS = 0x00467060;

// Global variables
HMODULE g_hModule = nullptr;
HANDLE g_consoleHandle = nullptr;
bool g_initialized = false;
std::mutex g_initMutex;

/**
 * @brief Initialize the console for output
 * @return True if successful
 */
bool initializeConsole() {
    // Create a console window for output
    if (!AllocConsole()) {
        return false;
    }
    
    // Redirect stdout to the console
    FILE* pFile = nullptr;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    
    // Get console handle for later use
    g_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Set console title
    SetConsoleTitle(L"Dark Ages AI Bot");
    
    // Log success
    utils::Logging::info("Console initialized");
    return true;
}

/**
 * @brief Clean up the console
 */
void cleanupConsole() {
    if (g_consoleHandle) {
        FreeConsole();
        g_consoleHandle = nullptr;
    }
}

/**
 * @brief Setup keyboard shortcuts for the bot
 */
void setupKeyboardShortcuts() {
    utils::InputHandler& inputHandler = utils::InputHandler::getInstance();
    
    // F9: Toggle display of outgoing packets
    inputHandler.registerKeyCallback(utils::KeyCode::F9, []() {
        auto& interceptor = core::Interceptor::getInstance();
        bool currentSetting = interceptor.getShowOutgoing();
        interceptor.setShowOutgoing(!currentSetting);
        utils::Logging::info(std::string("Outgoing packet display: ") + 
                             (interceptor.getShowOutgoing() ? "ENABLED" : "DISABLED"));
    });
    
    // F10: Toggle display of incoming packets
    inputHandler.registerKeyCallback(utils::KeyCode::F10, []() {
        auto& interceptor = core::Interceptor::getInstance();
        bool currentSetting = interceptor.getShowIncoming();
        interceptor.setShowIncoming(!currentSetting);
        utils::Logging::info(std::string("Incoming packet display: ") + 
                             (interceptor.getShowIncoming() ? "ENABLED" : "DISABLED"));
    });
    
    // F11: Dump current player list
    inputHandler.registerKeyCallback(utils::KeyCode::F11, []() {
        utils::Logging::info("Dumping player list:");
        
        auto& gameData = game::GameDataManager::getInstance();
        auto players = gameData.getAllPlayers();
        
        if (players.empty()) {
            utils::Logging::info("No players in memory");
        } else {
            for (const auto& player : players) {
                const auto& pos = player->getPosition();
                utils::Logging::info(player->getName() + " - ID: " + std::to_string(player->getSerialId()) + 
                                    " - Pos: (" + std::to_string(pos.getX()) + ", " + 
                                    std::to_string(pos.getY()) + ")");
            }
        }
    });
    
    // F12: Display help information
    inputHandler.registerKeyCallback(utils::KeyCode::F12, []() {
        utils::Logging::info("Keyboard Shortcuts:");
        utils::Logging::info("F9  - Toggle display of outgoing packets");
        utils::Logging::info("F10 - Toggle display of incoming packets");
        utils::Logging::info("F11 - Dump current player list");
        utils::Logging::info("F12 - Display this help information");
    });
    
    // Start the input handler
    inputHandler.start();
    utils::Logging::info("Keyboard shortcuts initialized");
}

/**
 * @brief Initialize all subsystems
 * @return True if successful
 */
bool initializeSubsystems() {
    std::lock_guard<std::mutex> lock(g_initMutex);
    
    if (g_initialized) {
        utils::Logging::warning("Subsystems already initialized");
        return true;
    }
    
    utils::Logging::info("Initializing subsystems...");
    
    try {
        // Initialize the interceptor with function addresses
        core::Interceptor& interceptor = core::Interceptor::getInstance();
        
        HMODULE gameModule = GetModuleHandle(NULL);
        if (!gameModule) {
            utils::Logging::error("Failed to get game module handle");
            return false;
        }
        
        // Get function addresses
        FARPROC sendFunc = (FARPROC)((DWORD)gameModule + SEND_FUNCTION_ADDRESS);
        FARPROC recvFunc = (FARPROC)((DWORD)gameModule + RECV_FUNCTION_ADDRESS);
        
        // Initialize the interceptor
        if (!interceptor.initialize(sendFunc, recvFunc)) {
            utils::Logging::error("Failed to initialize interceptor");
            return false;
        }
        
        // Attach hooks
        if (!interceptor.attachHooks()) {
            utils::Logging::error("Failed to attach hooks");
            return false;
        }
        
        // Initialize the PacketHandlerRegistry
        network::PacketHandlerRegistry::getInstance().initialize();
        
        // Initialize the GameDataManager
        game::GameDataManager::getInstance().initialize();
        
        // Setup keyboard shortcuts
        setupKeyboardShortcuts();
        
        g_initialized = true;
        utils::Logging::info("Subsystems initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception during initialization: " + std::string(e.what()));
        return false;
    }
    catch (...) {
        utils::Logging::error("Unknown exception during initialization");
        return false;
    }
}

/**
 * @brief Clean up all subsystems
 */
void cleanupSubsystems() {
    std::lock_guard<std::mutex> lock(g_initMutex);
    
    if (!g_initialized) {
        return;
    }
    
    utils::Logging::info("Cleaning up subsystems...");
    
    try {
        // Stop the input handler
        utils::InputHandler::getInstance().stop();
        
        // Clean up the interceptor
        core::Interceptor& interceptor = core::Interceptor::getInstance();
        interceptor.cleanup();
        
        // Destroy the interceptor instance
        core::Interceptor::destroyInstance();
        
        g_initialized = false;
        utils::Logging::info("Subsystems cleaned up successfully");
    }
    catch (const std::exception& e) {
        utils::Logging::error("Exception during cleanup: " + std::string(e.what()));
    }
    catch (...) {
        utils::Logging::error("Unknown exception during cleanup");
    }
}

/**
 * @brief Main DLL entry point
 * @param hModule Handle to DLL module
 * @param fdwReason Reason for calling function
 * @param lpReserved Reserved
 * @return TRUE on success, FALSE on failure
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            // Store the module handle
            g_hModule = hModule;
            
            // Initialize the console
            if (!initializeConsole()) {
                return FALSE;
            }
            
            // Initialize the logging system
            utils::Logging::initialize();
            utils::Logging::info("DLL attached to process");
            
            // Initialize subsystems
            if (!initializeSubsystems()) {
                utils::Logging::error("Failed to initialize subsystems");
                cleanupConsole();
                return FALSE;
            }
            
            // Output success message
            utils::Logging::info("Dark Ages AI Bot initialized successfully");
            utils::Logging::info("Press F12 for help on keyboard shortcuts");
            break;
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            // No action needed
            break;
            
        case DLL_PROCESS_DETACH:
            utils::Logging::info("DLL detaching from process");
            
            // Clean up subsystems
            cleanupSubsystems();
            
            // Clean up logging
            utils::Logging::cleanup();
            
            // Clean up console
            cleanupConsole();
            break;
    }
    
    return TRUE;
}
