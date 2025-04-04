/**
 * @file input_handler.cpp
 * @brief Implementation of the InputHandler class
 */
#include "pch.h"
#include "input_handler.h"
#include "logging.h"

namespace utils {

// Minimum delay between key presses to prevent repeats (in milliseconds)
constexpr DWORD KEY_REPEAT_DELAY = 500;

InputHandler& InputHandler::getInstance() {
    static InputHandler instance;
    return instance;
}

InputHandler::InputHandler()
    : running_(false), lastPressTime_(0) {
}

InputHandler::~InputHandler() {
    stop();
}

bool InputHandler::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (running_) {
        Logging::warning("Input handler already running");
        return true;
    }
    
    try {
        // Start the input thread
        running_ = true;
        inputThread_ = std::thread(&InputHandler::inputThreadFunction, this);
        
        Logging::info("Input handler started");
        return true;
    }
    catch (const std::exception& e) {
        Logging::error("Failed to start input handler: " + std::string(e.what()));
        running_ = false;
        return false;
    }
}

void InputHandler::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!running_) {
        return;
    }
    
    // Signal thread to stop
    running_ = false;
    
    // Wait for thread to finish
    if (inputThread_.joinable()) {
        inputThread_.join();
    }
    
    Logging::info("Input handler stopped");
}

void InputHandler::registerKeyCallback(KeyCode key, KeyCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    keyCallbacks_[static_cast<int>(key)] = callback;
    Logging::debug("Registered callback for key code " + std::to_string(static_cast<int>(key)));
}

bool InputHandler::unregisterKeyCallback(KeyCode key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = keyCallbacks_.find(static_cast<int>(key));
    if (it != keyCallbacks_.end()) {
        keyCallbacks_.erase(it);
        Logging::debug("Unregistered callback for key code " + std::to_string(static_cast<int>(key)));
        return true;
    }
    
    return false;
}

bool InputHandler::isKeyPressed(KeyCode key) const {
    return (GetAsyncKeyState(static_cast<int>(key)) & 0x8000) != 0;
}

void InputHandler::inputThreadFunction() {
    Logging::debug("Input thread started");
    
    while (running_) {
        // Check for key presses
        checkKeys();
        
        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    Logging::debug("Input thread stopped");
}

void InputHandler::checkKeys() {
    // Get current time
    DWORD currentTime = GetTickCount();
    
    // Check if enough time has passed since last key press
    if (currentTime - lastPressTime_ < KEY_REPEAT_DELAY) {
        return;
    }
    
    // Make a copy of the callbacks map to avoid holding the lock during callbacks
    std::unordered_map<int, KeyCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks = keyCallbacks_;
    }
    
    // Check each registered key
    for (const auto& pair : callbacks) {
        int keyCode = pair.first;
        
        // Check if key is pressed
        if (GetAsyncKeyState(keyCode) & 0x8000) {
            // Update last press time
            lastPressTime_ = currentTime;
            
            try {
                // Call the callback
                pair.second();
            }
            catch (const std::exception& e) {
                Logging::error("Exception in key callback: " + std::string(e.what()));
            }
            catch (...) {
                Logging::error("Unknown exception in key callback");
            }
            
            // Only handle one key at a time to avoid conflicts
            break;
        }
    }
}

} // namespace utils
