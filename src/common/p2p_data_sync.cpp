#include "p2p_data_sync.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cstring>
#include <curl/curl.h>
#include <hiredis/hiredis.h>
#include <libpq-fe.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>
#include "showmsg.hpp"
#include "timer.hpp"
#include "utils.hpp"

using json = nlohmann::json;

namespace rathena {

// Static member initialization
P2PDataSync* P2PDataSync::instance = nullptr;

// Helper function for CURL
size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch(std::bad_alloc &e) {
        return 0;
    }
    return newLength;
}

// Helper function for generating UUIDs
std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4";

    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";

    ss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";

    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }

    return ss.str();
}

// Helper function for base64 encoding
std::string base64Encode(const std::string& input) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    unsigned int in_len = input.size();
    const char* bytes_to_encode = input.c_str();

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

// Helper function for base64 decoding
std::string base64Decode(const std::string& encoded_string) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && 
           (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

// P2PDataSync implementation
P2PDataSync& P2PDataSync::getInstance() {
    if (!instance) {
        instance = new P2PDataSync();
    }
    return *instance;
}

P2PDataSync::P2PDataSync() 
    : redis_port(6379), 
      sync_interval_ms(1000), 
      cache_expiry_seconds(3600), 
      critical_timeout_ms(500),
      max_queue_size(10000), 
      worker_threads(4), 
      debug_mode(false),
      stop_workers(false),
      total_operations(0),
      successful_operations(0),
      failed_operations(0),
      processing_critical(false),
      cached_operations(0),
      last_status_update(std::chrono::system_clock::now()) {
    // Initialize CURL globally
    curl_global_init(CURL_GLOBAL_ALL);
}

P2PDataSync::~P2PDataSync() {
    final();
    curl_global_cleanup();
}

bool P2PDataSync::init(const std::string& config_path) {
    ShowInfo("Initializing P2P Data Synchronization System\n");
    
    // Load configuration
    if (!loadConfig(config_path)) {
        ShowError("Failed to load P2P data sync configuration\n");
        return false;
    }
    
    // Initialize Redis cache
    redis_cache = std::make_unique<RedisCache>(redis_host, redis_port, redis_password);
    if (!redis_cache->init()) {
        // Redis is important for real-time performance, but we can continue without it
        ShowWarning("Failed to initialize Redis cache, continuing without caching\n");
    } else {
        ShowInfo("Redis cache initialized successfully\n");
    }
    
    // Initialize PgVector database
    pg_vector_db = std::make_unique<PgVectorDB>(pg_connection_string);
    if (!pg_vector_db->init()) {
        // PgVector is not critical for real-time performance, so we can continue without it
        ShowWarning("Failed to initialize PgVector database, continuing without vector search\n");
    } else {
        ShowInfo("PgVector database initialized successfully\n");
    }
    
    // Initialize embedding provider
    if (use_azure_openai) {
        embedding_provider = std::make_unique<AzureOpenAIEmbeddingProvider>(azure_openai_endpoint, azure_openai_key);
        ShowInfo("Using Azure OpenAI for embeddings\n");
    } else {
        embedding_provider = std::make_unique<OpenAIEmbeddingProvider>(openai_api_key);
        ShowInfo("Using OpenAI for embeddings\n");
    }
    
    // Start worker threads
    stop_workers = false;
    for (uint32 i = 0; i < worker_threads; ++i) {
        worker_threads_vec.emplace_back(&P2PDataSync::syncWorkerThread, this);
    }
    
    // Start sync timer
    add_timer_func_list(P2PDataSync::sync_timer, "P2PDataSync::sync_timer");
    add_timer_interval(gettick() + sync_interval_ms, P2PDataSync::sync_timer, 0, 0, sync_interval_ms);
    
    ShowInfo("P2P Data Synchronization System initialized with %d worker threads\n", worker_threads);
    return true;
}

void P2PDataSync::final() {
    ShowInfo("Finalizing P2P Data Synchronization System\n");
    
    // Stop worker threads
    stop_workers = true;
    queue_cv.notify_all();
    
    for (auto& thread : worker_threads_vec) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_vec.clear();
    
    // Finalize components
    if (redis_cache) {
        redis_cache->final();
    }
    
    if (pg_vector_db) {
        pg_vector_db->final();
    }
    
    // Clear queues and results
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        while (!sync_queue.empty()) {
            sync_queue.pop();
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(results_mutex);
        operation_results.clear();
    }
    
    ShowInfo("P2P Data Synchronization System finalized\n");
}

bool P2PDataSync::loadConfig(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        ShowError("Failed to open P2P data sync configuration file: %s\n", config_path.c_str());
        return false;
    }
    
    try {
        std::string line;
        while (std::getline(config_file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#' || line[0] == '/') {
                continue;
            }
            
            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (key == "main_server_endpoint") {
                main_server_endpoint = value;
            } else if (key == "redis_host") {
                redis_host = value;
            } else if (key == "redis_port") {
                redis_port = std::stoi(value);
            } else if (key == "redis_password") {
                // Password is optional
                redis_password = value;
            } else if (key == "pg_connection_string") {
                pg_connection_string = value;
            } else if (key == "openai_api_key") {
                openai_api_key = value;
            } else if (key == "azure_openai_endpoint") {
                azure_openai_endpoint = value;
            } else if (key == "azure_openai_key") {
                azure_openai_key = value;
            } else if (key == "use_azure_openai") {
                use_azure_openai = (value == "1" || value == "true" || value == "yes");
            } else if (key == "encryption_key") {
                encryption_key = value;
            } else if (key == "sync_interval_ms") {
                sync_interval_ms = std::stoi(value);
            } else if (key == "critical_timeout_ms") {
                critical_timeout_ms = std::stoi(value);
            } else if (key == "cache_expiry_seconds") {
                cache_expiry_seconds = std::stoi(value);
            } else if (key == "max_queue_size") {
                max_queue_size = std::stoi(value);
            } else if (key == "worker_threads") {
                worker_threads = std::stoi(value);
            } else if (key == "debug_mode") {
                debug_mode = (value == "1" || value == "true" || value == "yes");
            }
        }
    } catch (const std::exception& e) {
        ShowError("Error parsing P2P data sync configuration: %s\n", e.what());
        return false;
    }
    
    // Validate required configuration
    if (main_server_endpoint.empty()) {
        ShowError("Missing main_server_endpoint in P2P data sync configuration\n");
        return false;
    }
    
    if (encryption_key.empty()) {
        ShowWarning("No encryption key specified, generating a random one\n");
        // Generate a random encryption key
        unsigned char key_data[32];
        RAND_bytes(key_data, sizeof(key_data));
        encryption_key = base64Encode(std::string(reinterpret_cast<char*>(key_data), sizeof(key_data)));
    }
    
    return true;
}

