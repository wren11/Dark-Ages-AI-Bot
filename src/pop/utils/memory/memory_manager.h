/**
 * @file memory_manager.h
 * @brief High-level memory management and scanning operations
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <functional>
#include "memory_scanner.h"
#include "../../pch.h"

namespace utils {

/**
 * @brief Structure representing a memory region
 */
struct MemoryRegion {
    uintptr_t start;
    uintptr_t end;
    std::string name;
    DWORD protection;
    std::string module;
    bool isExecutable;
    bool isReadable;
    bool isWritable;
};

/**
 * @brief High-level memory management and introspection
 * 
 * Provides functionality to inspect and modify memory at runtime,
 * monitor memory changes, and automatically discover game structures.
 */
class MemoryManager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static MemoryManager& getInstance();
    
    /**
     * @brief Initialize the memory manager
     * @return True if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Get memory regions in the process
     * @param executableOnly Whether to only include executable regions
     * @return Vector of memory regions
     */
    std::vector<MemoryRegion> getMemoryRegions(bool executableOnly = false);
    
    /**
     * @brief Dump all memory maps to a file
     * @param filename Name of file to save maps to
     * @return True if dump succeeded
     */
    bool dumpMemoryMaps(const std::string& filename);
    
    /**
     * @brief Start monitoring a memory location for changes
     * @param address Memory address to monitor
     * @param size Size of memory region to monitor
     * @param callback Function to call when memory changes
     * @return ID for the monitor
     */
    uint32_t addMemoryMonitor(uintptr_t address, size_t size, 
        std::function<void(uintptr_t, const std::vector<uint8_t>&, const std::vector<uint8_t>&)> callback);
    
    /**
     * @brief Stop monitoring a memory location
     * @param monitorId ID of the monitor to stop
     * @return True if monitor was removed
     */
    bool removeMemoryMonitor(uint32_t monitorId);
    
    /**
     * @brief Find references to a memory address
     * @param targetAddress Address to find references to
     * @param startAddress Start address for scan
     * @param endAddress End address for scan
     * @return Vector of addresses that reference targetAddress
     */
    std::vector<uintptr_t> findReferences(uintptr_t targetAddress,
        uintptr_t startAddress = 0, uintptr_t endAddress = 0);
    
    /**
     * @brief Find all vtables in memory
     * @return Map of vtable addresses to names
     */
    std::unordered_map<uintptr_t, std::string> findVTables();
    
    /**
     * @brief Find all function prologues in a memory region
     * @param startAddress Start address of region
     * @param endAddress End address of region
     * @return Vector of function addresses
     */
    std::vector<uintptr_t> findFunctionPrologues(uintptr_t startAddress, uintptr_t endAddress);
    
    /**
     * @brief Try to find names for functions
     * This attempts to associate function addresses with symbolic names
     * @return Map of function addresses to names
     */
    std::unordered_map<uintptr_t, std::string> identifyFunctions();
    
    /**
     * @brief Hook a function by address
     * @param address Function address to hook
     * @param hookFunction Replacement function
     * @param original Pointer to receive original function pointer
     * @return True if hook succeeded
     */
    template <typename T>
    bool hookFunction(uintptr_t address, T hookFunction, T* original) {
        // Use Detours to hook the function
        try {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            
            *original = reinterpret_cast<T>(address);
            LONG result = DetourAttach(reinterpret_cast<void**>(original), reinterpret_cast<void*>(hookFunction));
            
            if (result != NO_ERROR) {
                DetourTransactionAbort();
                return false;
            }
            
            result = DetourTransactionCommit();
            return (result == NO_ERROR);
        }
        catch (...) {
            DetourTransactionAbort();
            return false;
        }
    }
    
    /**
     * @brief Unhook a function
     * @param hookFunction Hooked function
     * @param original Original function
     * @return True if unhook succeeded
     */
    template <typename T>
    bool unhookFunction(T hookFunction, T original) {
        try {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            
            LONG result = DetourDetach(reinterpret_cast<void**>(&original), reinterpret_cast<void*>(hookFunction));
            
            if (result != NO_ERROR) {
                DetourTransactionAbort();
                return false;
            }
            
            result = DetourTransactionCommit();
            return (result == NO_ERROR);
        }
        catch (...) {
            DetourTransactionAbort();
            return false;
        }
    }
    
    /**
     * @brief Read memory from an address
     * @tparam T Type to read
     * @param address Address to read from
     * @return Value read from memory
     */
    template <typename T>
    T read(uintptr_t address) {
        T value = T();
        ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    }
    
    /**
     * @brief Write memory to an address
     * @tparam T Type to write
     * @param address Address to write to
     * @param value Value to write
     * @return True if write succeeded
     */
    template <typename T>
    bool write(uintptr_t address, const T& value) {
        DWORD oldProtect;
        if (!VirtualProtect(reinterpret_cast<LPVOID>(address), sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }
        
        bool result = WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
        
        VirtualProtect(reinterpret_cast<LPVOID>(address), sizeof(T), oldProtect, &oldProtect);
        return result;
    }
    
    /**
     * @brief Read a string from memory
     * @param address Address to read from
     * @param maxLength Maximum length of string to read
     * @return String read from memory
     */
    std::string readString(uintptr_t address, size_t maxLength = 256);
    
    /**
     * @brief Write a string to memory
     * @param address Address to write to
     * @param value String to write
     * @return True if write succeeded
     */
    bool writeString(uintptr_t address, const std::string& value);
    
    /**
     * @brief Read bytes from memory
     * @param address Address to read from
     * @param size Number of bytes to read
     * @return Vector of bytes read
     */
    std::vector<uint8_t> readBytes(uintptr_t address, size_t size);
    
    /**
     * @brief Write bytes to memory
     * @param address Address to write to
     * @param bytes Bytes to write
     * @return True if write succeeded
     */
    bool writeBytes(uintptr_t address, const std::vector<uint8_t>& bytes);

private:
    // Private constructor for singleton pattern
    MemoryManager() = default;
    ~MemoryManager() = default;
    
    // Prevent copy/assignment
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    
    // Thread safety mutex
    mutable std::mutex mutex_;
    
    // Initialization state
    bool initialized_ = false;
    
    // Memory monitors
    struct MemoryMonitor {
        uintptr_t address;
        size_t size;
        std::vector<uint8_t> lastValue;
        std::function<void(uintptr_t, const std::vector<uint8_t>&, const std::vector<uint8_t>&)> callback;
    };
    
    std::unordered_map<uint32_t, MemoryMonitor> monitors_;
    uint32_t nextMonitorId_ = 1;
    
    // Monitor thread
    std::unique_ptr<std::thread> monitorThread_;
    std::atomic<bool> stopMonitoring_ = false;
    
    // Monitor thread function
    void monitorThreadFunc();
    
    // Helper functions
    bool isValidMemory(uintptr_t address, size_t size);
    std::vector<MemoryRegion> getAllRegions();
};

} // namespace utils
