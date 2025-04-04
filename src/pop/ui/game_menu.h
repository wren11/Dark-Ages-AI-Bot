/**
 * @file game_menu.h
 * @brief In-game menu system for Dark Ages AI Bot
 */
#pragma once

#include "ui_manager.h"
#include "../utils/config/config_manager.h"
#include "../utils/memory/memory_scanner.h"
#include "../game/data_loader.h"
#include <functional>
#include <memory>
#include <string>

namespace ui {

/**
 * @brief Main menu for the Dark Ages AI Bot
 * 
 * Provides a UI for controlling the bot's features directly
 * within the game client.
 */
class GameMenu {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static GameMenu& getInstance();
    
    /**
     * @brief Initialize the game menu
     * @return True if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Show or hide the menu
     * @param visible Whether the menu should be visible
     */
    void setVisible(bool visible);
    
    /**
     * @brief Toggle menu visibility
     */
    void toggleVisibility();
    
    /**
     * @brief Check if menu is visible
     * @return True if menu is visible
     */
    bool isVisible() const;
    
    /**
     * @brief Update the menu UI
     * 
     * This should be called regularly to update the UI
     * based on the current game state.
     */
    void update();
    
    /**
     * @brief Register a hotkey to toggle the menu
     * @param key Virtual key code for the hotkey
     */
    void registerToggleHotkey(int key);

private:
    GameMenu();
    ~GameMenu();
    
    GameMenu(const GameMenu&) = delete;
    GameMenu& operator=(const GameMenu&) = delete;
    
    // Create different menu windows
    void createMainMenu();
    void createConfigMenu();
    void createScannerMenu();
    void createBotControlMenu();
    void createDebugMenu();
    
    // Helper methods
    void updateStatusLabels();
    void reloadConfiguration();
    void startMemoryScan(int scanType);
    void toggleBotFeature(const std::string& featureName);
    
    // UI elements
    WindowPtr mainWindow_;
    WindowPtr configWindow_;
    WindowPtr scannerWindow_;
    WindowPtr botControlWindow_;
    WindowPtr debugWindow_;
    
    // Status tracking
    bool botEnabled_ = false;
    bool autoLooting_ = false;
    bool autoHealing_ = false;
    bool autoBuffing_ = false;
    bool autoFollow_ = false;
    std::string targetName_ = "";
    
    // Labels to update
    LabelPtr statusLabel_ = nullptr;
    LabelPtr cpuUsageLabel_ = nullptr;
    LabelPtr memoryUsageLabel_ = nullptr;
    LabelPtr lastActionLabel_ = nullptr;
    
    // Toggle hotkey
    int toggleHotkey_ = VK_F8; // Default to F8
    bool initialized_ = false;
};

} // namespace ui