std::string P2PDataSync::syncToMainServer(SyncDataType type, uint32 account_id, const std::string& map_name, 
                                         const std::string& data, bool is_priority) {
    // Generate a unique operation ID
    std::string operation_id = generateOperationId();
    
    // Create a sync operation
    SyncOperation operation;
    operation.type = type;
    operation.account_id = account_id;
    operation.map_name = map_name;
    operation.data = data;
    operation.timestamp = std::chrono::system_clock::now();
    operation.is_priority = is_priority;
    operation.operation_id = operation_id;
    operation.is_critical = false;
    
    // Add to sync queue
    addToSyncQueue(operation);
    
    return operation_id;
}

std::string P2PDataSync::syncFromMainServer(SyncDataType type, uint32 account_id, const std::string& map_name, 
                                           const std::string& data, bool is_priority) {
    // Generate a unique operation ID
    std::string operation_id = generateOperationId();
    
    // Create a sync operation
    SyncOperation operation;
    operation.type = type;
    operation.account_id = account_id;
    operation.map_name = map_name;
    operation.data = data;
    operation.timestamp = std::chrono::system_clock::now();
    operation.is_priority = is_priority;
    operation.operation_id = operation_id;
    operation.is_critical = false;
    
    // Add to sync queue
    addToSyncQueue(operation);
    
    return operation_id;
}

std::string P2PDataSync::syncCriticalData(SyncDataType type, uint32 account_id, const std::string& map_name, 
                                         const std::string& data, bool is_priority) {
    // Generate a unique operation ID
    std::string operation_id = generateOperationId();
    
    // Create a sync operation
    SyncOperation operation;
    operation.type = type;
    operation.account_id = account_id;
    operation.map_name = map_name;
    operation.data = data;
    operation.timestamp = std::chrono::system_clock::now();
    operation.is_priority = is_priority;
    operation.is_critical = true;
    operation.operation_id = operation_id;
    
    // Process critical operations synchronously
    SyncResult result = processCriticalSyncOperation(operation);
    
    // Store result
    {
        std::lock_guard<std::mutex> lock(results_mutex);
        operation_results[operation_id] = result;
        results_cv.notify_all();
    }
    
    return operation_id;
}

SyncResult P2PDataSync::getSyncResult(const std::string& operation_id) {
    std::lock_guard<std::mutex> lock(results_mutex);
    auto it = operation_results.find(operation_id);
    if (it != operation_results.end()) {
        return it->second;
    }
    
    // Operation not found or not completed yet
    SyncResult result;
    result.success = false;
    result.error_message = "Operation not found or not completed yet";
    result.operation_id = operation_id;
    result.timestamp = std::chrono::system_clock::now();
    return result;
}

SyncResult P2PDataSync::waitForSyncResult(const std::string& operation_id, uint32 timeout_ms) {
    std::unique_lock<std::mutex> lock(results_mutex);
    
    // Check if result already exists
    auto it = operation_results.find(operation_id);
    if (it != operation_results.end()) {
        return it->second;
    }
    
    // Wait for result with timeout
    bool success = results_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this, &operation_id]() {
        return operation_results.find(operation_id) != operation_results.end();
    });
    
    if (success) {
        return operation_results[operation_id];
    }
    
    // Timeout occurred
    SyncResult result;
    result.success = false;
    result.error_message = "Operation timed out";
    result.operation_id = operation_id;
    result.timestamp = std::chrono::system_clock::now();
    return result;
}
std::string P2PDataSync::createEmbedding(const std::string& text) {
    if (!embedding_provider) {
        ShowError("Embedding provider not initialized\n");
        return "";
    }
    
    return embedding_provider->createEmbedding(text);
}

