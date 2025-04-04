/**
 * @file game_menu.cpp
 * @brief Implementation of the in-game menu system
 */
#include "game_menu.h"
#include "../utils/logging.h"
#include <Windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <psapi.h>
#include "../constants.h"

namespace ui {

// Helper functions for resource monitoring
float getCPUUsage() {
    static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    static int numProcessors;
    static HANDLE self;
    static bool initialized = false;
    
    if (!initialized) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numProcessors = sysInfo.dwNumberOfProcessors;
        
        self = GetCurrentProcess();
        initialized = true;
        
        FILETIME ftime, fsys, fuser;
        GetSystemTimeAsFileTime(&ftime);
        memcpy(&lastCPU, &ftime, sizeof(FILETIME));
        
        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
    }
    
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));
    
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    
    float percent = (sys.QuadPart - lastSysCPU.QuadPart) +
                   (user.QuadPart - lastUserCPU.QuadPart);
    
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;
    percent *= 100;
    
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;
    
    return percent;
}

float getMemoryUsageMB() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    
    // Convert from bytes to MB
    SIZE_T memoryUsageBytes = pmc.WorkingSetSize;
    float memoryUsageMB = static_cast<float>(memoryUsageBytes) / (1024 * 1024);
    
    return memoryUsageMB;
}

// GameMenu implementation
GameMenu& GameMenu::getInstance() {
    static GameMenu instance;
    return instance;
}

GameMenu::GameMenu() {
    // Initialize in initialize() method
}

GameMenu::~GameMenu() {
    // Clean up any resources
}

bool GameMenu::initialize() {
    if (initialized_) return true;
    
    try {
        // Initialize UI manager first
        auto& uiManager = UIManager::getInstance();
        if (!uiManager.initialize()) {
            utils::Logging::error("Failed to initialize UI manager");
            return false;
        }
        
        // Create the main menu and other windows
        createMainMenu();
        createConfigMenu();
        createScannerMenu();
        createBotControlMenu();
        createDebugMenu();
        
        // Hide config and other windows initially
        configWindow_->setVisible(false);
        scannerWindow_->setVisible(false);
        botControlWindow_->setVisible(false);
        debugWindow_->setVisible(false);
        
        initialized_ = true;
        utils::Logging::info("Game menu initialized");
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to initialize game menu: " + std::string(e.what()));
        return false;
    }
}

void GameMenu::setVisible(bool visible) {
    if (!initialized_) return;
    
    // Set visibility of all windows
    mainWindow_->setVisible(visible);
    
    // Only show the other windows if they were already visible
    // (don't automatically show them when toggling the main menu on)
    if (!visible) {
        configWindow_->setVisible(false);
        scannerWindow_->setVisible(false);
        botControlWindow_->setVisible(false);
        debugWindow_->setVisible(false);
    }
}

void GameMenu::toggleVisibility() {
    if (!initialized_) return;
    
    bool currentVisibility = mainWindow_->isVisible();
    setVisible(!currentVisibility);
}

bool GameMenu::isVisible() const {
    if (!initialized_) return false;
    
    return mainWindow_->isVisible();
}

void GameMenu::update() {
    if (!initialized_ || !isVisible()) return;
    
    // Update status labels
    updateStatusLabels();
}

void GameMenu::registerToggleHotkey(int key) {
    toggleHotkey_ = key;
}

