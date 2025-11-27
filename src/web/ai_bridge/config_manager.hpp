// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
/**
 * @file config_manager.hpp
 * @brief Configuration manager for Bridge Layer
 * 
 * Features:
 * - INI-style configuration file parsing
 * - Hot reload support (runtime config updates)
 * - Type-safe getters
 * - Default values
 * - Thread-safe operations
 */

#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <map>
#include <mutex>
#include <string>

namespace AIBridge {

/**
 * @brief Configuration manager for Bridge Layer
 * 
 * Manages configuration from conf/ai_bridge.conf
 * Supports hot reload without server restart
 */
class ConfigManager {
public:
    /**
     * @brief Get singleton instance
     * @return ConfigManager reference
     */
    static ConfigManager& instance();
    
    /**
     * @brief Load configuration from file
     * @param filepath Path to configuration file
     * @return true on success
     */
    bool load(const std::string& filepath);
    
    /**
     * @brief Reload configuration from last loaded file
     * @return true on success
     */
    bool reload();
    
    /**
     * @brief Get string value
     * @param section Section name (e.g., "ai_service")
     * @param key Key name (e.g., "base_url")
     * @param default_value Default if not found
     * @return Value or default
     */
    std::string get_string(const std::string& section, const std::string& key, const std::string& default_value = "") const;
    
    /**
     * @brief Get integer value
     * @param section Section name
     * @param key Key name
     * @param default_value Default if not found
     * @return Value or default
     */
    int64_t get_int(const std::string& section, const std::string& key, int64_t default_value = 0) const;
    
    /**
     * @brief Get boolean value
     * @param section Section name
     * @param key Key name
     * @param default_value Default if not found
     * @return Value or default
     */
    bool get_bool(const std::string& section, const std::string& key, bool default_value = false) const;
    
    /**
     * @brief Set value (runtime override)
     * @param section Section name
     * @param key Key name
     * @param value Value to set
     */
    void set_value(const std::string& section, const std::string& key, const std::string& value);
    
    /**
     * @brief Check if key exists
     * @param section Section name
     * @param key Key name
     * @return true if exists
     */
    bool has_key(const std::string& section, const std::string& key) const;
    
    /**
     * @brief Get all sections
     * @return Vector of section names
     */
    std::vector<std::string> get_sections() const;
    
    /**
     * @brief Get all keys in section
     * @param section Section name
     * @return Vector of key names
     */
    std::vector<std::string> get_keys(const std::string& section) const;
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    // Delete copy and move
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = delete;
    ConfigManager& operator=(ConfigManager&&) = delete;
    
    std::string make_key(const std::string& section, const std::string& key) const;
    void parse_line(const std::string& line, std::string& current_section);
    std::string trim(const std::string& str) const;
    
    std::map<std::string, std::string> config_;
    std::string filepath_;
    mutable std::mutex mutex_;
};

} // namespace AIBridge

#endif // CONFIG_MANAGER_HPP