#ifndef P2P_DATA_SYNC_HPP
#define P2P_DATA_SYNC_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>
#include "../common/cbasetypes.hpp"
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"

namespace rathena {

// Forward declarations
class RedisCache;
class PgVectorDB;
class EmbeddingProvider;

/**
 * @brief Enum representing different types of data that can be synchronized
 */
enum class SyncDataType {
    MAP_STATE,
    MONSTER_DATA,
    PLAYER_POSITION,
    ITEM_DROP,
    NPC_STATE,
    QUEST_PROGRESS,
    CRITICAL_GAME_STATE,
    CHAT_MESSAGE,
    VECTOR_EMBEDDING
};

/**
 * @brief Structure representing a data synchronization operation
 */
struct SyncOperation {
    SyncDataType type;
    uint32 account_id;
    std::string map_name;
    std::string data;
    std::chrono::system_clock::time_point timestamp;
    bool is_priority;
    bool is_critical;
    std::string operation_id;
};

/**
 * @brief Structure representing a data synchronization result
 */
struct SyncResult {
    bool success;
    std::string error_message;
    std::string operation_id;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Class for handling secure data synchronization between P2P hosts and the main server
 */
class P2PDataSync {
public:
    /**
     * @brief Get the singleton instance of P2PDataSync
     * @return Reference to the singleton instance
     */
    static P2PDataSync& getInstance();

    /**
     * @brief Initialize the data synchronization system
     * @param config_path Path to the configuration file
     * @return True if initialization was successful, false otherwise
     */
    bool init(const std::string& config_path);

    /**
     * @brief Finalize the data synchronization system
     */
    void final();

    /**
     * @brief Synchronize data from a P2P host to the main server
     * @param type Type of data to synchronize
     * @param account_id Account ID of the P2P host
     * @param map_name Name of the map
     * @param data Data to synchronize
     * @param is_priority Whether this is a priority synchronization
     * @return Operation ID for tracking the synchronization
     */
    std::string syncToMainServer(SyncDataType type, uint32 account_id, const std::string& map_name, 
                                const std::string& data, bool is_priority = false);

    /**
     * @brief Synchronize data from the main server to a P2P host
     * @param type Type of data to synchronize
     * @param account_id Account ID of the P2P host
     * @param map_name Name of the map
     * @param data Data to synchronize
     * @param is_priority Whether this is a priority synchronization
     * @return Operation ID for tracking the synchronization
     */
    std::string syncFromMainServer(SyncDataType type, uint32 account_id, const std::string& map_name, 
                                  const std::string& data, bool is_priority = false);

    /**
     * @brief Synchronize critical data to/from the main server with real-time guarantees
     * @param type Type of data to synchronize
     * @param account_id Account ID of the P2P host
     * @param map_name Name of the map
     * @param data Data to synchronize
     * @param is_priority Whether this is a priority synchronization
     * @return Operation ID for tracking the synchronization
     */
    std::string syncCriticalData(SyncDataType type, uint32 account_id, const std::string& map_name, 
                                  const std::string& data, bool is_priority = false);

    /**
     * @brief Get the result of a synchronization operation
     * @param operation_id ID of the operation to check
     * @return Result of the synchronization operation
     */
    SyncResult getSyncResult(const std::string& operation_id);

    /**
     * @brief Wait for a synchronization operation to complete
     * @param operation_id ID of the operation to wait for
     * @param timeout_ms Maximum time to wait in milliseconds (0 for infinite)
     * @return Result of the synchronization operation
     */
    SyncResult waitForSyncResult(const std::string& operation_id, uint32 timeout_ms = 5000);

    /**
     * @brief Create a vector embedding for text data
     * @param text Text to create an embedding for
     * @return Vector embedding as a string
     */
    std::string createEmbedding(const std::string& text);

    /**
     * @brief Search for similar vector embeddings
     * @param embedding Vector embedding to search for
     * @param limit Maximum number of results to return
     * @return Vector of similar embeddings
     */
    std::vector<std::string> searchSimilarEmbeddings(const std::string& embedding, size_t limit = 10);

    /**
     * @brief Cache data in Redis
     * @param key Key to cache the data under
     * @param value Data to cache
     * @param expiry_seconds Time in seconds after which the data expires (0 for no expiry)
     * @return True if caching was successful, false otherwise
     */
    bool cacheData(const std::string& key, const std::string& value, uint32 expiry_seconds = 0);

    /**
     * @brief Get cached data from Redis
     * @param key Key to retrieve the data for
     * @return Cached data, or empty string if not found
     */
    std::string getCachedData(const std::string& key);

    /**
     * @brief Delete cached data from Redis
     * @param key Key to delete the data for
     * @return True if deletion was successful, false otherwise
     */
    bool deleteCachedData(const std::string& key);

    /**
     * @brief Process the synchronization queue
     * @param max_operations Maximum number of operations to process (0 for all)
     * @return Number of operations processed
     */
    size_t processSyncQueue(size_t max_operations = 0);

    /**
     * @brief Get the status of the data synchronization system
     * @return Status information as a string
     */
    std::string getStatus() const;

    /**
     * @brief Timer callback for processing the synchronization queue
     * @param tid Timer ID
     * @param tick Current tick
     * @param id ID
     * @param data Data
     * @return 0 to continue the timer, 1 to stop it
     */
    static int sync_timer(int32 tid, t_tick tick, int32 id, intptr_t data);

private:
    P2PDataSync();
    ~P2PDataSync();

