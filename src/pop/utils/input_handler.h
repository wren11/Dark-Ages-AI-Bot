/**
 * @file input_handler.h
 * @brief Utility for handling keyboard input
 */
#pragma once

#include <windows.h>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>

namespace utils {

/**
 * @brief Key codes for commonly used keys
 */
enum class KeyCode {
    F1 = VK_F1,
    F2 = VK_F2,
    F3 = VK_F3,
    F4 = VK_F4,
    F5 = VK_F5,
    F6 = VK_F6,
    F7 = VK_F7,
    F8 = VK_F8,
    F9 = VK_F9,
    F10 = VK_F10,
    F11 = VK_F11,
    F12 = VK_F12
};

/**
 * @brief Handler for keyboard input
 * 
 * Manages keyboard input in a separate thread and triggers
 * callbacks when registered keys are pressed.
 */
class InputHandler {
public:
    using KeyCallback = std::function<void()>;
    
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static InputHandler& getInstance();
    
    /**
     * @brief Start the input handler thread
     * @return True if started successfully
     */
    bool start();
    
    /**
     * @brief Stop the input handler thread
     */
    void stop();
    
    /**
     * @brief Register a callback for a key
     * @param key Key to monitor
     * @param callback Function to call when key is pressed
     */
    void registerKeyCallback(KeyCode key, KeyCallback callback);
    
    /**
     * @brief Unregister a callback for a key
     * @param key Key to stop monitoring
     * @return True if callback was found and removed
     */
    bool unregisterKeyCallback(KeyCode key);
    
    /**
     * @brief Check if a key is currently pressed
     * @param key Key to check
     * @return True if key is currently pressed
     */
    bool isKeyPressed(KeyCode key) const;
    
    /**
     * @brief Prevent copying
     */
    InputHandler(const InputHandler&) = delete;
    
    /**
     * @brief Prevent assignment
     */
    InputHandler& operator=(const InputHandler&) = delete;

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    InputHandler();
    
    /**
     * @brief Private destructor
     */
    ~InputHandler();
    
    /**
     * @brief Input monitoring thread function
     */
    void inputThreadFunction();
    
    /**
     * @brief Check for key presses and trigger callbacks
     */
    void checkKeys();

    std::unordered_map<int, KeyCallback> keyCallbacks_;  // Map of key codes to callbacks
    std::thread inputThread_;                           // Thread for monitoring input
    std::atomic<bool> running_;                         // Whether the thread should continue running
    std::mutex mutex_;                                  // Mutex for thread safety
    std::atomic<DWORD> lastPressTime_;                  // Time of last key press to prevent repeats
};

} // namespace utils
