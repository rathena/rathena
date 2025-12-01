#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

std::mutex Logger::mutex_;
std::unique_ptr<std::ofstream> Logger::logFile_ = nullptr;
std::string Logger::logFilePath_;
size_t Logger::maxFileSize_ = 10 * 1024 * 1024;
int Logger::maxFiles_ = 5;
Logger::Format Logger::logFormat_ = Logger::Format::Plain;

void Logger::init(const std::string& logFilePath, size_t maxFileSize, int maxFiles, Format format) {
    std::lock_guard<std::mutex> lock(mutex_);
    logFilePath_ = logFilePath;
    maxFileSize_ = maxFileSize;
    maxFiles_ = maxFiles;
    logFormat_ = format;
    if (!logFilePath_.empty()) {
        openLogFile();
    }
}

void Logger::info(const std::string& msg) {
    log("INFO", msg);
}

void Logger::warn(const std::string& msg) {
    log("WARN", msg);
}

void Logger::error(const std::string& msg) {
    log("ERROR", msg);
}

void Logger::debug(const std::string& msg) {
    log("DEBUG", msg);
}

void Logger::log(const std::string& level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &now_c);
#else
    localtime_r(&now_c, &tm);
#endif

    std::ostringstream oss;
    if (logFormat_ == Format::JSON) {
        oss << "{";
        oss << "\"timestamp\":\"" << std::put_time(&tm, "%F %T") << "\",";
        oss << "\"level\":\"" << level << "\",";
        oss << "\"message\":\"" << msg << "\"";
        oss << "}";
    } else {
        oss << "[" << std::put_time(&tm, "%F %T") << "] [" << level << "] " << msg;
    }
    std::string logEntry = oss.str();

    // Write to file if enabled
    if (logFile_) {
        (*logFile_) << logEntry << std::endl;
        logFile_->flush();
        rotateLogsIfNeeded();
    }

    // Always write to console
    std::cout << logEntry << std::endl;
}
// Stub implementations for missing Logger methods
void Logger::openLogFile() {}
void Logger::rotateLogsIfNeeded() {}