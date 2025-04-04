#include "pch.h"

/**
 * @file memory_scanner.cpp
 * @brief Implementation of memory scanning and pattern matching
 */
#include "memory_scanner.h"
#include "../logging.h"
#include "../config/config_manager.h"
#include <Windows.h>
#include <Psapi.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace utils {

MemoryScanner& MemoryScanner::getInstance() {
    static MemoryScanner instance;
    return instance;
}

bool MemoryScanner::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }
    
    try {
        // Register known patterns
        registerGamePatterns();
        registerNetworkPatterns();
        
        // Get configuration for DirectDraw
        if (ConfigManager::getInstance().getBool("Intercept", "TraceDirectDraw", false)) {
            registerDirectDrawPatterns();
        }
        
        // Check if we should scan on startup
        if (ConfigManager::getInstance().getBool("MemoryScan", "ScanOnStartup", true)) {
            size_t found = scanAllPatterns();
            Logging::info("Found " + std::to_string(found) + " patterns in memory scan");
        }
        
        initialized_ = true;
        return true;
    }
    catch (const std::exception& e) {
        Logging::error("Failed to initialize memory scanner: " + std::string(e.what()));
        return false;
    }
}

std::optional<uintptr_t> MemoryScanner::findPattern(
    const std::string& pattern,
    const std::string& moduleName,
    uintptr_t startAddress,
    uintptr_t endAddress) {
    
    // Convert pattern to bytes and mask
    auto [bytes, mask] = patternToBytes(pattern);
    
    // Find the signature
    return findSignature(bytes, mask, moduleName, startAddress, endAddress);
}

std::optional<uintptr_t> MemoryScanner::findSignature(
    const std::vector<uint8_t>& signature,
    const std::string& mask,
    const std::string& moduleName,
    uintptr_t startAddress,
    uintptr_t endAddress) {
    
    // Get process and module handles
    if (startAddress == 0) {
        HMODULE moduleHandle = GetModuleHandle(moduleName.empty() ? NULL : moduleName.c_str());
        if (!moduleHandle) {
            Logging::error("Failed to get module handle for: " + (moduleName.empty() ? "main executable" : moduleName));
            return std::nullopt;
        }
        
        startAddress = reinterpret_cast<uintptr_t>(moduleHandle);
    }
    
    if (endAddress == 0) {
        MODULEINFO moduleInfo;
        HMODULE moduleHandle = GetModuleHandle(moduleName.empty() ? NULL : moduleName.c_str());
        
        if (GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo))) {
            endAddress = startAddress + moduleInfo.SizeOfImage;
        }
        else {
            Logging::error("Failed to get module information");
            return std::nullopt;
        }
    }
    
    // Validate addresses
    if (endAddress <= startAddress || endAddress - startAddress > 0x10000000) {
        Logging::error("Invalid address range for pattern scan");
        return std::nullopt;
    }
    
    // Perform the scan
    const size_t signatureSize = signature.size();
    for (uintptr_t address = startAddress; address < endAddress - signatureSize; address++) {
        bool found = true;
        
        for (size_t i = 0; i < signatureSize; i++) {
            if (mask[i] == 'x' && *reinterpret_cast<uint8_t*>(address + i) != signature[i]) {
                found = false;
                break;
            }
        }
        
        if (found) {
            return address;
        }
    }
    
    return std::nullopt;
}

std::unordered_map<std::string, uintptr_t> MemoryScanner::findPatterns(
    const std::vector<PatternDefinition>& patterns) {
    
    std::unordered_map<std::string, uintptr_t> results;
    
    for (const auto& def : patterns) {
        auto address = findPattern(def.pattern, def.module);
        if (address) {
            uintptr_t finalAddress = *address + def.offset;
            
            // If it's a relative address, resolve it
            if (def.relative) {
                finalAddress = getRelativeAddress(finalAddress, 4);
            }
            
            results[def.name] = finalAddress;
            
            // Call the callback if set
            if (patternFoundCallback_) {
                patternFoundCallback_(def.name, finalAddress);
            }
            
            Logging::debug("Found pattern " + def.name + " at 0x" + 
                           std::stringstream(std::stringstream() << std::hex << finalAddress).str());
        }
        else {
            Logging::warning("Failed to find pattern: " + def.name);
            results[def.name] = 0;
        }
    }
    
    return results;
}

std::vector<uintptr_t> MemoryScanner::findText(const std::string& text, bool caseSensitive) {
    std::vector<uintptr_t> results;
    
    // Get process info
    HMODULE moduleHandle = GetModuleHandle(NULL);
    MODULEINFO moduleInfo;
    
    if (!GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo))) {
        Logging::error("Failed to get module information for text scan");
        return results;
    }
    
    uintptr_t startAddress = reinterpret_cast<uintptr_t>(moduleHandle);
    uintptr_t endAddress = startAddress + moduleInfo.SizeOfImage;
    
    // Scan for text
    const size_t textLength = text.length();
    for (uintptr_t address = startAddress; address < endAddress - textLength; address++) {
        bool found = true;
        
        for (size_t i = 0; i < textLength; i++) {
            char memChar = *reinterpret_cast<char*>(address + i);
            char textChar = text[i];
            
            if (!caseSensitive) {
                memChar = std::tolower(memChar);
                textChar = std::tolower(textChar);
            }
            
            if (memChar != textChar) {
                found = false;
                break;
            }
        }
        
        if (found) {
            results.push_back(address);
        }
    }
    
    return results;
}