std::vector<std::string> P2PDataSync::searchSimilarEmbeddings(const std::string& embedding, size_t limit) {
    if (!pg_vector_db) {
        ShowError("PgVector database not initialized\n");
        return {};
    }
    
    return pg_vector_db->searchSimilarEmbeddings(embedding, limit);
}

bool P2PDataSync::cacheData(const std::string& key, const std::string& value, uint32 expiry_seconds) {
    if (!redis_cache) {
        if (debug_mode) {
            ShowDebug("Redis cache not initialized, skipping cache operation\n");
        }
        return false;
    }
    
    bool result = redis_cache->set(key, value, expiry_seconds > 0 ? expiry_seconds : cache_expiry_seconds);
    if (result) {
        cached_operations++;
    }
    return result;
}

std::string P2PDataSync::getCachedData(const std::string& key) {
    if (!redis_cache) {
        if (debug_mode) {
            ShowDebug("Redis cache not initialized, skipping cache operation\n");
        }
        return "";
    }
    
    return redis_cache->get(key);
}

bool P2PDataSync::deleteCachedData(const std::string& key) {
    if (!redis_cache) {
        if (debug_mode) {
            ShowDebug("Redis cache not initialized, skipping cache operation\n");
        }
        return false;
    }
    
    return redis_cache->del(key);
}

size_t P2PDataSync::processSyncQueue(size_t max_operations) {
    std::vector<SyncOperation> operations_to_process;
    
    // Get operations from the queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        size_t count = max_operations > 0 ? std::min(max_operations, sync_queue.size()) : sync_queue.size();
        for (size_t i = 0; i < count; ++i) {
            operations_to_process.push_back(sync_queue.front());
            sync_queue.pop();
        }
    }
    
    // Process operations
    for (const auto& operation : operations_to_process) {
        // Skip critical operations, they are processed synchronously
        SyncResult result = processSyncOperation(operation);
        
        // Store result
        {
            std::lock_guard<std::mutex> lock(results_mutex);
            operation_results[operation.operation_id] = result;
        }
        results_cv.notify_all();
    }
    
    return operations_to_process.size();
}

std::string P2PDataSync::getStatus() const {
    std::stringstream ss;
    
    ss << "P2P Data Synchronization System Status:\n";
    ss << "  Total operations: " << total_operations << "\n";
    ss << "  Successful operations: " << successful_operations << "\n";
    ss << "  Failed operations: " << failed_operations << "\n";
    ss << "  Cached operations: " << cached_operations << "\n";
    ss << "  Current queue size: " << sync_queue.size() << "\n";
    
    if (redis_cache) {
        ss << "  Redis cache: " << redis_cache->getStatus() << "\n";
    } else {
        ss << "  Redis cache: Not initialized\n";
    }
    
    if (pg_vector_db) {
        ss << "  PgVector database: " << pg_vector_db->getStatus() << "\n";
    } else {
        ss << "  PgVector database: Not initialized\n";
    }
    
    if (embedding_provider) {
        ss << "  Embedding provider: " << embedding_provider->getStatus() << "\n";
    } else {
        ss << "  Embedding provider: Not initialized\n";
    }
    
    return ss.str();
}

int P2PDataSync::sync_timer(int32 tid, t_tick tick, int32 id, intptr_t data) {
    P2PDataSync::getInstance().processSyncQueue(100); // Process up to 100 operations per tick
    return 0; // Continue the timer
}

std::string P2PDataSync::generateOperationId() {
    return generateUUID();
}

void P2PDataSync::syncWorkerThread() {
    ShowInfo("P2P data sync worker thread started\n");
    
    while (!stop_workers) {
        SyncOperation operation;
        bool has_operation = false;
        
        // Wait for an operation
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait_for(lock, std::chrono::milliseconds(100), [this] { 
                return !sync_queue.empty() || stop_workers; 
            });
            
            if (!sync_queue.empty()) {
                operation = sync_queue.front();
                sync_queue.pop();
                has_operation = true;
            }
        }
        
        // Process the operation
        if (has_operation) {
            // Skip critical operations, they are processed synchronously
            SyncResult result = processSyncOperation(operation);
            
            // Store result
            {
                std::lock_guard<std::mutex> lock(results_mutex);
                operation_results[operation.operation_id] = result;
            }
            results_cv.notify_all();
        }
    }
    
    ShowInfo("P2P data sync worker thread stopped\n");
}

void P2PDataSync::addToSyncQueue(const SyncOperation& operation) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    // Critical operations are processed synchronously, not added to the queue
    if (operation.is_critical) {
        return;
    }
    
    // Check if queue is full
    if (sync_queue.size() >= max_queue_size) {
        ShowWarning("P2P data sync queue is full, dropping oldest operation\n");
        sync_queue.pop(); // Remove oldest operation
    }
    
    // Add operation to queue
    if (operation.is_priority) {
        // For priority operations, we need to create a new queue with the priority operation at the front
        std::queue<SyncOperation> new_queue;
        new_queue.push(operation);
        
        // Copy existing operations
        while (!sync_queue.empty()) {
            new_queue.push(sync_queue.front());
            sync_queue.pop();
        }
        
        sync_queue = std::move(new_queue);
    } else {
        sync_queue.push(operation);
    }
    
    total_operations++;
    queue_cv.notify_one();
}

