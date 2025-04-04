/**
 * @file memory_scanner.h
 * @brief Fast memory scanning and pattern matching utilities
 */
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <mutex>
#include "../../pch.h"

namespace utils {

/**
 * @brief Represents a pattern to search for in memory
 */
struct PatternDefinition {
    std::string name;           // Name of the pattern
    std::string pattern;        // Pattern in byte format (e.g., "48 8B ? ? 48 83 EC")
    std::string module;         // Module name (empty for main executable)
    int32_t offset;             // Offset from the found pattern
    bool relative;              // Whether address is a relative jump/call
    std::string description;    // Description of what this pattern represents
};

/**
 * @brief Class for scanning memory and finding patterns
 * 
 * Provides utilities for scanning process memory to find specific byte patterns,
 * typically used to locate functions or data structures in the game.
 */
class MemoryScanner {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to the singleton instance
     */
    static MemoryScanner& getInstance();
    
    /**
     * @brief Initialize the memory scanner
     * @return True if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Scan for a pattern in memory
     * @param pattern Pattern to search for (e.g., "48 8B ? ? 48 83 EC")
     * @param moduleName Module to scan in (empty for main executable)
     * @param startAddress Start address for scan (optional, module base if not specified)
     * @param endAddress End address for scan (optional, module end if not specified)
     * @return Found address, or nullopt if not found
     */
    std::optional<uintptr_t> findPattern(
        const std::string& pattern,
        const std::string& moduleName = "",
        uintptr_t startAddress = 0,
        uintptr_t endAddress = 0);
    
    /**
     * @brief Scan for a signature in memory
     * @param signature Byte signature to search for
     * @param mask Mask for the signature ("x" for exact match, "?" for wildcard)
     * @param moduleName Module to scan in (empty for main executable)
     * @param startAddress Start address for scan
     * @param endAddress End address for scan
     * @return Found address, or nullopt if not found
     */
    std::optional<uintptr_t> findSignature(
        const std::vector<uint8_t>& signature,
        const std::string& mask,
        const std::string& moduleName = "",
        uintptr_t startAddress = 0,
        uintptr_t endAddress = 0);
    
    /**
     * @brief Scan for multiple patterns at once
     * @param patterns Vector of pattern definitions
     * @return Map of pattern names to found addresses
     */
    std::unordered_map<std::string, uintptr_t> findPatterns(
        const std::vector<PatternDefinition>& patterns);
    
    /**
     * @brief Scan for text strings in memory
     * @param text Text to search for
     * @param caseSensitive Whether the search is case-sensitive
     * @return Vector of addresses where the text was found
     */
    std::vector<uintptr_t> findText(const std::string& text, bool caseSensitive = true);
    
    /**
     * @brief Scan for a value in memory
     * @tparam T Type of value to search for
     * @param value Value to search for
     * @param startAddress Start address for scan
     * @param endAddress End address for scan
     * @return Vector of addresses where the value was found
     */
    template <typename T>
    std::vector<uintptr_t> findValue(
        const T& value,
        uintptr_t startAddress = 0,
        uintptr_t endAddress = 0) {
        std::vector<uintptr_t> results;
        
        // Get process info
        if (!initialize()) {
            return results;
        }
        
        // Set default scan range
        if (startAddress == 0) {
            startAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
        }
        
        if (endAddress == 0) {
            MODULEINFO moduleInfo;
            if (GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(moduleInfo))) {
                endAddress = startAddress + moduleInfo.SizeOfImage;
            } else {
                // Default to a reasonable range if module info fails
                endAddress = startAddress + 0x10000000;
            }
        }
        
        // Get value as bytes
        const uint8_t* valueBytes = reinterpret_cast<const uint8_t*>(&value);
        size_t valueSize = sizeof(T);
        
        // Scan memory
        uintptr_t address = startAddress;
        while (address < endAddress - valueSize) {
            bool found = true;
            // Compare byte by byte
            for (size_t i = 0; i < valueSize; i++) {
                if (*reinterpret_cast<uint8_t*>(address + i) != valueBytes[i]) {
                    found = false;
                    break;
                }
            }
            
            if (found) {
                results.push_back(address);
            }
            
            address++;
        }
        
        return results;
    }
    
    /**
     * @brief Get the module base address
     * @param moduleName Module name (empty for main executable)
     * @return Base address of the module, or 0 if not found
     */
    uintptr_t getModuleBase(const std::string& moduleName = "");
    
    /**
     * @brief Add a pattern definition to the database
     * @param definition Pattern definition to add
     */
    void addPatternDefinition(const PatternDefinition& definition);
    
    /**
     * @brief Scan for all registered patterns
     * @return Number of patterns found
     */
    size_t scanAllPatterns();
    
    /**
     * @brief Get address for a named pattern
     * @param name Name of the pattern
     * @return Address, or 0 if not found
     */
    uintptr_t getAddressForPattern(const std::string& name);
    
    /**
     * @brief Dump memory region to file for analysis
     * @param startAddress Start address of memory region
     * @param size Size of memory region to dump
     * @param filename Name of file to save dump to
     * @return True if dump succeeded
     */
    bool dumpMemoryRegion(uintptr_t startAddress, size_t size, const std::string& filename);
    
    /**
     * @brief Dump all strings found in memory
     * @param minLength Minimum string length to consider
     * @param filename Name of file to save strings to
     * @return Number of strings found
     */
    size_t dumpStrings(size_t minLength, const std::string& filename);
    
    /**
     * @brief Set callback for when patterns are found
     * @param callback Function to call with pattern name and address
     */
    void setPatternFoundCallback(std::function<void(const std::string&, uintptr_t)> callback);
    
    /**
     * @brief Register a pattern for DirectDraw functions
     * Adds common DirectDraw function patterns to the database
     */
    void registerDirectDrawPatterns();
    
    /**
     * @brief Register patterns for TCP/IP functions
     * Adds common TCP/IP function patterns to the database
     */
    void registerNetworkPatterns();
    
    /**
     * @brief Register patterns for game-specific functions
     * Adds Dark Ages specific function patterns to the database
     */
    void registerGamePatterns();

private:
    // Private constructor for singleton pattern
    MemoryScanner() = default;
    ~MemoryScanner() = default;
    
    // Prevent copy/assignment
    MemoryScanner(const MemoryScanner&) = delete;
    MemoryScanner& operator=(const MemoryScanner&) = delete;
    
    // Convert pattern string to byte vector and mask
    std::pair<std::vector<uint8_t>, std::string> patternToBytes(const std::string& pattern);
    
    // Get relative address from instruction
    uintptr_t getRelativeAddress(uintptr_t address, int instructionSize);
    
    // Database of registered patterns
    std::unordered_map<std::string, PatternDefinition> patternDefinitions_;
    
    // Cache of found addresses
    std::unordered_map<std::string, uintptr_t> patternAddresses_;
    
    // Callback for when patterns are found
    std::function<void(const std::string&, uintptr_t)> patternFoundCallback_;
    
    // Thread safety mutex
    mutable std::mutex mutex_;
    
    // Initialization state
    bool initialized_ = false;
};

} // namespace utils
