#pragma once

#include <string>
#include <mutex>
#include <fstream>
#include <memory>

class Logger {
public:
    enum class Format { Plain, JSON };

    // Initialize logger with file path, max file size (bytes), max files, and format
    static void init(const std::string& logFilePath = "",
                     size_t maxFileSize = 10 * 1024 * 1024,
                     int maxFiles = 5,
                     Format format = Format::Plain);

    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    static void debug(const std::string& msg);

    // Set structured logging format (JSON or plain)
    static void setFormat(Format format);

private:
    static void log(const std::string& level, const std::string& msg);
    static void rotateLogsIfNeeded();
    static void openLogFile();
    static void closeLogFile();

    static std::mutex mutex_;
    static std::unique_ptr<std::ofstream> logFile_;
    static std::string logFilePath_;
    static size_t maxFileSize_;
    static int maxFiles_;
    static Format logFormat_;
};