SyncResult P2PDataSync::processCriticalSyncOperation(const SyncOperation& operation) {
    SyncResult result;
    result.operation_id = operation.operation_id;
    result.timestamp = std::chrono::system_clock::now();
    
    // Set the processing_critical flag to true
    bool expected = false;
    if (!processing_critical.compare_exchange_strong(expected, true)) {
        // Another critical operation is already being processed
        // Wait for a short time and try again
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (processing_critical.load()) {
            result.success = false;
            result.error_message = "Another critical operation is already being processed";
            return result;
        }
    }
    
    // Make sure to reset the flag when we're done
    auto guard = std::unique_ptr<bool, std::function<void(bool*)>>(
        new bool(true),
        [this](bool* p) {
            processing_critical.store(false);
            delete p;
        }
    );
    
    try {
        // First check if we can use cached data for this operation
        std::string cache_key = "p2p_sync:critical:" + operation.operation_id;
        std::string cached_result;
        
        if (redis_cache) {
            cached_result = redis_cache->get(cache_key);
        }
        
        if (!cached_result.empty()) {
            if (debug_mode) {
                ShowDebug("Using cached result for critical operation %s\n", operation.operation_id.c_str());
            }
            
            // Parse cached result
            json cached_json = json::parse(cached_result);
            result.success = cached_json["success"].get<bool>();
            result.error_message = cached_json["error_message"].get<std::string>();
            
            if (result.success) {
                successful_operations++;
            } else {
                failed_operations++;
            }
            
            return result;
        }
        
        // Prepare data for synchronization
        json data_json;
        data_json["type"] = static_cast<int>(operation.type);
        data_json["account_id"] = operation.account_id;
        data_json["map_name"] = operation.map_name;
        data_json["data"] = operation.data;
        data_json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            operation.timestamp.time_since_epoch()).count();
        data_json["operation_id"] = operation.operation_id;
        data_json["is_critical"] = true;
        
        // Encrypt the data
        std::string encrypted_data = encryptData(data_json.dump(), encryption_key);
        
        // Prepare the request
        CURL* curl = curl_easy_init();
        if (!curl) {
            result.success = false;
            result.error_message = "Failed to initialize CURL";
            failed_operations++;
            return result;
        }
        
        // Set up the request with a short timeout for critical operations
        std::string response_string;
        std::string error_buffer;
        error_buffer.resize(CURL_ERROR_SIZE);
        
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Create request body
        json request_json;
        request_json["encrypted_data"] = encrypted_data;
        request_json["operation_id"] = operation.operation_id;
        request_json["is_critical"] = true;
        std::string request_body = request_json.dump();
        
        // Set CURL options with a short timeout for critical operations
        curl_easy_setopt(curl, CURLOPT_URL, main_server_endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer.data());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, critical_timeout_ms); // Short timeout for critical operations
        
        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        
        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        // Check for errors
        if (res != CURLE_OK) {
            result.success = false;
            result.error_message = "CURL error: " + std::string(error_buffer.data());
            failed_operations++;
            return result;
        }
        
        // Parse the response
        json response_json;
        try {
            response_json = json::parse(response_string);
        } catch (const std::exception& e) {
            result.success = false;
            result.error_message = "Failed to parse response: " + std::string(e.what());
            failed_operations++;
            return result;
        }
        
        // Check if the response contains encrypted data
        if (response_json.contains("encrypted_data")) {
            std::string decrypted_data = decryptData(response_json["encrypted_data"].get<std::string>(), encryption_key);
            json decrypted_json = json::parse(decrypted_data);
            
            result.success = decrypted_json["success"].get<bool>();
            result.error_message = decrypted_json["error_message"].get<std::string>();
        } else {
            result.success = response_json["success"].get<bool>();
            result.error_message = response_json["error_message"].get<std::string>();
        }
        
        // Cache the result for a short time
        if (redis_cache) {
            json cache_json;
            cache_json["success"] = result.success;
            cache_json["error_message"] = result.error_message;
            redis_cache->set(cache_key, cache_json.dump(), 60); // Cache for 60 seconds
        }
        
        if (result.success) {
            successful_operations++;
        } else {
            failed_operations++;
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Exception: " + std::string(e.what());
        failed_operations++;
    }
    
    return result;
}