    /**
     * @brief Load configuration from file
     * @param config_path Path to the configuration file
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig(const std::string& config_path);

    /**
     * @brief Encrypt data for secure transmission
     * @param data Data to encrypt
     * @param key Encryption key
     * @return Encrypted data
     */
    std::string encryptData(const std::string& data, const std::string& key);

    /**
     * @brief Decrypt data after secure transmission
     * @param encrypted_data Encrypted data
     * @param key Encryption key
     * @return Decrypted data
     */
    std::string decryptData(const std::string& encrypted_data, const std::string& key);

    /**
     * @brief Generate a unique operation ID
     * @return Unique operation ID
     */
    std::string generateOperationId();

    /**
     * @brief Process a critical synchronization operation synchronously
     * @param operation Operation to process
     * @return Result of the operation
     */
    SyncResult processCriticalSyncOperation(const SyncOperation& operation);

    /**
     * @brief Worker thread function for processing the synchronization queue
     */
    void syncWorkerThread();

    /**
     * @brief Add an operation to the synchronization queue
     * @param operation Operation to add
     */
    void addToSyncQueue(const SyncOperation& operation);

    /**
     * @brief Process a single synchronization operation
     * @param operation Operation to process
     * @return Result of the operation
     */
    SyncResult processSyncOperation(const SyncOperation& operation);

    /**
     * @brief Connect to the main database server
     * @return True if connection was successful, false otherwise
     */
    bool connectToMainServer();

    /**
     * @brief Disconnect from the main database server
     */
    void disconnectFromMainServer();

    // Singleton instance
    static P2PDataSync* instance;

    // Configuration
    std::string main_server_endpoint;
    std::string redis_host;
    uint16 redis_port;
    std::string redis_password;
    uint16 redis_db;
    std::string pg_connection_string;
    std::string openai_api_key;
    std::string azure_openai_endpoint;
    std::string azure_openai_key;
    bool use_azure_openai;
    std::string encryption_key;
    uint32 sync_interval_ms;
    uint32 cache_expiry_seconds;
    uint32 max_queue_size;
    uint32 critical_timeout_ms;
    uint32 worker_threads;
    bool debug_mode;

    // Components
    std::unique_ptr<RedisCache> redis_cache;
    std::unique_ptr<PgVectorDB> pg_vector_db;
    std::unique_ptr<EmbeddingProvider> embedding_provider;

    // Synchronization queue
    std::queue<SyncOperation> sync_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> stop_workers;
    std::atomic<bool> processing_critical;

    // Worker threads
    std::vector<std::thread> worker_threads_vec;

    // Operation results
    std::map<std::string, SyncResult> operation_results;
    std::mutex results_mutex;
    
    // Condition variable for waiting on results
    std::condition_variable results_cv;

    // Statistics
    std::atomic<uint64> total_operations;
    std::atomic<uint64> successful_operations;
    std::atomic<uint64> failed_operations;
    std::atomic<uint64> cached_operations;
    std::chrono::system_clock::time_point last_status_update;
};

/**
 * @brief Redis cache implementation
 */
class RedisCache {
public:
    RedisCache(const std::string& host, uint16 port, const std::string& password, uint16 db = 0);
    ~RedisCache();

    bool init();
    void final();

    bool set(const std::string& key, const std::string& value, uint32 expiry_seconds = 0);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    bool publish(const std::string& channel, const std::string& message);
    bool flushAll();
    std::string getStatus() const;

private:
    std::string host;
    uint16 port;
    std::string password;
    uint16 db;
    void* redis_context; // hiredis redisContext*
    std::mutex redis_mutex;
    bool connected;
};

/**
 * @brief PgVector database implementation
 */
class PgVectorDB {
public:
    PgVectorDB(const std::string& connection_string);
    ~PgVectorDB();

    bool init();
    void final();

    bool storeEmbedding(const std::string& id, const std::string& embedding, const std::string& metadata);
    std::vector<std::string> searchSimilarEmbeddings(const std::string& embedding, size_t limit = 10);
    bool deleteEmbedding(const std::string& id);
    std::string getStatus() const;

private:
    std::string connection_string;
    void* pg_conn; // PGconn*
    std::mutex pg_mutex;
    bool connected;
};

/**
 * @brief Embedding provider interface
 */
class EmbeddingProvider {
public:
    virtual ~EmbeddingProvider() = default;
    virtual std::string createEmbedding(const std::string& text) = 0;
    virtual std::string getStatus() const = 0;
};

/**
 * @brief OpenAI embedding provider implementation
 */
class OpenAIEmbeddingProvider : public EmbeddingProvider {
public:
    OpenAIEmbeddingProvider(const std::string& api_key);
    ~OpenAIEmbeddingProvider() override;

    std::string createEmbedding(const std::string& text) override;
    std::string getStatus() const override;

private:
    std::string api_key;
    std::string model;
    std::mutex api_mutex;
};

/**
 * @brief Azure OpenAI embedding provider implementation
 */
class AzureOpenAIEmbeddingProvider : public EmbeddingProvider {
public:
    AzureOpenAIEmbeddingProvider(const std::string& endpoint, const std::string& api_key);
    ~AzureOpenAIEmbeddingProvider() override;

    std::string createEmbedding(const std::string& text) override;
    std::string getStatus() const override;

private:
    std::string endpoint;
    std::string api_key;
    std::string deployment_id;
    std::mutex api_mutex;
};

} // namespace rathena

#endif // P2P_DATA_SYNC_HPP