uintptr_t MemoryScanner::getModuleBase(const std::string& moduleName) {
    HMODULE moduleHandle = GetModuleHandle(moduleName.empty() ? NULL : moduleName.c_str());
    return reinterpret_cast<uintptr_t>(moduleHandle);
}

void MemoryScanner::addPatternDefinition(const PatternDefinition& definition) {
    std::lock_guard<std::mutex> lock(mutex_);
    patternDefinitions_[definition.name] = definition;
}

size_t MemoryScanner::scanAllPatterns() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t foundCount = 0;
    
    // Convert to vector for findPatterns
    std::vector<PatternDefinition> patterns;
    for (const auto& [name, def] : patternDefinitions_) {
        patterns.push_back(def);
    }
    
    // Find all patterns
    auto results = findPatterns(patterns);
    
    // Store results
    for (const auto& [name, address] : results) {
        if (address != 0) {
            patternAddresses_[name] = address;
            foundCount++;
            
            // Also update the config
            ConfigManager::getInstance().setAddress("GameAddresses", name, address);
        }
    }
    
    return foundCount;
}

uintptr_t MemoryScanner::getAddressForPattern(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if we have it cached
    if (patternAddresses_.count(name) > 0) {
        return patternAddresses_[name];
    }
    
    // Not cached, check if we have a definition for it
    if (patternDefinitions_.count(name) > 0) {
        // Try to find it
        auto def = patternDefinitions_[name];
        auto address = findPattern(def.pattern, def.module);
        
        if (address) {
            uintptr_t finalAddress = *address + def.offset;
            
            // If it's a relative address, resolve it
            if (def.relative) {
                finalAddress = getRelativeAddress(finalAddress, 4);
            }
            
            // Cache and return
            patternAddresses_[name] = finalAddress;
            return finalAddress;
        }
    }
    
    // Not found, check the config
    uintptr_t configAddress = ConfigManager::getInstance().getAddress("GameAddresses", name, 0);
    if (configAddress != 0) {
        patternAddresses_[name] = configAddress;
        return configAddress;
    }
    
    // Really not found
    return 0;
}

bool MemoryScanner::dumpMemoryRegion(uintptr_t startAddress, size_t size, const std::string& filename) {
    try {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            Logging::error("Failed to open file for memory dump: " + filename);
            return false;
        }
        
        // Read memory
        std::vector<uint8_t> buffer(size);
        SIZE_T bytesRead;
        
        if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(startAddress),
                              buffer.data(), size, &bytesRead)) {
            Logging::error("Failed to read memory for dump");
            return false;
        }
        
        // Write to file
        file.write(reinterpret_cast<const char*>(buffer.data()), bytesRead);
        
        Logging::info("Dumped " + std::to_string(bytesRead) + " bytes to " + filename);
        return true;
    }
    catch (const std::exception& e) {
        Logging::error("Error dumping memory: " + std::string(e.what()));
        return false;
    }
}

size_t MemoryScanner::dumpStrings(size_t minLength, const std::string& filename) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            Logging::error("Failed to open file for string dump: " + filename);
            return 0;
        }
        
        // Get process info
        HMODULE moduleHandle = GetModuleHandle(NULL);
        MODULEINFO moduleInfo;
        
        if (!GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo))) {
            Logging::error("Failed to get module information for string dump");
            return 0;
        }
        
        uintptr_t startAddress = reinterpret_cast<uintptr_t>(moduleHandle);
        uintptr_t endAddress = startAddress + moduleInfo.SizeOfImage;
        
        size_t stringCount = 0;
        std::string currentString;
        
        for (uintptr_t address = startAddress; address < endAddress; address++) {
            char c = *reinterpret_cast<char*>(address);
            
            // If it's a printable character, add to current string
            if (std::isprint(c)) {
                currentString += c;
            }
            // If it's a null terminator and we have a string of sufficient length
            else if (c == '\0' && currentString.length() >= minLength) {
                file << std::hex << std::setw(8) << std::setfill('0') << address - currentString.length()
                     << ": " << currentString << std::endl;
                stringCount++;
                currentString.clear();
            }
            // Reset current string
            else {
                currentString.clear();
            }
        }
        
        Logging::info("Dumped " + std::to_string(stringCount) + " strings to " + filename);
        return stringCount;
    }
    catch (const std::exception& e) {
        Logging::error("Error dumping strings: " + std::string(e.what()));
        return 0;
    }
}

void MemoryScanner::setPatternFoundCallback(std::function<void(const std::string&, uintptr_t)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    patternFoundCallback_ = callback;
}

