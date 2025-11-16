#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

std::mutex Logger::mutex_;

void Logger::init() {
    // Optionally configure log output, file, etc.
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
    std::cout << "[" << std::put_time(&tm, "%F %T") << "] [" << level << "] " << msg << std::endl;
}