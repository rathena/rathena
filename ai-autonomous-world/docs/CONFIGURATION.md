# Configuration Guide

## Overview

This document provides comprehensive configuration options for the AI Autonomous World system.

**Last Updated:** 2025-11-08

---

## Configuration File

Main configuration file: `ai-service/config.py`

Settings can be overridden using:
1. Environment variables (highest priority)
2. `.env` file
3. YAML config file
4. Default values (lowest priority)

---

## Service Configuration

```python
# Service name
service_name: str = "ai-service"

# Service host (0.0.0.0 = all interfaces)
service_host: str = "0.0.0.0"

# Service port
service_port: int = 8000

# Environment: development, staging, production
environment: str = "development"

# Debug mode
debug: bool = True
```

---

## Database Configuration

### DragonflyDB (High-Speed Caching)

```python
# DragonFlyDB host (Redis protocol)
dragonfly_host: str = "127.0.0.1"

# DragonFlyDB port
dragonfly_port: int = 6379

# DragonFlyDB database number
dragonfly_db: int = 0

# DragonFlyDB password (optional)
dragonfly_password: Optional[str] = None

# Maximum connections
dragonfly_max_connections: int = 50
```

### PostgreSQL 17 (Persistent Storage)

```python
# PostgreSQL host
postgres_host: str = "localhost"

# PostgreSQL port
postgres_port: int = 5432

# Database name
postgres_db: str = "ai_world_memory"

# Database user
postgres_user: str = "ai_world_user"

# Database password
postgres_password: str = "ai_world_pass_2025"

# Connection pool size
postgres_pool_size: int = 10

# Maximum overflow connections
postgres_max_overflow: int = 20

# Echo SQL queries (debug mode)
postgres_echo_sql: bool = False

# Connection retry configuration
db_connection_max_retries: int = 5
db_connection_retry_delay: float = 2.0
```

**Required PostgreSQL Extensions:**
- pgvector - Vector similarity search
- TimescaleDB - Time-series data
- Apache AGE - Graph database capabilities

**Database Schema:**
- **18 tables total** (7 AI-specific + 11 rAthena integration)
- Automated setup via `setup-database.sh` script
- All tables created and indexed automatically

**Database Setup:**
```bash
# Use the automated setup script (recommended)
cd /ai-mmorpg-world/rathena-AI-world
./setup-database.sh
```

---

## LLM Provider Configuration

**Important:** All LLM provider configurations are loaded from YAML files in `config/` directory.
These YAML files use environment variable placeholders in the format `${VARIABLE_NAME}`.
Actual values must be set in the `.env` file.

### Default Provider

```python
# Default LLM provider: azure_openai, openai, deepseek, anthropic, google
default_llm_provider: str = "azure_openai"
```

**5 Providers Configured:**
1. Azure OpenAI (Primary, Production)
2. OpenAI (Fallback)
3. Anthropic Claude (Alternative)
4. Google Gemini (Alternative)
5. DeepSeek (Alternative)

### Azure OpenAI (Primary - Production Ready)

**Example Configuration:**
```bash
AZURE_OPENAI_API_KEY=your-azure-openai-api-key-here
AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com
AZURE_OPENAI_DEPLOYMENT_NAME=gpt-4
AZURE_OPENAI_API_VERSION=2024-08-01-preview
AZURE_OPENAI_EMBEDDING_DEPLOYMENT=text-embedding-ada-002
```

**Configuration File:** `config/llm/azure_openai.yaml`

```python
# Azure OpenAI API key (REQUIRED)
azure_openai_api_key: str = "${AZURE_OPENAI_API_KEY}"

# Azure OpenAI endpoint (REQUIRED)
azure_openai_endpoint: str = "${AZURE_OPENAI_ENDPOINT}"

# Deployment name (REQUIRED)
azure_openai_deployment: str = "${AZURE_OPENAI_DEPLOYMENT_NAME}"

# API version
azure_openai_api_version: str = "${AZURE_OPENAI_API_VERSION}"

# Embedding deployment (for vector search)
azure_openai_embedding_deployment: str = "${AZURE_OPENAI_EMBEDDING_DEPLOYMENT}"
```

### OpenAI