void MemoryScanner::registerDirectDrawPatterns() {
    // DirectDraw functions
    addPatternDefinition({
        "DirectDrawCreate",
        "8B 44 24 0C 8B 4C 24 08 8B 54 24 04 56",
        "",
        0,
        false,
        "DirectDrawCreate function"
    });
    
    addPatternDefinition({
        "DirectDrawCreateEx",
        "8B 44 24 10 8B 4C 24 0C 8B 54 24 08 8B 5C 24 04",
        "",
        0,
        false,
        "DirectDrawCreateEx function"
    });
    
    addPatternDefinition({
        "DirectDrawCreateClipper",
        "8B 44 24 08 8B 4C 24 04 8B 54 24 0C 56",
        "",
        0,
        false,
        "DirectDrawCreateClipper function"
    });
    
    // Blit operations
    addPatternDefinition({
        "Blt",
        "55 8B EC 83 EC 34 53 56 57 8B F1 8B 86",
        "",
        0,
        false,
        "DirectDraw surface Blt function"
    });
    
    addPatternDefinition({
        "BltFast",
        "55 8B EC 83 EC 24 53 56 57 8B F1 8B 46",
        "",
        0,
        false,
        "DirectDraw surface BltFast function"
    });
    
    Logging::info("Registered DirectDraw patterns");
}

void MemoryScanner::registerNetworkPatterns() {
    // TCP/IP socket functions
    addPatternDefinition({
        "WSASend",
        "55 8B EC 83 EC 10 53 56 57 8B 7D 08 8B F1",
        "",
        0,
        false,
        "WSASend socket function"
    });
    
    addPatternDefinition({
        "WSARecv",
        "55 8B EC 83 EC 10 53 56 57 8B 7D 08 8B D9",
        "",
        0,
        false,
        "WSARecv socket function"
    });
    
    addPatternDefinition({
        "send",
        "55 8B EC 83 EC 08 56 8B 75 08 57 8B 7D 0C",
        "",
        0,
        false,
        "send socket function"
    });
    
    addPatternDefinition({
        "recv",
        "55 8B EC 83 EC 08 56 8B 75 08 57 8B 7D 0C",
        "",
        0,
        false,
        "recv socket function"
    });
    
    // Dark Ages specific network functions
    addPatternDefinition({
        "SendPacketFunction",
        "55 8B EC 83 EC ? 53 56 8B 75 ? 8B 46 ? 57",
        "",
        0,
        false,
        "Dark Ages send packet function"
    });
    
    addPatternDefinition({
        "RecvPacketFunction",
        "55 8B EC 83 EC ? 53 56 57 8B 7D ? 8B 47",
        "",
        0,
        false,
        "Dark Ages receive packet function"
    });
    
    Logging::info("Registered network patterns");
}

void MemoryScanner::registerGamePatterns() {
    // Game structures
    addPatternDefinition({
        "PlayerInfoStruct",
        "? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 53 56 57 8B F9 33 DB",
        "",
        5,
        true,
        "Dark Ages player information structure"
    });
    
    addPatternDefinition({
        "GameStateStruct",
        "A1 ? ? ? ? 85 C0 74 ? 8B 48 ? 85 C9 74 ? 8B 11",
        "",
        1,
        false,
        "Dark Ages game state structure"
    });
    
    addPatternDefinition({
        "SpriteManagerStruct",
        "8B 0D ? ? ? ? 85 C9 74 ? 8B 01 8B 40 ? FF D0",
        "",
        2,
        false,
        "Dark Ages sprite manager"
    });
    
    // Game functions
    addPatternDefinition({
        "SendSpellFunction",
        "55 8B EC 83 EC ? 56 8B 75 ? 8D 4D ? E8",
        "",
        0,
        false,
        "Dark Ages function to cast a spell"
    });
    
    addPatternDefinition({
        "EquipItemFunction",
        "55 8B EC 83 EC ? 53 56 57 8B 7D ? 8B 5D",
        "",
        0,
        false,
        "Dark Ages function to equip an item"
    });
    
    addPatternDefinition({
        "DropItemFunction",
        "55 8B EC 51 56 57 8B 7D ? 8B 47 ? 8B 77",
        "",
        0,
        false,
        "Dark Ages function to drop an item"
    });
    
    Logging::info("Registered game patterns");
}

std::pair<std::vector<uint8_t>, std::string> MemoryScanner::patternToBytes(const std::string& pattern) {
    std::vector<uint8_t> bytes;
    std::string mask;
    
    std::stringstream ss(pattern);
    std::string byteStr;
    
    while (ss >> byteStr) {
        if (byteStr == "?" || byteStr == "??") {
            bytes.push_back(0);
            mask += "?";
        }
        else {
            // Convert hex string to byte
            bytes.push_back(static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16)));
            mask += "x";
        }
    }
    
    return {bytes, mask};
}

uintptr_t MemoryScanner::getRelativeAddress(uintptr_t address, int instructionSize) {
    // Address is a 4-byte relative address stored at location
    int32_t offset = *reinterpret_cast<int32_t*>(address);
    return address + offset + instructionSize;
}

} // namespace utils
