# P2P Data Synchronization System

This document describes the secure data synchronization system between P2P hosts and the main database server in the rAthena P2P-first hosting architecture.

## Overview

The P2P Data Synchronization System enables real-time data synchronization between P2P hosts and the main server without exposing sensitive information. It uses a combination of technologies to ensure security, performance, and reliability:

1. **Secure Communication**: All data is encrypted before transmission, and the main server's IP and credentials are never exposed to P2P hosts.
2. **Redis Caching**: Redis is used for high-performance caching to reduce load on the main database and improve response times.
3. **pgvector Database**: PostgreSQL with pgvector extension is used for vector similarity search, enabling AI features.
4. **OpenAI/Azure OpenAI Integration**: Text embedding capabilities for semantic search and AI-powered features.
5. **Multi-threaded Processing**: Asynchronous processing of synchronization operations for optimal performance.
6. **Real-time Performance Optimization**: Special handling for critical game data to ensure real-time performance.

## Architecture

The system consists of the following components:

### 1. P2PDataSync Class

The core class that manages data synchronization between P2P hosts and the main server. It provides methods for:

- Synchronizing data to/from the main server
- Caching data in Redis
- Creating and searching vector embeddings
- Processing synchronization operations asynchronously
- Real-time synchronization for critical game data

### 2. Redis Cache

Redis is used for caching frequently accessed data to improve performance and reduce load on the main database server. The `RedisCache` class provides methods for:

- Setting cache entries with optional expiration
- Getting cached data
- Deleting cache entries
- Checking if a cache entry exists
- Publishing messages for real-time updates

### 3. PgVector Database

PostgreSQL with pgvector extension is used for vector similarity search, enabling AI features. The `PgVectorDB` class provides methods for:

- Storing vector embeddings with metadata
- Searching for similar embeddings
- Deleting embeddings

### 4. Embedding Providers

The system supports both OpenAI and Azure OpenAI for text embedding. The `EmbeddingProvider` interface is implemented by:

- `OpenAIEmbeddingProvider`: Uses OpenAI's API for text embedding
- `AzureOpenAIEmbeddingProvider`: Uses Azure OpenAI's API for text embedding

### 5. Real-time Performance Optimization

The system includes special handling for critical game data that requires real-time performance:

- **Critical Data Synchronization**: Synchronous processing of critical operations with priority handling
- **Redis Pub/Sub**: Real-time notification system for critical updates
- **Low-latency Communication**: Optimized communication paths for critical data
- **Prioritized Processing**: Critical operations are processed before non-critical ones

## Security Considerations

The system is designed with security as a top priority:

1. **Encrypted Communication**: All data is encrypted using AES-256-CBC before transmission.
2. **No Sensitive Information Exposure**: The main server's IP, database credentials, and other sensitive information are never exposed to P2P hosts.
3. **Server-Side Validation**: All data from P2P hosts is validated on the server side before being applied to the database.
4. **Secure Key Management**: Encryption keys are securely managed and can be rotated as needed.

## Setup and Configuration

### Prerequisites

1. Redis server
2. PostgreSQL with pgvector extension
3. OpenAI or Azure OpenAI API key (for text embedding)
4. libhiredis and libpq development libraries

### Configuration File

The system is configured through the `p2p_data_sync.conf` file, which includes settings for:

- Main server endpoint
- Redis connection
- PostgreSQL connection
- OpenAI/Azure OpenAI API keys
- Real-time performance settings
- Encryption key
- Synchronization settings
- Debug mode

Example configuration:

```ini
# Main server endpoint (required)
main_server_endpoint = https://sync-proxy.example.com/p2p-sync

# Redis cache configuration for high-performance caching
redis_host = localhost
redis_port = 6379
redis_password = 
redis_db = 0

# Real-time performance settings
critical_timeout_ms = 500
critical_data_types = PLAYER_POSITION,MONSTER_DATA,MAP_STATE

# PostgreSQL with pgvector configuration for vector similarity search
pg_connection_string = postgresql://postgres:postgres@localhost:5432/rathena_p2p

# OpenAI configuration
openai_api_key = 

# Azure OpenAI configuration
azure_openai_endpoint = 
azure_openai_key = 
use_azure_openai = 0

# Encryption key for secure data transmission
encryption_key = 

# Synchronization settings
sync_interval_ms = 1000
cache_expiry_seconds = 3600
max_queue_size = 10000
worker_threads = 4

# Debug mode (0 = disabled, 1 = enabled)
debug_mode = 0
```

### Database Setup

The system requires a PostgreSQL database with the pgvector extension. The schema is defined in `sql-files/p2p_data_sync.sql` and includes tables for:

- Vector embeddings
- Synchronization operations
- Map states
- Security validations
- Cache entries
- Host metrics
- Host assignments

To set up the database:

1. Install PostgreSQL and the pgvector extension
2. Create a database for the P2P data synchronization system
3. Run the SQL script to create the schema:

```bash
psql -U postgres -d rathena_p2p -f sql-files/p2p_data_sync.sql
```

## Usage

### Initializing the System

To initialize the P2P data synchronization system:

```cpp
#include "common/p2p_data_sync.hpp"

// Initialize the system
rathena::P2PDataSync::getInstance().init("conf/p2p_data_sync.conf");
```

### Synchronizing Data to the Main Server

To synchronize data from a P2P host to the main server:

```cpp
#include "common/p2p_data_sync.hpp"

// Synchronize map state data
std::string operation_id = rathena::P2PDataSync::getInstance().syncToMainServer(
    rathena::SyncDataType::MAP_STATE,
    account_id,
    map_name,
    map_state_json,
    false // not priority
);

// Check the result later
rathena::SyncResult result = rathena::P2PDataSync::getInstance().getSyncResult(operation_id);
if (result.success) {
    // Synchronization successful
} else {
    // Synchronization failed
    ShowError("Sync error: %s\n", result.error_message.c_str());
}
```