SyncResult P2PDataSync::processSyncOperation(const SyncOperation& operation) {
    SyncResult result;
    result.operation_id = operation.operation_id;
    result.timestamp = std::chrono::system_clock::now();
    
    try {
        // Check if we can use cached data
        // For critical operations, use a different cache key prefix
        std::string cache_prefix = "p2p_sync:";
        if (operation.is_critical) {
            cache_prefix = "p2p_sync:critical:";
        }
        
        std::string cache_key = "p2p_sync:" + operation.operation_id;
        std::string cached_result = getCachedData(cache_key);
        
        if (!cached_result.empty()) {
            if (debug_mode) {
                ShowDebug("Using cached result for operation %s\n", operation.operation_id.c_str());
            }
            
            // Parse cached result
            json cached_json = json::parse(cached_result);
            result.success = cached_json["success"].get<bool>();
            result.error_message = cached_json["error_message"].get<std::string>();
            
            if (result.success) {
                successful_operations++;
            } else {
                failed_operations++;
            }
            
            return result;
        }
        
        // Prepare data for synchronization
        json data_json;
        data_json["type"] = static_cast<int>(operation.type);
        data_json["account_id"] = operation.account_id;
        data_json["map_name"] = operation.map_name;
        data_json["data"] = operation.data;
        data_json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            operation.timestamp.time_since_epoch()).count();
        data_json["operation_id"] = operation.operation_id;
        data_json["is_critical"] = operation.is_critical;
        
        // Encrypt the data
        std::string encrypted_data = encryptData(data_json.dump(), encryption_key);
        
        // Prepare the request
        CURL* curl = curl_easy_init();
        if (!curl) {
            result.success = false;
            result.error_message = "Failed to initialize CURL";
            failed_operations++;
            return result;
        }
        
        // Set up the request
        std::string response_string;
        std::string error_buffer;
        error_buffer.resize(CURL_ERROR_SIZE);
        
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Create request body
        json request_json;
        request_json["encrypted_data"] = encrypted_data;
        request_json["operation_id"] = operation.operation_id;
        request_json["is_critical"] = operation.is_critical;
        std::string request_body = request_json.dump();
        
        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_URL, main_server_endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer.data());
        
        // Use a shorter timeout for critical operations
        if (operation.is_critical) {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, critical_timeout_ms);
        } else {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); // 10 seconds timeout for non-critical operations
        }
        
        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        
        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        // Check for errors
        if (res != CURLE_OK) {
            result.success = false;
            result.error_message = "CURL error: " + std::string(error_buffer.data());
            failed_operations++;
            return result;
        }
        
        // Parse the response
        json response_json;
        try {
            response_json = json::parse(response_string);
        } catch (const std::exception& e) {
            result.success = false;
            result.error_message = "Failed to parse response: " + std::string(e.what());
            failed_operations++;
            return result;
        }
        
        // Check if the response contains encrypted data
        if (response_json.contains("encrypted_data")) {
            std::string decrypted_data = decryptData(response_json["encrypted_data"].get<std::string>(), encryption_key);
            json decrypted_json = json::parse(decrypted_data);
            
            result.success = decrypted_json["success"].get<bool>();
            result.error_message = decrypted_json["error_message"].get<std::string>();
        } else {
            result.success = response_json["success"].get<bool>();
            result.error_message = response_json["error_message"].get<std::string>();
        }
        
        // Cache the result
        json cache_json;
        cache_json["success"] = result.success;
        cache_json["error_message"] = result.error_message;
        
        // Use a shorter cache expiry for critical operations
        if (operation.is_critical) {
            cacheData(cache_key, cache_json.dump(), 60); // 60 seconds for critical operations
        } else {
            cacheData(cache_key, cache_json.dump(), cache_expiry_seconds);
        }
        cacheData(cache_key, cache_json.dump(), cache_expiry_seconds);
        
        if (result.success) {
            successful_operations++;
        } else {
            failed_operations++;
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Exception: " + std::string(e.what());
        failed_operations++;
    }
    
    return result;
}

bool P2PDataSync::connectToMainServer() {
    // This is a placeholder for actual connection logic
    // In a real implementation, this would establish a secure connection to the main server
    return true;
}

void P2PDataSync::disconnectFromMainServer() {
    // This is a placeholder for actual disconnection logic
}

std::string P2PDataSync::encryptData(const std::string& data, const std::string& key) {
    // Decode the base64 key
    std::string decoded_key = base64Decode(key);
    
    // Initialize OpenSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ShowError("Failed to create OpenSSL cipher context\n");
        return "";
    }
    
    // Generate a random IV
    unsigned char iv[16];
    RAND_bytes(iv, sizeof(iv));
    
    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                          reinterpret_cast<const unsigned char*>(decoded_key.data()), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ShowError("Failed to initialize encryption\n");
        return "";
    }
    
    // Allocate memory for the ciphertext
    int ciphertext_len = data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc());
    unsigned char* ciphertext = new unsigned char[ciphertext_len];
    
    // Encrypt the data
    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, 
                         reinterpret_cast<const unsigned char*>(data.data()), data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] ciphertext;
        ShowError("Failed to encrypt data\n");
        return "";
    }
    
    int ciphertext_final_len = ciphertext_len - len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &ciphertext_final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] ciphertext;
        ShowError("Failed to finalize encryption\n");
        return "";
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Combine IV and ciphertext
    std::string result(reinterpret_cast<char*>(iv), sizeof(iv));
    result.append(reinterpret_cast<char*>(ciphertext), len + ciphertext_final_len);
    
    delete[] ciphertext;
    
    // Base64 encode the result
    return base64Encode(result);
}