```python
# OpenAI API key
openai_api_key: str = "your-api-key"

# Model name
openai_model: str = "gpt-4"

# Temperature (0.0-2.0)
openai_temperature: float = 0.7

# Max tokens
openai_max_tokens: int = 2000
```

### Anthropic Claude

**Current Configuration:**
```bash
ANTHROPIC_API_KEY=<configured>
ANTHROPIC_MODEL=claude-3-5-sonnet-20241022
```

```python
# Anthropic API key
anthropic_api_key: str = "your-api-key"

# Model name
anthropic_model: str = "claude-3-5-sonnet-20241022"

# Temperature
anthropic_temperature: float = 0.7

# Max tokens
anthropic_max_tokens: int = 2000
```

### Google Gemini

```python
# Google API key
google_api_key: str = "your-api-key"

# Model name
google_model: str = "gemini-pro"

# Temperature
google_temperature: float = 0.7

# Max tokens
google_max_tokens: int = 2000
```

---

## NPC Movement Configuration

### Basic Movement

```python
# Enable NPC movement
npc_movement_enabled: bool = True

# Update interval (seconds)
npc_movement_update_interval: int = 5

# Maximum movement distance (cells)
npc_movement_max_distance: int = 10

# Personality influence (0.0-1.0)
npc_movement_personality_influence: float = 0.7

# Require walking sprite
npc_movement_require_walking_sprite: bool = True
```

### Advanced Movement Features

```python
# Map data integration
npc_movement_map_data_integration: bool = False

# Pathfinding enabled
npc_movement_pathfinding_enabled: bool = False

# Pathfinding algorithm: astar, dijkstra
npc_movement_pathfinding_algorithm: str = "astar"

# Social movement (follow/approach)
npc_movement_social_enabled: bool = False

# Social follow distance (cells)
npc_movement_social_follow_distance: int = 3

# Goal-directed movement
npc_movement_goal_directed_enabled: bool = False

# Dynamic patrol routes
npc_movement_dynamic_patrol_enabled: bool = False

# Animation sync
npc_movement_animation_sync_enabled: bool = False

# Collision avoidance
npc_movement_collision_avoidance_enabled: bool = False

# Collision detection radius (cells)
npc_movement_collision_detection_radius: int = 2

# Time-based movement patterns
npc_movement_time_based_enabled: bool = False
```

---

## Free-Form Text Input Configuration

```python
# Enable free-form text input
freeform_text_enabled: bool = True

# Input mode: chat_command, client_ui, web_interface
freeform_text_mode: str = "chat_command"

# Maximum message length (characters)
freeform_text_max_length: int = 500

# Enable rate limiting
freeform_text_rate_limit_enabled: bool = True

# Messages per minute
freeform_text_rate_limit_messages: int = 10

# Enable profanity filter
freeform_text_profanity_filter_enabled: bool = False

# Response timeout (seconds)
freeform_text_response_timeout: int = 30

# Fallback mode: show_error, use_buttons, use_cached
freeform_text_fallback_mode: str = "show_error"
```

---

## Chat Command Configuration

```python
# Enable chat command interface
chat_command_enabled: bool = True

# Command prefix
chat_command_prefix: str = "@npc"

# Fallback mode
chat_command_fallback_mode: str = "show_error"

# Maximum message length
chat_command_max_length: int = 500

# Cooldown (seconds)
chat_command_cooldown: int = 2

# Require NPC proximity
chat_command_require_npc_proximity: bool = True

# Proximity range (cells)
chat_command_proximity_range: int = 5

# Log all interactions
chat_command_log_all_interactions: bool = True
```

---

## Environment Variables

Override any setting using environment variables. See `ai-service/.env.example` for complete list.

