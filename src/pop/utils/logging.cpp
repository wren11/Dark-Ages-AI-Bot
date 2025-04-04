#include "pch.h"

/**
 * @file logging.cpp
 * @brief Implementation of the Logging utility class
 */
#include "logging.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>

namespace utils {

// Singleton instance
Logging* s_loggingInstance = nullptr;
std::mutex s_loggingMutex;

// Color codes for console output
constexpr WORD COLOR_DEBUG = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
constexpr WORD COLOR_INFO = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
constexpr WORD COLOR_WARNING = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
constexpr WORD COLOR_ERROR = FOREGROUND_RED | FOREGROUND_INTENSITY;
constexpr WORD COLOR_CRITICAL = FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;

Logging::Logging()
    : level_(LogLevel::Info), logToFile_(false) {
}

Logging::~Logging() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

Logging& Logging::getInstance() {
    std::lock_guard<std::mutex> lock(s_loggingMutex);
    
    if (s_loggingInstance == nullptr) {
        s_loggingInstance = new Logging();
    }
    
    return *s_loggingInstance;
}

bool Logging::initialize(bool logToFile, const std::string& logFilePath) {
    Logging& instance = getInstance();
    std::lock_guard<std::mutex> lock(instance.mutex_);
    
    instance.logToFile_ = logToFile;
    
    // Open log file if needed
    if (logToFile) {
        instance.logFile_.open(logFilePath, std::ios::out | std::ios::app);
        
        if (!instance.logFile_.is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            instance.logToFile_ = false;
            return false;
        }
    }
    
    // Log initialization
    instance.log(LogLevel::Info, "Logging system initialized");
    
    return true;
}

void Logging::cleanup() {
    std::lock_guard<std::mutex> lock(s_loggingMutex);
    
    if (s_loggingInstance != nullptr) {
        // Log cleanup
        s_loggingInstance->log(LogLevel::Info, "Logging system shutting down");
        
        // Close file if open
        if (s_loggingInstance->logFile_.is_open()) {
            s_loggingInstance->logFile_.close();
        }
        
        // Delete instance
        delete s_loggingInstance;
        s_loggingInstance = nullptr;
    }
}

void Logging::setLevel(LogLevel level) {
    Logging& instance = getInstance();
    std::lock_guard<std::mutex> lock(instance.mutex_);
    
    instance.level_ = level;
    
    // Log level change
    instance.log(LogLevel::Info, "Log level set to " + instance.getLevelString(level));
}

LogLevel Logging::getLevel() {
    Logging& instance = getInstance();
    std::lock_guard<std::mutex> lock(instance.mutex_);
    
    return instance.level_;
}

void Logging::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logging::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logging::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logging::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logging::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

void Logging::log(LogLevel level, const std::string& message) {
    Logging& instance = getInstance();
    std::lock_guard<std::mutex> lock(instance.mutex_);
    
    // Check if this level should be logged
    if (level < instance.level_) {
        return;
    }
    
    // Format the message
    std::string formattedMessage = instance.formatLogMessage(level, message);
    
    // Get console handle for coloring
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Set color based on log level
    WORD color;
    switch (level) {
        case LogLevel::Debug:
            color = COLOR_DEBUG;
            break;
        case LogLevel::Info:
            color = COLOR_INFO;
            break;
        case LogLevel::Warning:
            color = COLOR_WARNING;
            break;
        case LogLevel::Error:
            color = COLOR_ERROR;
            break;
        case LogLevel::Critical:
            color = COLOR_CRITICAL;
            break;
        default:
            color = COLOR_INFO;
            break;
    }
    
    // Set console text color
    SetConsoleTextAttribute(consoleHandle, color);
    
    // Output to console
    std::cout << formattedMessage << std::endl;
    
    // Reset console color
    SetConsoleTextAttribute(consoleHandle, COLOR_INFO);
    
    // Output to file if enabled
    if (instance.logToFile_ && instance.logFile_.is_open()) {
        instance.logFile_ << formattedMessage << std::endl;
        instance.logFile_.flush();
    }
}

std::string Logging::hexString(std::uint8_t byte) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return oss.str();
}

std::string Logging::formatLogMessage(LogLevel level, const std::string& message) {
    // Get timestamp
    std::string timestamp = getTimestamp();
    
    // Get level string
    std::string levelStr = getLevelString(level);
    
    // Format message
    std::ostringstream oss;
    oss << "[" << timestamp << "] [" << levelStr << "] " << message;
    
    return oss.str();
}

std::string Logging::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

std::string Logging::getTimestamp() {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    
    // Format time
    std::tm timeInfo;
    localtime_s(&timeInfo, &timeT);
    
    // Get milliseconds
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch() % std::chrono::seconds(1));
    
    // Format timestamp
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") << "." 
        << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

} // namespace utils
