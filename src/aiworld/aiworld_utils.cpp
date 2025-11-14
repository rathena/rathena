/**
 * aiworld_utils.cpp
 * Utility functions for rAthena AIWorld Plugin/Module
 */

#include "aiworld_utils.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace aiworld {

std::string generate_correlation_id() {
    // Generate a random 16-character hex string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) {
        oss << std::hex << dis(gen);
    }
    return oss.str();
}

std::string json_to_string(const nlohmann::json& j) {
    return j.dump();
}

nlohmann::json string_to_json(const std::string& s) {
    return nlohmann::json::parse(s);
}

void log_info(const std::string& msg) {
    std::cout << "[AIWORLD][INFO] " << msg << std::endl;
}

void log_warn(const std::string& msg) {
    std::cout << "[AIWORLD][WARN] " << msg << std::endl;
}

void log_error(const std::string& msg) {
    std::cerr << "[AIWORLD][ERROR] " << msg << std::endl;
}

} // namespace aiworld