```bash
# Service
export SERVICE_NAME="ai-service"
export SERVICE_HOST="0.0.0.0"
export SERVICE_PORT=8000
export SERVICE_ENV="development"
export LOG_LEVEL="DEBUG"

# DragonFlyDB (Redis protocol)
export DRAGONFLY_HOST="127.0.0.1"
export DRAGONFLY_PORT=6379
export DRAGONFLY_PASSWORD=""
export DRAGONFLY_DB=0
export DRAGONFLY_MAX_CONNECTIONS=50

# (Deprecated) REDIS_* variables are supported for backward compatibility only.
# Use DRAGONFLY_* for all new deployments.

# PostgreSQL
export POSTGRES_HOST="localhost"
export POSTGRES_PORT=5432
export POSTGRES_DB="ai_world_memory"
export POSTGRES_USER="ai_world_user"
export POSTGRES_PASSWORD="ai_world_pass_2025"
export POSTGRES_POOL_SIZE=10
export POSTGRES_MAX_OVERFLOW=20
export POSTGRES_ECHO_SQL=false

# LLM Provider
export PRIMARY_LLM_PROVIDER="azure_openai"

# Azure OpenAI (Recommended)
export AZURE_OPENAI_API_KEY="your-key"
export AZURE_OPENAI_ENDPOINT="https://your-resource.openai.azure.com"
export AZURE_OPENAI_API_VERSION="2024-02-15-preview"
export AZURE_OPENAI_DEPLOYMENT_NAME="gpt-4"

# OpenAI (Alternative)
export OPENAI_API_KEY="your-key"
export OPENAI_MODEL="gpt-4-turbo-preview"

# Anthropic Claude
export ANTHROPIC_API_KEY="your-key"
export CLAUDE_MODEL="claude-3-opus-20240229"

# Google Gemini
export GOOGLE_API_KEY="your-key"
export GEMINI_MODEL="gemini-pro"

# DeepSeek
export DEEPSEEK_API_KEY="your-key"
export DEEPSEEK_MODEL="deepseek-chat"

# Movement
export NPC_MOVEMENT_ENABLED=true
export NPC_MOVEMENT_PATHFINDING_ENABLED=true

# Free-Form Text
export FREEFORM_TEXT_ENABLED=true
export CHAT_COMMAND_PREFIX="@talk"

# Rate Limiting
export RATE_LIMIT_ENABLED=true
export RATE_LIMIT_REQUESTS=100
export RATE_LIMIT_PERIOD=60
```

---

## GPU Acceleration Configuration

### Master GPU Settings

```python
# Enable GPU acceleration (master switch)
gpu_enabled: bool = False

# GPU device selection: cuda, mps, rocm, auto
gpu_device: str = "auto"

# Maximum GPU memory usage (0.0-1.0)
gpu_memory_fraction: float = 0.8

# Dynamic GPU memory allocation
gpu_allow_growth: bool = True

# Fallback to CPU if GPU unavailable
gpu_fallback_to_cpu: bool = True

# Log GPU memory usage
gpu_log_memory_usage: bool = True
```

### GPU Feature Flags

```python
# Enable GPU for LLM inference
llm_use_gpu: bool = True

# Enable GPU for pathfinding (experimental)
pathfinding_use_gpu: bool = False

# Enable GPU for vector operations
vector_search_use_gpu: bool = True

# Enable GPU for batch processing
batch_processing_use_gpu: bool = True
```

### GPU Performance Tuning

```python
# Batch size for GPU operations
gpu_batch_size: int = 32

# Maximum context length for GPU inference
gpu_max_context_length: int = 4096

# Number of CPU threads for GPU operations
gpu_num_threads: int = 4

# Number of batches to prefetch
gpu_prefetch_batches: int = 2
```

### GPU Library Configuration

```python
# Use PyTorch for GPU operations
gpu_use_pytorch: bool = True

# Use CuPy for NumPy-like GPU operations
gpu_use_cupy: bool = False

# Use FAISS-GPU for vector search
gpu_use_faiss_gpu: bool = True

# Use vLLM for LLM inference acceleration
gpu_use_vllm: bool = False

# Use TensorRT-LLM for inference
gpu_use_tensorrt: bool = False
```

### Environment Variables

```bash
# GPU Master Settings
export GPU_ENABLED=true
export GPU_DEVICE=auto
export GPU_MEMORY_FRACTION=0.8
export GPU_ALLOW_GROWTH=true
export GPU_FALLBACK_TO_CPU=true

# GPU Features
export LLM_USE_GPU=true
export VECTOR_SEARCH_USE_GPU=true
export PATHFINDING_USE_GPU=false

# GPU Performance
export GPU_BATCH_SIZE=32
export GPU_MAX_CONTEXT_LENGTH=4096

# GPU Libraries
export GPU_USE_PYTORCH=true
export GPU_USE_FAISS_GPU=true
export GPU_USE_VLLM=false
```

---

**Last Updated**: 2025-11-06
**Version**: 1.0.0