void GameMenu::createMainMenu() {
    // Create main menu window
    auto& uiManager = UIManager::getInstance();
    
    mainWindow_ = uiManager.createWindow("Dark Ages AI Bot", 10, 10, 220, 350);
    mainWindow_->setId("mainMenu");
    mainWindow_->setBackgroundColor(Color(0, 0, 0, 220));
    mainWindow_->setBorderColor(Color(100, 100, 255, 200));
    
    // Create status label
    statusLabel_ = std::make_shared<Label>("Status: Ready", Rect(10, 30, 200, 20));
    statusLabel_->setId("statusLabel");
    statusLabel_->setTextColor(Color::Green());
    mainWindow_->addElement(statusLabel_);
    
    // Create CPU usage label
    cpuUsageLabel_ = std::make_shared<Label>("CPU: 0.0%", Rect(10, 50, 200, 20));
    cpuUsageLabel_->setId("cpuUsageLabel");
    cpuUsageLabel_->setTextColor(Color::White());
    mainWindow_->addElement(cpuUsageLabel_);
    
    // Create memory usage label
    memoryUsageLabel_ = std::make_shared<Label>("Memory: 0.0 MB", Rect(10, 70, 200, 20));
    memoryUsageLabel_->setId("memoryUsageLabel");
    memoryUsageLabel_->setTextColor(Color::White());
    mainWindow_->addElement(memoryUsageLabel_);
    
    // Create last action label
    lastActionLabel_ = std::make_shared<Label>("Last Action: None", Rect(10, 90, 200, 20));
    lastActionLabel_->setId("lastActionLabel");
    lastActionLabel_->setTextColor(Color::White());
    mainWindow_->addElement(lastActionLabel_);
    
    // Add separator line
    auto separatorLabel = std::make_shared<Label>("------------------------", Rect(10, 110, 200, 20));
    separatorLabel->setTextColor(Color(150, 150, 150));
    mainWindow_->addElement(separatorLabel);
    
    // Add main menu buttons
    auto configButton = std::make_shared<Button>("Configuration", Rect(10, 130, 200, 30));
    configButton->setId("configButton");
    configButton->setOnClick([this]() {
        configWindow_->setVisible(!configWindow_->isVisible());
    });
    mainWindow_->addElement(configButton);
    
    auto scannerButton = std::make_shared<Button>("Memory Scanner", Rect(10, 170, 200, 30));
    scannerButton->setId("scannerButton");
    scannerButton->setOnClick([this]() {
        scannerWindow_->setVisible(!scannerWindow_->isVisible());
    });
    mainWindow_->addElement(scannerButton);
    
    auto botControlButton = std::make_shared<Button>("Bot Controls", Rect(10, 210, 200, 30));
    botControlButton->setId("botControlButton");
    botControlButton->setOnClick([this]() {
        botControlWindow_->setVisible(!botControlWindow_->isVisible());
    });
    mainWindow_->addElement(botControlButton);
    
    auto debugButton = std::make_shared<Button>("Debug Tools", Rect(10, 250, 200, 30));
    debugButton->setId("debugButton");
    debugButton->setOnClick([this]() {
        debugWindow_->setVisible(!debugWindow_->isVisible());
    });
    mainWindow_->addElement(debugButton);
    
    auto exitButton = std::make_shared<Button>("Close Menu", Rect(10, 290, 200, 30));
    exitButton->setId("exitButton");
    exitButton->setBackgroundColor(Color(150, 30, 30, 200));
    exitButton->setHoverColor(Color(200, 40, 40, 200));
    exitButton->setOnClick([this]() {
        setVisible(false);
    });
    mainWindow_->addElement(exitButton);
}

