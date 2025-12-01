#pragma once
// aiworld_utils.hpp
// Utility functions for rAthena AIWorld Plugin/Module

#include <string>
#include <nlohmann/json.hpp>

namespace aiworld {

// Generate a unique correlation ID for IPC messages
std::string generate_correlation_id();

// Serialize a JSON object to string for ZeroMQ transmission
std::string json_to_string(const nlohmann::json& j);

// Deserialize a string to JSON object
nlohmann::json string_to_json(const std::string& s);

/**
 * Logging utility (can be redirected to rAthena's logging system)
 */
void log_info(const std::string& msg);
void log_warn(const std::string& msg);
void log_error(const std::string& msg);

/**
 * Structured performance logging (outputs JSON to stdout)
 */
void log_performance(const nlohmann::json& perf);

} // namespace aiworld