### Synchronizing Critical Data with Real-time Performance

For critical game data that requires real-time performance:

```cpp
#include "common/p2p_data_sync.hpp"

// Synchronize critical map state data
std::string operation_id = rathena::P2PDataSync::getInstance().syncCriticalData(
    rathena::SyncDataType::MAP_STATE,
    account_id,
    map_name,
    map_state_json,
    true // priority
);

// Wait for the result to ensure real-time performance
rathena::SyncResult result = rathena::P2PDataSync::getInstance().waitForSyncResult(operation_id, 500); // 500ms timeout
```

### Synchronizing Data from the Main Server

To synchronize data from the main server to a P2P host:

```cpp
#include "common/p2p_data_sync.hpp"

// Synchronize monster data
std::string operation_id = rathena::P2PDataSync::getInstance().syncFromMainServer(
    rathena::SyncDataType::MONSTER_DATA,
    account_id,
    map_name,
    request_json,
    true // priority
);

// Check the result later
rathena::SyncResult result = rathena::P2PDataSync::getInstance().getSyncResult(operation_id);
if (result.success) {
    // Synchronization successful
    // Process the data
} else {
    // Synchronization failed
    ShowError("Sync error: %s\n", result.error_message.c_str());
}
```

### Using Redis Cache

To cache data for improved performance:

```cpp
#include "common/p2p_data_sync.hpp"

// Cache data
std::string cache_key = "map_state:" + map_name;
rathena::P2PDataSync::getInstance().cacheData(cache_key, map_state_json, 3600);

// Get cached data
std::string cached_data = rathena::P2PDataSync::getInstance().getCachedData(cache_key);
if (!cached_data.empty()) {
    // Use cached data
} else {
    // Cache miss, fetch from main server
}
```

### Using Vector Embeddings

To create and search for vector embeddings:

```cpp
#include "common/p2p_data_sync.hpp"

// Create an embedding for text
std::string text = "This is a sample text for embedding";
std::string embedding = rathena::P2PDataSync::getInstance().createEmbedding(text);

// Search for similar embeddings
std::vector<std::string> similar_embeddings = rathena::P2PDataSync::getInstance().searchSimilarEmbeddings(embedding, 5);
for (const auto& similar : similar_embeddings) {
    // Process similar embeddings
}
```

## Integration with P2P Hosting

The P2P data synchronization system is integrated with the P2P hosting system to enable real-time data synchronization between P2P hosts and the main server. The integration points include:

### 1. Map State Synchronization

When a player enters or leaves a map, the map state is synchronized between the P2P host and the main server:

```cpp
bool P2PMapServer::onPlayerEnterMap(struct map_session_data* sd, const std::string& map_name) {
    // ... existing code ...
    
    // Use critical sync for real-time performance
    // Synchronize map state
    syncMapState(map_name, host->getAccountId(), true);
    
    // Synchronize player data
    syncPlayerData(sd, map_name, true);
    
    // ... rest of the method ...
}
```

### 2. Host Metrics Synchronization

For non-critical data like host metrics, use regular synchronization:

```cpp
bool P2PHostManager::updateHostMetrics() {
    for (const auto& pair : hosts) {
        auto host = pair.second;
        
        // Create host metrics JSON
        json metrics_json = createHostMetricsJson(host);
        
        // Synchronize host metrics
        std::string operation_id = P2PDataSync::getInstance().syncToMainServer(
            SyncDataType::HOST_METRICS,
            host->getAccountId(),
            "",
            metrics_json.dump(),
            false
        );
    }
    
    return true;
}
```

### 3. Security Validation

Security validation is critical and uses real-time synchronization:

```cpp
bool P2PHost::validateHosting(ValidationType type) {
    // ... validation logic ...
    
    // Synchronize validation result
    P2PMapServer::getInstance().syncSecurityValidation(account_id, map_name, type, result, true);
    
    return result;
}
```

## Performance Considerations

The system is designed for optimal performance:

1. **Synchronous Processing for Critical Data**: Critical game data is processed synchronously with short timeouts to ensure real-time performance.
2. **Asynchronous Processing for Non-Critical Data**: Non-critical synchronization operations are processed asynchronously to avoid blocking the main thread.
3. **Redis Caching**: Frequently accessed data is cached in Redis to reduce load on the main database.
4. **Connection Pooling**: Database connections are pooled for efficient resource usage.
5. **Batched Operations**: Multiple operations can be batched together for efficient processing.
6. **Priority Queue**: Critical operations can be prioritized over less important ones.

## Troubleshooting

### Common Issues

1. **Connection Errors**: If the system cannot connect to Redis or PostgreSQL, check the connection settings in the configuration file.
2. **Synchronization Failures**: If synchronization operations fail, check the error messages in the logs and ensure the main server endpoint is correct.
3. **Performance Issues**: If the system is slow, try increasing the number of worker threads or adjusting the cache expiry settings.

### Logging

The system logs information, warnings, and errors using the rAthena logging system. To enable debug logging, set `debug_mode = 1` in the configuration file.

### Monitoring

The system provides a status method that returns information about the current state of the system:

```cpp
std::string status = rathena::P2PDataSync::getInstance().getStatus();
ShowInfo("%s\n", status.c_str());
```

## Conclusion

The P2P Data Synchronization System provides a secure, high-performance solution for real-time data synchronization between P2P hosts and the main server. By using Redis for caching, pgvector for vector similarity search, and OpenAI/Azure OpenAI for text embedding, the system enables advanced features while maintaining security and performance.