std::string P2PDataSync::decryptData(const std::string& encrypted_data, const std::string& key) {
    // Decode the base64 data and key
    std::string decoded_data = base64Decode(encrypted_data);
    std::string decoded_key = base64Decode(key);
    
    // Check if we have enough data for the IV
    if (decoded_data.size() <= 16) {
        ShowError("Encrypted data too short\n");
        return "";
    }
    
    // Extract the IV
    unsigned char iv[16];
    std::memcpy(iv, decoded_data.data(), 16);
    
    // Initialize OpenSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ShowError("Failed to create OpenSSL cipher context\n");
        return "";
    }
    
    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                          reinterpret_cast<const unsigned char*>(decoded_key.data()), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        ShowError("Failed to initialize decryption\n");
        return "";
    }
    
    // Allocate memory for the plaintext
    int plaintext_len = decoded_data.size() - 16;
    unsigned char* plaintext = new unsigned char[plaintext_len];
    
    // Decrypt the data
    int len;
    if (EVP_DecryptUpdate(ctx, plaintext, &len, 
                         reinterpret_cast<const unsigned char*>(decoded_data.data() + 16), 
                         decoded_data.size() - 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] plaintext;
        ShowError("Failed to decrypt data\n");
        return "";
    }
    
    int plaintext_final_len = plaintext_len - len;
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &plaintext_final_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        delete[] plaintext;
        ShowError("Failed to finalize decryption\n");
        return "";
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Create the result string
    std::string result(reinterpret_cast<char*>(plaintext), len + plaintext_final_len);
    
    delete[] plaintext;
    
    return result;
}

// RedisCache implementation
RedisCache::RedisCache(const std::string& host, uint16 port, const std::string& password, uint16 db)
    : host(host), port(port), password(password), redis_context(nullptr), connected(false) {
}

RedisCache::~RedisCache() {
    final();
}

bool RedisCache::init() {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    // Connect to Redis
    redisContext* context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err) {
        if (context) {
            ShowError("Redis connection error: %s\n", context->errstr);
            redisFree(context);
        } else {
            ShowError("Failed to allocate Redis context\n");
        }
        return false;
    }
    
    // Authenticate if password is provided
    if (!password.empty()) {
        redisReply* reply = (redisReply*)redisCommand(context, "AUTH %s", password.c_str());
        if (reply == nullptr) {
            ShowError("Redis authentication error: %s\n", context->errstr);
            redisFree(context);
            return false;
        }
        
        if (reply->type == REDIS_REPLY_ERROR) {
            ShowError("Redis authentication error: %s\n", reply->str);
            freeReplyObject(reply);
            redisFree(context);
            return false;
        }
        
        freeReplyObject(reply);
    }
    
    // Select database if specified
    if (db > 0) {
        redisReply* reply = (redisReply*)redisCommand(context, "SELECT %d", db);
        if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
            ShowError("Redis SELECT error: %s\n", context->errstr);
            freeReplyObject(reply);
            redisFree(context);
            return false;
        }
    }
    
    redis_context = context;
    connected = true;
    
    ShowInfo("Connected to Redis at %s:%d\n", host.c_str(), port);
    return true;
}

void RedisCache::final() {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (redis_context) {
        redisFree((redisContext*)redis_context);
        redis_context = nullptr;
    }
    
    connected = false;
}

bool RedisCache::set(const std::string& key, const std::string& value, uint32 expiry_seconds) {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (!connected || !redis_context) {
        return false;
    }
    
    redisReply* reply;
    
    if (expiry_seconds > 0) {
        reply = (redisReply*)redisCommand((redisContext*)redis_context, 
                                         "SETEX %s %d %s", 
                                         key.c_str(), expiry_seconds, value.c_str());
    } else {
        reply = (redisReply*)redisCommand((redisContext*)redis_context, 
                                         "SET %s %s", 
                                         key.c_str(), value.c_str());
    }
    
    if (reply == nullptr) {
        ShowError("Redis SET error: %s\n", ((redisContext*)redis_context)->errstr);
        return false;
    }
    
    bool success = (reply->type != REDIS_REPLY_ERROR);
    
    if (!success) {
        ShowError("Redis SET error: %s\n", reply->str);
    }
    
    freeReplyObject(reply);
    return success;
}

std::string RedisCache::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (!connected || !redis_context) {
        return "";
    }
    
    redisReply* reply = (redisReply*)redisCommand((redisContext*)redis_context, 
                                                 "GET %s", 
                                                 key.c_str());
    
    if (reply == nullptr) {
        ShowError("Redis GET error: %s\n", ((redisContext*)redis_context)->errstr);
        return "";
    }
    
    std::string result;
    
    if (reply->type == REDIS_REPLY_STRING) {
        result = std::string(reply->str, reply->len);
    }
    
    freeReplyObject(reply);
    return result;
}