void GameMenu::createConfigMenu() {
    // Create config menu window
    auto& uiManager = UIManager::getInstance();
    
    configWindow_ = uiManager.createWindow("Configuration", 240, 10, 250, 300);
    configWindow_->setId("configMenu");
    configWindow_->setBackgroundColor(Color(0, 0, 0, 220));
    configWindow_->setBorderColor(Color(100, 100, 255, 200));
    
    // Add config options
    auto titleLabel = std::make_shared<Label>("Bot Configuration", Rect(10, 30, 230, 20));
    titleLabel->setTextColor(Color(200, 200, 255));
    titleLabel->setFontStyle(FontStyle::Bold);
    configWindow_->addElement(titleLabel);
    
    // Add owner name text field (would need to implement TextField class)
    auto ownerLabel = std::make_shared<Label>("Bot Owner: ", Rect(10, 60, 230, 20));
    ownerLabel->setTextColor(Color::White());
    configWindow_->addElement(ownerLabel);
    
    // Feature toggles
    auto autoHealCheck = std::make_shared<CheckBox>("Auto Healing", false, Rect(10, 90, 230, 20));
    autoHealCheck->setId("autoHealCheck");
    autoHealCheck->setOnChange([this](bool checked) {
        autoHealing_ = checked;
        this->toggleBotFeature("AutoHealing");
    });
    configWindow_->addElement(autoHealCheck);
    
    auto autoLootCheck = std::make_shared<CheckBox>("Auto Looting", false, Rect(10, 120, 230, 20));
    autoLootCheck->setId("autoLootCheck");
    autoLootCheck->setOnChange([this](bool checked) {
        autoLooting_ = checked;
        this->toggleBotFeature("AutoLooting");
    });
    configWindow_->addElement(autoLootCheck);
    
    auto autoBuffCheck = std::make_shared<CheckBox>("Auto Buffing", false, Rect(10, 150, 230, 20));
    autoBuffCheck->setId("autoBuffCheck");
    autoBuffCheck->setOnChange([this](bool checked) {
        autoBuffing_ = checked;
        this->toggleBotFeature("AutoBuffing");
    });
    configWindow_->addElement(autoBuffCheck);
    
    auto targetLabel = std::make_shared<Label>("Target Mode:", Rect(10, 180, 100, 20));
    targetLabel->setTextColor(Color::White());
    configWindow_->addElement(targetLabel);
    
    auto targetMenu = std::make_shared<Menu>(
        std::vector<std::string>{"None", "Nearest", "Weakest", "Custom"}, Rect(110, 180, 130, 25));
    targetMenu->setId("targetMenu");
    targetMenu->setOnChange([](int index) {
        // Handle target mode change
    });
    configWindow_->addElement(targetMenu);
    
    auto saveButton = std::make_shared<Button>("Save Settings", Rect(10, 220, 110, 30));
    saveButton->setId("saveConfigButton");
    saveButton->setOnClick([this]() {
        this->reloadConfiguration();
    });
    configWindow_->addElement(saveButton);
    
    auto resetButton = std::make_shared<Button>("Reset Settings", Rect(130, 220, 110, 30));
    resetButton->setId("resetConfigButton");
    resetButton->setOnClick([]() {
        // Reset to default settings
    });
    configWindow_->addElement(resetButton);
}

void GameMenu::createScannerMenu() {
    // Create scanner menu window
    auto& uiManager = UIManager::getInstance();
    
    scannerWindow_ = uiManager.createWindow("Memory Scanner", 240, 320, 250, 230);
    scannerWindow_->setId("scannerMenu");
    scannerWindow_->setBackgroundColor(Color(0, 0, 0, 220));
    scannerWindow_->setBorderColor(Color(100, 100, 255, 200));
    
    // Add scanner options
    auto titleLabel = std::make_shared<Label>("Memory Scanning Tools", Rect(10, 30, 230, 20));
    titleLabel->setTextColor(Color(200, 200, 255));
    titleLabel->setFontStyle(FontStyle::Bold);
    scannerWindow_->addElement(titleLabel);
    
    auto infoLabel = std::make_shared<Label>("Find game functions automatically", Rect(10, 50, 230, 20));
    infoLabel->setTextColor(Color(180, 180, 180));
    scannerWindow_->addElement(infoLabel);
    
    auto quickScanButton = std::make_shared<Button>("Quick Scan", Rect(10, 80, 230, 30));
    quickScanButton->setId("quickScanButton");
    quickScanButton->setOnClick([this]() {
        this->startMemoryScan(0);
    });
    scannerWindow_->addElement(quickScanButton);
    
    auto thoroughScanButton = std::make_shared<Button>("Thorough Scan", Rect(10, 120, 230, 30));
    thoroughScanButton->setId("thoroughScanButton");
    thoroughScanButton->setOnClick([this]() {
        this->startMemoryScan(1);
    });
    scannerWindow_->addElement(thoroughScanButton);
    
    auto fullScanButton = std::make_shared<Button>("Full Scan (Slow)", Rect(10, 160, 230, 30));
    fullScanButton->setId("fullScanButton");
    fullScanButton->setOnClick([this]() {
        this->startMemoryScan(2);
    });
    scannerWindow_->addElement(fullScanButton);
}

