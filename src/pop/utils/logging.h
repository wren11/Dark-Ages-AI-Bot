/**
 * @file logging.h
 * @brief Utility for structured logging
 */
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace utils {

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

/**
 * @brief Thread-safe logging utility
 * 
 * Provides structured logging with severity levels and
 * formatted output. Implements a singleton pattern.
 */
class Logging {
public:
    /**
     * @brief Initialize the logging system
     * 
     * Sets up output streams and default log level.
     * 
     * @param logToFile Whether to log to a file
     * @param logFilePath Path to the log file
     * @return True if initialization succeeded
     */
    static bool initialize(bool logToFile = true, const std::string& logFilePath = "bot.log");
    
    /**
     * @brief Clean up logging resources
     */
    static void cleanup();
    
    /**
     * @brief Set the minimum log level to display
     * @param level Minimum level
     */
    static void setLevel(LogLevel level);
    
    /**
     * @brief Get the current minimum log level
     * @return Current level
     */
    static LogLevel getLevel();
    
    /**
     * @brief Log a debug message
     * @param message Message to log
     */
    static void debug(const std::string& message);
    
    /**
     * @brief Log an info message
     * @param message Message to log
     */
    static void info(const std::string& message);
    
    /**
     * @brief Log a warning message
     * @param message Message to log
     */
    static void warning(const std::string& message);
    
    /**
     * @brief Log an error message
     * @param message Message to log
     */
    static void error(const std::string& message);
    
    /**
     * @brief Log a critical message
     * @param message Message to log
     */
    static void critical(const std::string& message);
    
    /**
     * @brief Log a message with a specific level
     * @param level Log level
     * @param message Message to log
     */
    static void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Convert a byte to a hex string
     * @param byte Byte to convert
     * @return Hex string
     */
    static std::string hexString(std::uint8_t byte);

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Logging();
    
    /**
     * @brief Private destructor
     */
    ~Logging();
    
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static Logging& getInstance();
    
    /**
     * @brief Format a log message with timestamp and level
     * @param level Log level
     * @param message Message to log
     * @return Formatted message
     */
    std::string formatLogMessage(LogLevel level, const std::string& message);
    
    /**
     * @brief Get string representation of a log level
     * @param level Log level
     * @return String representation
     */
    std::string getLevelString(LogLevel level);
    
    /**
     * @brief Get current timestamp as a string
     * @return Timestamp string
     */
    std::string getTimestamp();

    LogLevel level_;           // Minimum level to log
    bool logToFile_;           // Whether to log to a file
    std::ofstream logFile_;    // Log file stream
    std::mutex mutex_;         // Mutex for thread safety
};

} // namespace utils
