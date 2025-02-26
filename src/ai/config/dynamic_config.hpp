/**
 * @file dynamic_config.hpp
 * @brief Dynamic configuration system for AI agents
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the dynamic configuration system for AI agents.
 */

#ifndef DYNAMIC_CONFIG_HPP
#define DYNAMIC_CONFIG_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

/**
 * @brief Configuration value types
 */
enum class ConfigValueType {
    BOOLEAN,    ///< Boolean value
    INTEGER,    ///< Integer value
    FLOAT,      ///< Float value
    STRING,     ///< String value
    ARRAY,      ///< Array value
    OBJECT      ///< Object value
};

/**
 * @brief Configuration value
 * 
 * This class represents a configuration value that can be of different types.
 */
class ConfigValue {
public:
    /**
     * @brief Constructor for boolean value
     * @param value Boolean value
     */
    ConfigValue(bool value);

    /**
     * @brief Constructor for integer value
     * @param value Integer value
     */
    ConfigValue(int value);

    /**
     * @brief Constructor for float value
     * @param value Float value
     */
    ConfigValue(float value);

    /**
     * @brief Constructor for string value
     * @param value String value
     */
    ConfigValue(const std::string& value);

    /**
     * @brief Constructor for array value
     * @param value Array value
     */
    ConfigValue(const std::vector<ConfigValue>& value);

    /**
     * @brief Constructor for object value
     * @param value Object value
     */
    ConfigValue(const std::unordered_map<std::string, ConfigValue>& value);

    /**
     * @brief Get value type
     * @return Value type
     */
    ConfigValueType getType() const;

    /**
     * @brief Get boolean value
     * @return Boolean value
     */
    bool getBool() const;

    /**
     * @brief Get integer value
     * @return Integer value
     */
    int getInt() const;

    /**
     * @brief Get float value
     * @return Float value
     */
    float getFloat() const;

    /**
     * @brief Get string value
     * @return String value
     */
    std::string getString() const;

    /**
     * @brief Get array value
     * @return Array value
     */
    std::vector<ConfigValue> getArray() const;

    /**
     * @brief Get object value
     * @return Object value
     */
    std::unordered_map<std::string, ConfigValue> getObject() const;

    /**
     * @brief Set boolean value
     * @param value Boolean value
     */
    void setBool(bool value);

    /**
     * @brief Set integer value
     * @param value Integer value
     */
    void setInt(int value);

    /**
     * @brief Set float value
     * @param value Float value
     */
    void setFloat(float value);

    /**
     * @brief Set string value
     * @param value String value
     */
    void setString(const std::string& value);

    /**
     * @brief Set array value
     * @param value Array value
     */
    void setArray(const std::vector<ConfigValue>& value);

    /**
     * @brief Set object value
     * @param value Object value
     */
    void setObject(const std::unordered_map<std::string, ConfigValue>& value);

    /**
     * @brief Convert to JSON string
     * @return JSON string
     */
    std::string toJson() const;

    /**
     * @brief Parse from JSON string
     * @param json JSON string
     * @return Parsed value
     */
    static ConfigValue fromJson(const std::string& json);

private:
    ConfigValueType m_type;                                  ///< Value type
    union {
        bool m_boolValue;                                    ///< Boolean value
        int m_intValue;                                      ///< Integer value
        float m_floatValue;                                  ///< Float value
    };
    std::string m_stringValue;                               ///< String value
    std::vector<ConfigValue> m_arrayValue;                   ///< Array value
    std::unordered_map<std::string, ConfigValue> m_objectValue; ///< Object value
};

/**
 * @brief Configuration change callback
 * 
 * This type represents a callback function that is called when a configuration value changes.
 */
using ConfigChangeCallback = std::function<void(const std::string&, const ConfigValue&)>;

/**
 * @brief Dynamic configuration
 * 
 * This class represents a dynamic configuration that can be loaded and modified at runtime.
 */
class DynamicConfig {
public:
    /**
     * @brief Constructor
     * @param name Configuration name
     */
    DynamicConfig(const std::string& name);

    /**
     * @brief Destructor
     */
    virtual ~DynamicConfig();

    /**
     * @brief Load configuration from file
     * @param filename File name
     * @return True if loading was successful, false otherwise
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief Save configuration to file
     * @param filename File name
     * @return True if saving was successful, false otherwise
     */
    bool saveToFile(const std::string& filename);

    /**
     * @brief Load configuration from JSON string
     * @param json JSON string
     * @return True if loading was successful, false otherwise
     */
    bool loadFromJson(const std::string& json);

    /**
     * @brief Save configuration to JSON string
     * @return JSON string
     */
    std::string saveToJson() const;

    /**
     * @brief Get configuration name
     * @return Configuration name
     */
    std::string getName() const;

    /**
     * @brief Get boolean value
     * @param key Configuration key
     * @param defaultValue Default value
     * @return Boolean value
     */
    bool getBool(const std::string& key, bool defaultValue = false) const;

    /**
     * @brief Get integer value
     * @param key Configuration key
     * @param defaultValue Default value
     * @return Integer value
     */
    int getInt(const std::string& key, int defaultValue = 0) const;

    /**
     * @brief Get float value
     * @param key Configuration key
     * @param defaultValue Default value
     * @return Float value
     */
    float getFloat(const std::string& key, float defaultValue = 0.0f) const;