void GameMenu::createBotControlMenu() {
    // Create bot control menu window
    auto& uiManager = UIManager::getInstance();
    
    botControlWindow_ = uiManager.createWindow("Bot Controls", 500, 10, 250, 200);
    botControlWindow_->setId("botControlMenu");
    botControlWindow_->setBackgroundColor(Color(0, 0, 0, 220));
    botControlWindow_->setBorderColor(Color(100, 100, 255, 200));
    
    // Add bot control options
    auto titleLabel = std::make_shared<Label>("Bot Control Panel", Rect(10, 30, 230, 20));
    titleLabel->setTextColor(Color(200, 200, 255));
    titleLabel->setFontStyle(FontStyle::Bold);
    botControlWindow_->addElement(titleLabel);
    
    auto toggleBotButton = std::make_shared<Button>("Enable Bot", Rect(10, 60, 230, 30));
    toggleBotButton->setId("toggleBotButton");
    toggleBotButton->setOnClick([this, toggleBotButton]() {
        botEnabled_ = !botEnabled_;
        
        if (botEnabled_) {
            toggleBotButton->setText("Disable Bot");
            toggleBotButton->setBackgroundColor(Color(150, 30, 30, 200));
            toggleBotButton->setHoverColor(Color(200, 40, 40, 200));
            statusLabel_->setText("Status: Active");
            statusLabel_->setTextColor(Color::Green());
        } else {
            toggleBotButton->setText("Enable Bot");
            toggleBotButton->setBackgroundColor(Color(50, 50, 50, 200));
            toggleBotButton->setHoverColor(Color(80, 80, 80, 200));
            statusLabel_->setText("Status: Inactive");
            statusLabel_->setTextColor(Color(200, 200, 0));
        }
    });
    botControlWindow_->addElement(toggleBotButton);
    
    auto pauseButton = std::make_shared<Button>("Pause Actions", Rect(10, 100, 110, 30));
    pauseButton->setId("pauseButton");
    pauseButton->setOnClick([]() {
        // Pause bot actions
    });
    botControlWindow_->addElement(pauseButton);
    
    auto resumeButton = std::make_shared<Button>("Resume", Rect(130, 100, 110, 30));
    resumeButton->setId("resumeButton");
    resumeButton->setOnClick([]() {
        // Resume bot actions
    });
    botControlWindow_->addElement(resumeButton);
    
    auto emergencyStopButton = std::make_shared<Button>("EMERGENCY STOP", Rect(10, 140, 230, 30));
    emergencyStopButton->setId("emergencyStopButton");
    emergencyStopButton->setBackgroundColor(Color(200, 0, 0, 200));
    emergencyStopButton->setHoverColor(Color(255, 0, 0, 200));
    emergencyStopButton->setOnClick([this, toggleBotButton]() {
        // Emergency stop all bot actions
        botEnabled_ = false;
        autoHealing_ = false;
        autoLooting_ = false;
        autoBuffing_ = false;
        autoFollow_ = false;
        
        toggleBotButton->setText("Enable Bot");
        toggleBotButton->setBackgroundColor(Color(50, 50, 50, 200));
        toggleBotButton->setHoverColor(Color(80, 80, 80, 200));
        statusLabel_->setText("Status: EMERGENCY STOPPED");
        statusLabel_->setTextColor(Color::Red());
        
        // Reset all checkboxes
        auto autoHealCheck = std::dynamic_pointer_cast<CheckBox>(configWindow_->getElementById("autoHealCheck"));
        if (autoHealCheck) autoHealCheck->setChecked(false);
        
        auto autoLootCheck = std::dynamic_pointer_cast<CheckBox>(configWindow_->getElementById("autoLootCheck"));
        if (autoLootCheck) autoLootCheck->setChecked(false);
        
        auto autoBuffCheck = std::dynamic_pointer_cast<CheckBox>(configWindow_->getElementById("autoBuffCheck"));
        if (autoBuffCheck) autoBuffCheck->setChecked(false);
    });
    botControlWindow_->addElement(emergencyStopButton);
}

