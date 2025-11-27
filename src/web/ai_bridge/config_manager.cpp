// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "config_manager.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <common/showmsg.hpp>

namespace AIBridge {

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    filepath_ = filepath;
    config_.clear();
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        ShowError("[ConfigManager] Failed to open config file: %s\n", filepath.c_str());
        return false;
    }
    
    std::string current_section;
    std::string line;
    int32 line_num = 0;
    
    while (std::getline(file, line)) {
        line_num++;
        
        // Trim whitespace
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Check for section header [section_name]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            current_section = line.substr(1, line.length() - 2);
            current_section = trim(current_section);
            continue;
        }
        
        // Parse key = value
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            ShowWarning("[ConfigManager] Invalid config line %d: %s\n", line_num, line.c_str());
            continue;
        }
        
        std::string key = trim(line.substr(0, eq_pos));
        std::string value = trim(line.substr(eq_pos + 1));
        
        if (key.empty()) {
            ShowWarning("[ConfigManager] Empty key at line %d\n", line_num);
            continue;
        }
        
        // Store with section prefix
        std::string full_key = make_key(current_section, key);
        config_[full_key] = value;
    }
    
    file.close();
    
    ShowInfo("[ConfigManager] Loaded %zu config entries from %s\n", config_.size(), filepath.c_str());
    return true;
}

bool ConfigManager::reload() {
    if (filepath_.empty()) {
        ShowError("[ConfigManager] No config file loaded yet\n");
        return false;
    }
    
    ShowInfo("[ConfigManager] Reloading configuration from %s\n", filepath_.c_str());
    return load(filepath_);
}

std::string ConfigManager::get_string(const std::string& section, const std::string& key, const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string full_key = make_key(section, key);
    auto it = config_.find(full_key);
    
    if (it != config_.end()) {
        return it->second;
    }
    
    return default_value;
}

int64_t ConfigManager::get_int(const std::string& section, const std::string& key, int64_t default_value) const {
    std::string value = get_string(section, key);
    
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stoll(value);
    } catch (...) {
        ShowWarning("[ConfigManager] Failed to parse int value for %s.%s: %s\n",
                    section.c_str(), key.c_str(), value.c_str());
        return default_value;
    }
}

bool ConfigManager::get_bool(const std::string& section, const std::string& key, bool default_value) const {
    std::string value = get_string(section, key);
    
    if (value.empty()) {
        return default_value;
    }
    
    // Convert to lowercase for comparison
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    
    if (lower_value == "true" || lower_value == "yes" || lower_value == "1" || lower_value == "on") {
        return true;
    }
    
    if (lower_value == "false" || lower_value == "no" || lower_value == "0" || lower_value == "off") {
        return false;
    }
    
    ShowWarning("[ConfigManager] Invalid bool value for %s.%s: %s\n",
                section.c_str(), key.c_str(), value.c_str());
    return default_value;
}

void ConfigManager::set_value(const std::string& section, const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string full_key = make_key(section, key);
    config_[full_key] = value;
}

bool ConfigManager::has_key(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string full_key = make_key(section, key);
    return config_.find(full_key) != config_.end();
}

std::vector<std::string> ConfigManager::get_sections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> sections;
    for (const auto& [key, value] : config_) {
        size_t dot_pos = key.find('.');
        if (dot_pos != std::string::npos) {
            std::string section = key.substr(0, dot_pos);
            if (std::find(sections.begin(), sections.end(), section) == sections.end()) {
                sections.push_back(section);
            }
        }
    }
    
    return sections;
}

std::vector<std::string> ConfigManager::get_keys(const std::string& section) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> keys;
    std::string section_prefix = section + ".";
    
    for (const auto& [key, value] : config_) {
        if (key.find(section_prefix) == 0) {
            keys.push_back(key.substr(section_prefix.length()));
        }
    }
    
    return keys;
}

std::string ConfigManager::make_key(const std::string& section, const std::string& key) const {
    if (section.empty()) {
        return key;
    }
    return section + "." + key;
}

std::string ConfigManager::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

} // namespace AIBridge