    /**
     * @brief Get string value
     * @param key Configuration key
     * @param defaultValue Default value
     * @return String value
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;

    /**
     * @brief Get array value
     * @param key Configuration key
     * @param defaultValue Default value
     * @return Array value
     */
    std::vector<ConfigValue> getArray(const std::string& key, const std::vector<ConfigValue>& defaultValue = {}) const;

    /**
     * @brief Get object value
     * @param key Configuration key
     * @param defaultValue Default value
     * @return Object value
     */
    std::unordered_map<std::string, ConfigValue> getObject(const std::string& key, const std::unordered_map<std::string, ConfigValue>& defaultValue = {}) const;

    /**
     * @brief Set boolean value
     * @param key Configuration key
     * @param value Boolean value
     */
    void setBool(const std::string& key, bool value);

    /**
     * @brief Set integer value
     * @param key Configuration key
     * @param value Integer value
     */
    void setInt(const std::string& key, int value);

    /**
     * @brief Set float value
     * @param key Configuration key
     * @param value Float value
     */
    void setFloat(const std::string& key, float value);

    /**
     * @brief Set string value
     * @param key Configuration key
     * @param value String value
     */
    void setString(const std::string& key, const std::string& value);

    /**
     * @brief Set array value
     * @param key Configuration key
     * @param value Array value
     */
    void setArray(const std::string& key, const std::vector<ConfigValue>& value);

    /**
     * @brief Set object value
     * @param key Configuration key
     * @param value Object value
     */
    void setObject(const std::string& key, const std::unordered_map<std::string, ConfigValue>& value);

    /**
     * @brief Check if key exists
     * @param key Configuration key
     * @return True if key exists, false otherwise
     */
    bool hasKey(const std::string& key) const;

    /**
     * @brief Remove key
     * @param key Configuration key
     * @return True if key was removed, false otherwise
     */
    bool removeKey(const std::string& key);

    /**
     * @brief Clear all keys
     */
    void clear();

    /**
     * @brief Get all keys
     * @return All keys
     */
    std::vector<std::string> getKeys() const;

    /**
     * @brief Register change callback
     * @param key Configuration key
     * @param callback Callback function
     * @return Callback ID
     */
    int registerChangeCallback(const std::string& key, ConfigChangeCallback callback);

    /**
     * @brief Unregister change callback
     * @param callbackId Callback ID
     * @return True if callback was unregistered, false otherwise
     */
    bool unregisterChangeCallback(int callbackId);

private:
    /**
     * @brief Notify change callbacks
     * @param key Configuration key
     * @param value New value
     */
    void notifyChangeCallbacks(const std::string& key, const ConfigValue& value);

    std::string m_name;                                      ///< Configuration name
    std::unordered_map<std::string, ConfigValue> m_values;   ///< Configuration values
    std::unordered_map<int, std::pair<std::string, ConfigChangeCallback>> m_callbacks; ///< Change callbacks
    int m_nextCallbackId;                                    ///< Next callback ID
    mutable std::mutex m_mutex;                              ///< Mutex for thread safety
};

/**
 * @brief Dynamic configuration manager
 * 
 * This class manages dynamic configurations for different components.
 */
class DynamicConfigManager {
public:
    /**
     * @brief Get instance
     * @return Instance
     */
    static DynamicConfigManager& getInstance();

    /**
     * @brief Destructor
     */
    ~DynamicConfigManager();

    /**
     * @brief Initialize the manager
     * @return True if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Finalize the manager
     */
    void finalize();

    /**
     * @brief Get configuration
     * @param name Configuration name
     * @return Configuration
     */
    std::shared_ptr<DynamicConfig> getConfig(const std::string& name);

    /**
     * @brief Create configuration
     * @param name Configuration name
     * @return Configuration
     */
    std::shared_ptr<DynamicConfig> createConfig(const std::string& name);

    /**
     * @brief Remove configuration
     * @param name Configuration name
     * @return True if configuration was removed, false otherwise
     */
    bool removeConfig(const std::string& name);

    /**
     * @brief Check if configuration exists
     * @param name Configuration name
     * @return True if configuration exists, false otherwise
     */
    bool hasConfig(const std::string& name) const;

    /**
     * @brief Get all configuration names
     * @return All configuration names
     */
    std::vector<std::string> getConfigNames() const;

    /**
     * @brief Load all configurations from directory
     * @param directory Directory path
     * @return Number of configurations loaded
     */
    int loadAllConfigs(const std::string& directory);

    /**
     * @brief Save all configurations to directory
     * @param directory Directory path
     * @return Number of configurations saved
     */
    int saveAllConfigs(const std::string& directory);

private:
    /**
     * @brief Constructor
     */
    DynamicConfigManager();

    /**
     * @brief Copy constructor (deleted)
     */
    DynamicConfigManager(const DynamicConfigManager&) = delete;

    /**
     * @brief Assignment operator (deleted)
     */
    DynamicConfigManager& operator=(const DynamicConfigManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<DynamicConfig>> m_configs; ///< Configurations
    mutable std::mutex m_mutex;                              ///< Mutex for thread safety
    bool m_initialized;                                      ///< Initialization flag
};

#endif // DYNAMIC_CONFIG_HPP