void GameMenu::createDebugMenu() {
    // Create debug menu window
    auto& uiManager = UIManager::getInstance();
    
    debugWindow_ = uiManager.createWindow("Debug Tools", 500, 220, 250, 300);
    debugWindow_->setId("debugMenu");
    debugWindow_->setBackgroundColor(Color(0, 0, 0, 220));
    debugWindow_->setBorderColor(Color(100, 100, 255, 200));
    
    // Add debug options
    auto titleLabel = std::make_shared<Label>("Debug & Diagnostics", Rect(10, 30, 230, 20));
    titleLabel->setTextColor(Color(200, 200, 255));
    titleLabel->setFontStyle(FontStyle::Bold);
    debugWindow_->addElement(titleLabel);
    
    auto dumpMemoryButton = std::make_shared<Button>("Dump Memory", Rect(10, 60, 230, 30));
    dumpMemoryButton->setId("dumpMemoryButton");
    dumpMemoryButton->setOnClick([]() {
        // Dump memory to file
        utils::MemoryScanner::getInstance().dumpMemoryToFile("memory_dump.bin");
    });
    debugWindow_->addElement(dumpMemoryButton);
    
    auto logPacketsCheck = std::make_shared<CheckBox>("Log All Packets", false, Rect(10, 100, 230, 20));
    logPacketsCheck->setId("logPacketsCheck");
    logPacketsCheck->setOnChange([](bool checked) {
        // Toggle packet logging
        utils::ConfigManager::getInstance().setBool("Debug", "LogAllPackets", checked);
    });
    debugWindow_->addElement(logPacketsCheck);
    
    auto verboseLoggingCheck = std::make_shared<CheckBox>("Verbose Logging", false, Rect(10, 130, 230, 20));
    verboseLoggingCheck->setId("verboseLoggingCheck");
    verboseLoggingCheck->setOnChange([](bool checked) {
        // Toggle verbose logging
        utils::ConfigManager::getInstance().setBool("Debug", "VerboseLogging", checked);
    });
    debugWindow_->addElement(verboseLoggingCheck);
    
    auto showUICheck = std::make_shared<CheckBox>("Show UI Elements", true, Rect(10, 160, 230, 20));
    showUICheck->setId("showUICheck");
    showUICheck->setOnChange([](bool checked) {
        // Toggle UI visibility
        utils::ConfigManager::getInstance().setBool("UI", "ShowInGame", checked);
    });
    debugWindow_->addElement(showUICheck);
    
    auto openLogButton = std::make_shared<Button>("Open Log File", Rect(10, 190, 230, 30));
    openLogButton->setId("openLogButton");
    openLogButton->setOnClick([]() {
        // Open log file in default text editor
        system("start notepad.exe logs\\bot.log");
    });
    debugWindow_->addElement(openLogButton);
    
    auto testButton = std::make_shared<Button>("Run Tests", Rect(10, 230, 230, 30));
    testButton->setId("testButton");
    testButton->setOnClick([]() {
        // Run diagnostic tests
    });
    debugWindow_->addElement(testButton);
}