bool RedisCache::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (!connected || !redis_context) {
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand((redisContext*)redis_context, 
                                                 "DEL %s", 
                                                 key.c_str());
    
    if (reply == nullptr) {
        ShowError("Redis DEL error: %s\n", ((redisContext*)redis_context)->errstr);
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    
    freeReplyObject(reply);
    return success;
}

bool RedisCache::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (!connected || !redis_context) {
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand((redisContext*)redis_context, 
                                                 "EXISTS %s", 
                                                 key.c_str());
    
    if (reply == nullptr) {
        ShowError("Redis EXISTS error: %s\n", ((redisContext*)redis_context)->errstr);
        return false;
    }
    
    bool exists = (reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
    
    freeReplyObject(reply);
    return exists;
}

bool RedisCache::publish(const std::string& channel, const std::string& message) {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (!connected || !redis_context) {
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand((redisContext*)redis_context, 
                                                 "PUBLISH %s %s", 
                                                 channel.c_str(), message.c_str());
    
    if (reply == nullptr) {
        ShowError("Redis PUBLISH error: %s\n", ((redisContext*)redis_context)->errstr);
        return false;
    }
    
    freeReplyObject(reply);
    return true;
}

bool RedisCache::flushAll() {
    std::lock_guard<std::mutex> lock(redis_mutex);
    
    if (!connected || !redis_context) {
        return false;
    }
    
    redisReply* reply = (redisReply*)redisCommand((redisContext*)redis_context, "FLUSHALL");
    
    if (reply == nullptr) {
        ShowError("Redis FLUSHALL error: %s\n", ((redisContext*)redis_context)->errstr);
        return false;
    }
    
    bool success = (reply->type == REDIS_REPLY_STATUS && 
                   std::string(reply->str) == "OK");
    
    freeReplyObject(reply);
    return success;
}

std::string RedisCache::getStatus() const {
    if (!connected) {
        return "Not connected";
    }
    
    return "Connected to " + host + ":" + std::to_string(port);
}

// PgVectorDB implementation
PgVectorDB::PgVectorDB(const std::string& connection_string)
    : connection_string(connection_string), pg_conn(nullptr), connected(false) {
}

PgVectorDB::~PgVectorDB() {
    final();
}

bool PgVectorDB::init() {
    std::lock_guard<std::mutex> lock(pg_mutex);
    
    // Connect to PostgreSQL
    PGconn* conn = PQconnectdb(connection_string.c_str());
    
    if (PQstatus(conn) != CONNECTION_OK) {
        ShowError("PostgreSQL connection error: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return false;
    }
    
    // Check if pgvector extension is installed
    PGresult* result = PQexec(conn, "SELECT 1 FROM pg_extension WHERE extname = 'vector'");
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        ShowError("PostgreSQL query error: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return false;
    }
    
    if (PQntuples(result) == 0) {
        ShowError("pgvector extension is not installed in the database\n");
        PQclear(result);
        PQfinish(conn);
        return false;
    }
    
    PQclear(result);
    
    // Create embeddings table if it doesn't exist
    result = PQexec(conn, 
                   "CREATE TABLE IF NOT EXISTS p2p_embeddings ("
                   "  id TEXT PRIMARY KEY,"
                   "  embedding VECTOR(1536),"
                   "  metadata JSONB,"
                   "  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()"
                   ")");
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        ShowError("Failed to create embeddings table: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return false;
    }
    
    PQclear(result);
    
    // Create index on embeddings
    result = PQexec(conn, 
                   "CREATE INDEX IF NOT EXISTS p2p_embeddings_embedding_idx "
                   "ON p2p_embeddings USING ivfflat (embedding vector_cosine_ops)");
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        ShowError("Failed to create embeddings index: %s\n", PQerrorMessage(conn));
        PQclear(result);
        PQfinish(conn);
        return false;
    }
    
    PQclear(result);
    
    pg_conn = conn;
    connected = true;
    
    ShowInfo("Connected to PostgreSQL with pgvector extension\n");
    return true;
}

void PgVectorDB::final() {
    std::lock_guard<std::mutex> lock(pg_mutex);
    
    if (pg_conn) {
        PQfinish((PGconn*)pg_conn);
        pg_conn = nullptr;
    }
    
    connected = false;
}

bool PgVectorDB::storeEmbedding(const std::string& id, const std::string& embedding, const std::string& metadata) {
    std::lock_guard<std::mutex> lock(pg_mutex);
    
    if (!connected || !pg_conn) {
        return false;
    }
    
    // Prepare the query
    const char* paramValues[3];
    paramValues[0] = id.c_str();
    paramValues[1] = embedding.c_str();
    paramValues[2] = metadata.c_str();
    
    PGresult* result = PQexecParams((PGconn*)pg_conn,
                                   "INSERT INTO p2p_embeddings (id, embedding, metadata) "
                                   "VALUES ($1, $2::vector, $3::jsonb) "
                                   "ON CONFLICT (id) DO UPDATE "
                                   "SET embedding = $2::vector, metadata = $3::jsonb",
                                   3, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        ShowError("Failed to store embedding: %s\n", PQerrorMessage((PGconn*)pg_conn));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

std::vector<std::string> PgVectorDB::searchSimilarEmbeddings(const std::string& embedding, size_t limit) {
    std::lock_guard<std::mutex> lock(pg_mutex);
    
    std::vector<std::string> results;
    
    if (!connected || !pg_conn) {
        return results;
    }
    
    // Prepare the query
    std::string query = "SELECT id, metadata, embedding <=> $1::vector AS distance "
                       "FROM p2p_embeddings "
                       "ORDER BY distance "
                       "LIMIT " + std::to_string(limit);
    
    const char* paramValues[1];
    paramValues[0] = embedding.c_str();
    
    PGresult* result = PQexecParams((PGconn*)pg_conn, query.c_str(), 1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        ShowError("Failed to search embeddings: %s\n", PQerrorMessage((PGconn*)pg_conn));
        PQclear(result);
        return results;
    }
    
    // Process results
    int rows = PQntuples(result);
    for (int i = 0; i < rows; ++i) {
        std::string id = PQgetvalue(result, i, 0);
        std::string metadata = PQgetvalue(result, i, 1);
        
        // Create a JSON object with the result
        json result_json;
        result_json["id"] = id;
        result_json["metadata"] = json::parse(metadata);
        result_json["distance"] = std::stod(PQgetvalue(result, i, 2));
        
        results.push_back(result_json.dump());
    }
    
    PQclear(result);
    return results;
}

bool PgVectorDB::deleteEmbedding(const std::string& id) {
    std::lock_guard<std::mutex> lock(pg_mutex);
    
    if (!connected || !pg_conn) {
        return false;
    }
    
    // Prepare the query
    const char* paramValues[1];
    paramValues[0] = id.c_str();
    
    PGresult* result = PQexecParams((PGconn*)pg_conn,
                                   "DELETE FROM p2p_embeddings WHERE id = $1",
                                   1, NULL, paramValues, NULL, NULL, 0);
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        ShowError("Failed to delete embedding: %s\n", PQerrorMessage((PGconn*)pg_conn));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

std::string PgVectorDB::getStatus() const {
    if (!connected) {
        return "Not connected";
    }
    
    return "Connected to PostgreSQL with pgvector extension";
}

// OpenAIEmbeddingProvider implementation
OpenAIEmbeddingProvider::OpenAIEmbeddingProvider(const std::string& api_key)
    : api_key(api_key), model("text-embedding-ada-002") {
}

OpenAIEmbeddingProvider::~OpenAIEmbeddingProvider() {
}

std::string OpenAIEmbeddingProvider::createEmbedding(const std::string& text) {
    std::lock_guard<std::mutex> lock(api_mutex);
    
    if (api_key.empty()) {
        ShowError("OpenAI API key is not set\n");
        return "";
    }
    
    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        ShowError("Failed to initialize CURL\n");
        return "";
    }
    
    // Prepare the request
    std::string response_string;
    std::string error_buffer;
    error_buffer.resize(CURL_ERROR_SIZE);
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
    
    // Create request body
    json request_json;
    request_json["input"] = text;
    request_json["model"] = model;
    std::string request_body = request_json.dump();
    
    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/embeddings");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer.data());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30); // 30 seconds timeout
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        ShowError("CURL error: %s\n", error_buffer.data());
        return "";
    }
    
    // Parse the response
    try {
        json response_json = json::parse(response_string);
        
        if (response_json.contains("error")) {
            ShowError("OpenAI API error: %s\n", response_json["error"]["message"].get<std::string>().c_str());
            return "";
        }
        
        // Extract the embedding
        std::vector<float> embedding = response_json["data"][0]["embedding"].get<std::vector<float>>();
        
        // Convert the embedding to a string
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < embedding.size(); ++i) {
            if (i > 0) {
                ss << ",";
            }
            ss << embedding[i];
        }
        ss << "]";
        
        return ss.str();
    } catch (const std::exception& e) {
        ShowError("Failed to parse OpenAI response: %s\n", e.what());
        return "";
    }
}

std::string OpenAIEmbeddingProvider::getStatus() const {
    return "OpenAI embedding provider using model " + model;
}

// AzureOpenAIEmbeddingProvider implementation
AzureOpenAIEmbeddingProvider::AzureOpenAIEmbeddingProvider(const std::string& endpoint, const std::string& api_key)
    : endpoint(endpoint), api_key(api_key), deployment_id("text-embedding-ada-002") {
}

AzureOpenAIEmbeddingProvider::~AzureOpenAIEmbeddingProvider() {
}

std::string AzureOpenAIEmbeddingProvider::createEmbedding(const std::string& text) {
    std::lock_guard<std::mutex> lock(api_mutex);
    
    if (endpoint.empty() || api_key.empty()) {
        ShowError("Azure OpenAI endpoint or API key is not set\n");
        return "";
    }
    
    // Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        ShowError("Failed to initialize CURL\n");
        return "";
    }
    
    // Prepare the request
    std::string response_string;
    std::string error_buffer;
    error_buffer.resize(CURL_ERROR_SIZE);
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("api-key: " + api_key).c_str());
    
    // Create request body
    json request_json;
    request_json["input"] = text;
    std::string request_body = request_json.dump();
    
    // Set CURL options
    std::string url = endpoint + "/openai/deployments/" + deployment_id + "/embeddings?api-version=2023-05-15";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer.data());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30); // 30 seconds timeout
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        ShowError("CURL error: %s\n", error_buffer.data());
        return "";
    }
    
    // Parse the response
    try {
        json response_json = json::parse(response_string);
        
        if (response_json.contains("error")) {
            ShowError("Azure OpenAI API error: %s\n", response_json["error"]["message"].get<std::string>().c_str());
            return "";
        }
        
        // Extract the embedding
        std::vector<float> embedding = response_json["data"][0]["embedding"].get<std::vector<float>>();
        
        // Convert the embedding to a string
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < embedding.size(); ++i) {
            if (i > 0) {
                ss << ",";
            }
            ss << embedding[i];
        }
        ss << "]";
        
        return ss.str();
    } catch (const std::exception& e) {
        ShowError("Failed to parse Azure OpenAI response: %s\n", e.what());
        return "";
    }
}

std::string AzureOpenAIEmbeddingProvider::getStatus() const {
    return "Azure OpenAI embedding provider using deployment " + deployment_id;
}

} // namespace rathena