void GameMenu::updateStatusLabels() {
    if (!statusLabel_ || !cpuUsageLabel_ || !memoryUsageLabel_) return;
    
    // Update CPU usage
    float cpu = getCPUUsage();
    std::stringstream cpuSs;
    cpuSs << "CPU: " << std::fixed << std::setprecision(1) << cpu << "%";
    cpuUsageLabel_->setText(cpuSs.str());
    
    // Update memory usage
    float memory = getMemoryUsageMB();
    std::stringstream memorySs;
    memorySs << "Memory: " << std::fixed << std::setprecision(1) << memory << " MB";
    memoryUsageLabel_->setText(memorySs.str());
    
    // Update colors based on values
    if (cpu > 80.0f) {
        cpuUsageLabel_->setTextColor(Color::Red());
    } else if (cpu > 50.0f) {
        cpuUsageLabel_->setTextColor(Color::Yellow());
    } else {
        cpuUsageLabel_->setTextColor(Color::Green());
    }
    
    if (memory > 200.0f) {
        memoryUsageLabel_->setTextColor(Color::Red());
    } else if (memory > 100.0f) {
        memoryUsageLabel_->setTextColor(Color::Yellow());
    } else {
        memoryUsageLabel_->setTextColor(Color::Green());
    }
}

void GameMenu::reloadConfiguration() {
    // Save current settings to configuration file
    auto& config = utils::ConfigManager::getInstance();
    
    config.setBool("Bot", "AutoHealing", autoHealing_);
    config.setBool("Bot", "AutoLooting", autoLooting_);
    config.setBool("Bot", "AutoBuffing", autoBuffing_);
    config.setBool("Bot", "AutoFollow", autoFollow_);
    config.setString("Bot", "TargetName", targetName_);
    
    // Save the configuration
    if (config.save()) {
        statusLabel_->setText("Status: Config Saved");
        statusLabel_->setTextColor(Color::Green());
    } else {
        statusLabel_->setText("Status: Save Failed");
        statusLabel_->setTextColor(Color::Red());
    }
    
    // Reset status after a brief delay
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (botEnabled_) {
            statusLabel_->setText("Status: Active");
            statusLabel_->setTextColor(Color::Green());
        } else {
            statusLabel_->setText("Status: Inactive");
            statusLabel_->setTextColor(Color(200, 200, 0));
        }
    }).detach();
}

void GameMenu::startMemoryScan(int scanType) {
    // Update UI to show scan is starting
    statusLabel_->setText("Status: Scanning Memory...");
    statusLabel_->setTextColor(Color(255, 165, 0)); // Orange
    
    // Start the scan in a separate thread
    std::thread([this, scanType]() {
        try {
            auto& scanner = utils::MemoryScanner::getInstance();
            
            int foundBefore = scanner.getFoundPatternCount();
            
            if (scanType == 0) {
                scanner.quickScan();
            } else if (scanType == 1) {
                scanner.scanAllPatterns();
            } else {
                scanner.deepScan();
            }
            
            int foundAfter = scanner.getFoundPatternCount();
            int newlyFound = foundAfter - foundBefore;
            
            // Update UI when scan completes
            std::stringstream ss;
            ss << "Found " << newlyFound << " new patterns";
            lastActionLabel_->setText(ss.str());
            
            // Save found addresses to configuration
            utils::ConfigManager::getInstance().save();
            
            if (botEnabled_) {
                statusLabel_->setText("Status: Active");
                statusLabel_->setTextColor(Color::Green());
            } else {
                statusLabel_->setText("Status: Inactive");
                statusLabel_->setTextColor(Color(200, 200, 0));
            }
        }
        catch (const std::exception& e) {
            statusLabel_->setText("Status: Scan Error");
            statusLabel_->setTextColor(Color::Red());
            utils::Logging::error("Memory scan error: " + std::string(e.what()));
        }
    }).detach();
}

void GameMenu::toggleBotFeature(const std::string& featureName) {
    // Update last action label
    std::stringstream ss;
    ss << "Toggled " << featureName;
    lastActionLabel_->setText(ss.str());
    
    // You can add specific feature toggling code here
}

} // namespace ui
