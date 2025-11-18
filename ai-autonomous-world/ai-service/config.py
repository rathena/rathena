"""
Configuration management for AI Service
Loads settings from YAML config file and environment variables

Note: All cache and real-time state configuration should reference DragonFlyDB (not Redis) as the backend.
"""

import re
from pathlib import Path
from typing import Optional, Dict, Any, List, Union
from pydantic_settings import BaseSettings
from pydantic import Field, field_validator
import yaml
from loguru import logger


class Settings(BaseSettings):
    """Application settings with environment variable support"""
    
    # Service Configuration
    service_name: str = Field(default="ai-service", env="SERVICE_NAME")
    service_host: str = Field(default="192.168.0.100", env="SERVICE_HOST")
    service_port: int = Field(default=8000, env="SERVICE_PORT")
    environment: str = Field(default="development", env="ENVIRONMENT")
    debug: bool = Field(default=True, env="DEBUG")
    # Agent Mode Selection (per-agent, comma-separated, e.g. "dialogue:LLM,decision:hybrid,quest:LLM,memory:CPU,world:hybrid,economy:CPU,faction:hybrid")
    agent_modes: str = Field(default="dialogue:LLM,decision:hybrid,quest:LLM,memory:CPU,world:hybrid,economy:CPU,faction:hybrid", env="AGENT_MODES")

    # rAthena Bridge Configuration (for pushing commands back to game server)

    # DragonflyDB Configuration (for caching and real-time state)
    redis_host: str = Field(default="192.168.0.100", env="REDIS_HOST")
    redis_port: int = Field(default=6379, env="REDIS_PORT")
    redis_db: int = Field(default=0, env="REDIS_DB")
    redis_password: Optional[str] = Field(default=None, env="REDIS_PASSWORD")
    redis_max_connections: int = Field(default=50, env="REDIS_MAX_CONNECTIONS")

    # PostgreSQL Configuration (for persistent memory storage via OpenMemory SDK)
    postgres_host: str = Field(default="192.168.0.100", env="POSTGRES_HOST")
    postgres_port: int = Field(default=5432, env="POSTGRES_PORT")
    postgres_db: str = Field(default="ai_world_memory", env="POSTGRES_DB")
    postgres_user: str = Field(default="ai_world_user", env="POSTGRES_USER")
    postgres_password: str = Field(default="ai_world_pass_2025", env="POSTGRES_PASSWORD")
    postgres_pool_size: int = Field(default=10, env="POSTGRES_POOL_SIZE")
    postgres_max_overflow: int = Field(default=20, env="POSTGRES_MAX_OVERFLOW")
    postgres_echo_sql: bool = Field(default=False, env="POSTGRES_ECHO_SQL")

    # OpenMemory Configuration (for long-term persistent memory)
    openmemory_url: str = Field(default="http://localhost:8081", env="OPENMEMORY_URL")
    openmemory_api_key: str = Field(default="", env="OPENMEMORY_API_KEY")

    # Database connection retry configuration
    db_connection_max_retries: int = Field(default=5, env="DB_CONNECTION_MAX_RETRIES")
    db_connection_retry_delay: float = Field(default=2.0, env="DB_CONNECTION_RETRY_DELAY")
    
    # LLM Provider Configuration
    default_llm_provider: str = Field(default="azure_openai", env="DEFAULT_LLM_PROVIDER")
    
    # OpenAI Configuration
    openai_api_key: Optional[str] = Field(default=None, env="OPENAI_API_KEY")
    openai_model: str = Field(default="gpt-4", env="OPENAI_MODEL")
    openai_temperature: float = Field(default=0.7, env="OPENAI_TEMPERATURE")
    openai_max_tokens: int = Field(default=2000, env="OPENAI_MAX_TOKENS")
    
    # Anthropic Configuration
    anthropic_api_key: Optional[str] = Field(default=None, env="ANTHROPIC_API_KEY")
    anthropic_model: str = Field(default="claude-3-sonnet-20240229", env="ANTHROPIC_MODEL")
    anthropic_temperature: float = Field(default=0.7, env="ANTHROPIC_TEMPERATURE")
    anthropic_max_tokens: int = Field(default=2000, env="ANTHROPIC_MAX_TOKENS")
    
    # Google Gemini Configuration
    google_api_key: Optional[str] = Field(default=None, env="GOOGLE_API_KEY")
    google_model: str = Field(default="gemini-pro", env="GOOGLE_MODEL")
    google_temperature: float = Field(default=0.7, env="GOOGLE_TEMPERATURE")
    google_max_tokens: int = Field(default=2000, env="GOOGLE_MAX_TOKENS")

    # DeepSeek Configuration
    deepseek_api_key: Optional[str] = Field(default=None, env="DEEPSEEK_API_KEY")
    deepseek_base_url: str = Field(default="https://api.deepseek.com", env="DEEPSEEK_BASE_URL")
    deepseek_model: str = Field(default="deepseek-chat", env="DEEPSEEK_MODEL")
    deepseek_temperature: float = Field(default=0.7, env="DEEPSEEK_TEMPERATURE")
    deepseek_max_tokens: int = Field(default=2000, env="DEEPSEEK_MAX_TOKENS")

    # Azure OpenAI Configuration
    azure_openai_api_key: Optional[str] = Field(default=None, env="AZURE_OPENAI_API_KEY")
    azure_openai_endpoint: Optional[str] = Field(default=None, env="AZURE_OPENAI_ENDPOINT")
    azure_openai_deployment: str = Field(default="gpt-4", env="AZURE_OPENAI_DEPLOYMENT")
    azure_openai_api_version: str = Field(default="2024-02-15-preview", env="AZURE_OPENAI_API_VERSION")

    @field_validator('azure_openai_endpoint')
    @classmethod
    def validate_azure_endpoint(cls, v: Optional[str]) -> Optional[str]:
        """Validate Azure OpenAI endpoint format"""
        if v is None:
            return v

        # Azure OpenAI endpoint format: https://{resource}.openai.azure.com/
        pattern = r'^https://[a-zA-Z0-9-]+\.openai\.azure\.com/?$'
        if not re.match(pattern, v):
            raise ValueError(
                f"Invalid Azure OpenAI endpoint format: {v}. "
                "Expected format: https://{{resource}}.openai.azure.com/"
            )

        return v.rstrip('/')  # Remove trailing slash for consistency

    # CrewAI Configuration
    crewai_verbose: bool = Field(default=True, env="CREWAI_VERBOSE")
    crewai_max_iterations: int = Field(default=15, env="CREWAI_MAX_ITERATIONS")

    # API Security
    api_key: Optional[str] = Field(default=None, env="API_KEY")
    api_key_header: str = Field(default="X-API-Key", env="API_KEY_HEADER")
    api_key_required: bool = Field(default=False, env="API_KEY_REQUIRED")
    cors_origins: Union[List[str], str] = Field(
        default=["http://192.168.0.100:8888", "http://localhost:8888", "http://127.0.0.1:8888"],
        env="CORS_ORIGINS",
        description="CORS allowed origins - restrict to known domains for security"
    )

    @field_validator('cors_origins', mode='before')
    @classmethod
    def parse_cors_origins(cls, v):
        """Parse CORS origins from comma-separated string or list"""
        if isinstance(v, str):
            if not v or v.strip() == '':
                return ["http://192.168.0.100:8888", "http://localhost:8888", "http://127.0.0.1:8888"]
            return [origin.strip() for origin in v.split(',') if origin.strip()]
        return v

    # Rate Limiting
    rate_limit_enabled: bool = Field(default=True, env="RATE_LIMIT_ENABLED")
    rate_limit_requests: int = Field(default=100, env="RATE_LIMIT_REQUESTS")
    rate_limit_period: int = Field(default=60, env="RATE_LIMIT_PERIOD")  # seconds

    # Request Size Limits
    max_request_size: int = Field(default=10485760, env="MAX_REQUEST_SIZE")  # 10MB in bytes
    
    # Logging Configuration
    log_level: str = Field(default="INFO", env="LOG_LEVEL")
    log_file: str = Field(default="logs/ai-service.log", env="LOG_FILE")
    
    # Performance Configuration
    max_concurrent_requests: int = Field(default=100, env="MAX_CONCURRENT_REQUESTS")
    request_timeout: int = Field(default=300, env="REQUEST_TIMEOUT")

    # ============================================================================
    # AUTONOMOUS FEATURES CONFIGURATION
    # ============================================================================

    # Autonomous NPC Movement Configuration
    npc_movement_enabled: bool = Field(default=True, env="NPC_MOVEMENT_ENABLED")
    npc_movement_mode: str = Field(
        default="event_driven",
        env="NPC_MOVEMENT_MODE",
        description="Movement trigger mode: 'event_driven' (on player interaction/idle), 'fixed_interval' (periodic), 'disabled'"
    )
    npc_movement_update_interval: int = Field(
        default=60,
        env="NPC_MOVEMENT_UPDATE_INTERVAL",
        description="Interval in seconds for fixed_interval mode (default 60s, not 5s to reduce LLM calls)"
    )
    npc_movement_idle_timeout: int = Field(
        default=60,
        env="NPC_MOVEMENT_IDLE_TIMEOUT",
        description="Seconds of inactivity before NPC considers moving (event_driven mode)"
    )
    npc_movement_chain_enabled: bool = Field(
        default=True,
        env="NPC_MOVEMENT_CHAIN_ENABLED",
        description="Allow NPCs to plan multi-step movement chains"
    )
    npc_movement_max_distance: int = Field(default=10, env="NPC_MOVEMENT_MAX_DISTANCE")  # cells
    npc_movement_personality_influence: float = Field(default=0.7, env="NPC_MOVEMENT_PERSONALITY_INFLUENCE")  # 0.0-1.0
    npc_movement_require_walking_sprite: bool = Field(default=True, env="NPC_MOVEMENT_REQUIRE_WALKING_SPRITE")
    npc_adaptive_behavior: bool = Field(
        default=True,
        env="NPC_ADAPTIVE_BEHAVIOR",
        description="NPCs learn and adapt vs follow fixed patterns"
    )

    # Advanced Movement Features
    npc_movement_map_data_integration: bool = Field(default=False, env="NPC_MOVEMENT_MAP_DATA_INTEGRATION")
    npc_movement_pathfinding_enabled: bool = Field(default=False, env="NPC_MOVEMENT_PATHFINDING_ENABLED")
    npc_movement_pathfinding_algorithm: str = Field(default="astar", env="NPC_MOVEMENT_PATHFINDING_ALGORITHM")  # astar, dijkstra
    npc_movement_social_enabled: bool = Field(default=False, env="NPC_MOVEMENT_SOCIAL_ENABLED")
    npc_movement_social_follow_distance: int = Field(default=3, env="NPC_MOVEMENT_SOCIAL_FOLLOW_DISTANCE")  # cells
    npc_movement_goal_directed_enabled: bool = Field(default=False, env="NPC_MOVEMENT_GOAL_DIRECTED_ENABLED")
    npc_movement_dynamic_patrol_enabled: bool = Field(default=False, env="NPC_MOVEMENT_DYNAMIC_PATROL_ENABLED")
    npc_movement_animation_sync_enabled: bool = Field(default=False, env="NPC_MOVEMENT_ANIMATION_SYNC_ENABLED")
    npc_movement_collision_avoidance_enabled: bool = Field(default=False, env="NPC_MOVEMENT_COLLISION_AVOIDANCE_ENABLED")
    npc_movement_collision_detection_radius: int = Field(default=2, env="NPC_MOVEMENT_COLLISION_DETECTION_RADIUS")  # cells
    npc_movement_time_based_enabled: bool = Field(default=False, env="NPC_MOVEMENT_TIME_BASED_ENABLED")

    # Free-Form Text Input Configuration
    freeform_text_enabled: bool = Field(default=True, env="FREEFORM_TEXT_ENABLED")
    freeform_text_mode: str = Field(default="chat_command", env="FREEFORM_TEXT_MODE")  # chat_command, client_ui, web_interface
    freeform_text_max_length: int = Field(default=500, env="FREEFORM_TEXT_MAX_LENGTH")  # characters
    freeform_text_rate_limit_enabled: bool = Field(default=True, env="FREEFORM_TEXT_RATE_LIMIT_ENABLED")
    freeform_text_rate_limit_messages: int = Field(default=10, env="FREEFORM_TEXT_RATE_LIMIT_MESSAGES")  # messages per minute
    freeform_text_profanity_filter_enabled: bool = Field(default=False, env="FREEFORM_TEXT_PROFANITY_FILTER_ENABLED")
    freeform_text_response_timeout: int = Field(default=30, env="FREEFORM_TEXT_RESPONSE_TIMEOUT")  # seconds
    freeform_text_fallback_mode: str = Field(default="show_error", env="FREEFORM_TEXT_FALLBACK_MODE")  # show_error, use_buttons, use_cached

    # Chat Command Interface Configuration
    chat_command_enabled: bool = Field(default=True, env="CHAT_COMMAND_ENABLED")
    chat_command_prefix: str = Field(default="@npc", env="CHAT_COMMAND_PREFIX")
    chat_command_fallback_mode: str = Field(default="show_error", env="CHAT_COMMAND_FALLBACK_MODE")  # show_error, use_buttons, use_cached
    chat_command_max_length: int = Field(default=500, env="CHAT_COMMAND_MAX_LENGTH")  # characters
    chat_command_cooldown: int = Field(default=2, env="CHAT_COMMAND_COOLDOWN")  # seconds
    chat_command_require_npc_proximity: bool = Field(default=True, env="CHAT_COMMAND_REQUIRE_NPC_PROXIMITY")
    chat_command_proximity_range: int = Field(default=5, env="CHAT_COMMAND_PROXIMITY_RANGE")  # cells
    chat_command_log_all_interactions: bool = Field(default=True, env="CHAT_COMMAND_LOG_ALL_INTERACTIONS")

    # GPU Acceleration Configuration
    gpu_enabled: bool = Field(default=False, env="GPU_ENABLED")  # Master switch for GPU acceleration
    gpu_device: str = Field(default="auto", env="GPU_DEVICE")  # cuda, mps, rocm, auto
    gpu_memory_fraction: float = Field(default=0.8, env="GPU_MEMORY_FRACTION")  # 0.0-1.0, max GPU memory usage
    gpu_allow_growth: bool = Field(default=True, env="GPU_ALLOW_GROWTH")  # Dynamic GPU memory allocation
    gpu_fallback_to_cpu: bool = Field(default=True, env="GPU_FALLBACK_TO_CPU")  # Fallback to CPU if GPU unavailable
    gpu_log_memory_usage: bool = Field(default=True, env="GPU_LOG_MEMORY_USAGE")  # Log GPU memory usage

    # GPU Feature Flags
    llm_use_gpu: bool = Field(default=True, env="LLM_USE_GPU")  # Enable GPU for LLM inference (when gpu_enabled=True)
    pathfinding_use_gpu: bool = Field(default=False, env="PATHFINDING_USE_GPU")  # Enable GPU for pathfinding (experimental)
    vector_search_use_gpu: bool = Field(default=True, env="VECTOR_SEARCH_USE_GPU")  # Enable GPU for vector operations
    batch_processing_use_gpu: bool = Field(default=True, env="BATCH_PROCESSING_USE_GPU")  # Enable GPU for batch operations

    # GPU Performance Tuning
    gpu_batch_size: int = Field(default=32, env="GPU_BATCH_SIZE")  # Batch size for GPU operations
    gpu_max_context_length: int = Field(default=4096, env="GPU_MAX_CONTEXT_LENGTH")  # Max context length for GPU inference
    gpu_num_threads: int = Field(default=4, env="GPU_NUM_THREADS")  # Number of CPU threads for GPU operations
    gpu_prefetch_batches: int = Field(default=2, env="GPU_PREFETCH_BATCHES")  # Number of batches to prefetch

    # GPU Library Configuration
    gpu_use_pytorch: bool = Field(default=True, env="GPU_USE_PYTORCH")  # Use PyTorch for GPU operations
    gpu_use_cupy: bool = Field(default=False, env="GPU_USE_CUPY")  # Use CuPy for NumPy-like GPU operations
    gpu_use_faiss_gpu: bool = Field(default=True, env="GPU_USE_FAISS_GPU")  # Use FAISS-GPU for vector search
    gpu_use_vllm: bool = Field(default=False, env="GPU_USE_VLLM")  # Use vLLM for LLM inference acceleration
    gpu_use_tensorrt: bool = Field(default=False, env="GPU_USE_TENSORRT")  # Use TensorRT-LLM for inference

    # Economic Simulation Configuration
    economy_enabled: bool = Field(default=True, env="ECONOMY_ENABLED")
    economy_update_mode: str = Field(
        default="daily",
        env="ECONOMY_UPDATE_MODE",
        description="Economy update mode: 'daily' (once per day), 'fixed_interval' (periodic), 'event_driven' (on trades), 'disabled'"
    )
    economy_update_interval: int = Field(
        default=86400,
        env="ECONOMY_UPDATE_INTERVAL",
        description="Interval in seconds for fixed_interval mode (default 86400 = 24 hours)"
    )
    economy_adaptive_pricing: bool = Field(
        default=True,
        env="ECONOMY_ADAPTIVE_PRICING",
        description="Dynamic pricing based on supply/demand vs fixed prices"
    )
    economy_learning_enabled: bool = Field(
        default=True,
        env="ECONOMY_LEARNING_ENABLED",
        description="EconomyAgent learns from past decisions and adapts"
    )
    economy_inflation_enabled: bool = Field(default=True, env="ECONOMY_INFLATION_ENABLED")
    economy_supply_demand_enabled: bool = Field(default=True, env="ECONOMY_SUPPLY_DEMAND_ENABLED")
    economy_events_enabled: bool = Field(default=True, env="ECONOMY_EVENTS_ENABLED")

    # Shop Restocking Configuration
    shop_restock_enabled: bool = Field(default=True, env="SHOP_RESTOCK_ENABLED")
    shop_restock_mode: str = Field(
        default="npc_driven",
        env="SHOP_RESTOCK_MODE",
        description="Restock mode: 'npc_driven' (NPC decides when/what), 'fixed_interval' (periodic), 'disabled'"
    )
    shop_restock_check_interval: int = Field(
        default=86400,
        env="SHOP_RESTOCK_CHECK_INTERVAL",
        description="How often NPC evaluates if restock needed (default 86400 = 24 hours)"
    )
    shop_npc_learning_enabled: bool = Field(
        default=True,
        env="SHOP_NPC_LEARNING_ENABLED",
        description="Shop owner NPCs learn from restocking outcomes"
    )
    shop_default_restock_interval: int = Field(
        default=3600,
        env="SHOP_DEFAULT_RESTOCK_INTERVAL",
        description="Default restock interval for fixed_interval mode (3600 = 1 hour)"
    )

    # Faction System Configuration
    faction_enabled: bool = Field(default=True, env="FACTION_ENABLED")
    faction_reputation_decay_enabled: bool = Field(default=True, env="FACTION_REPUTATION_DECAY_ENABLED")
    faction_reputation_decay_interval: int = Field(
        default=604800,
        env="FACTION_REPUTATION_DECAY_INTERVAL",
        description="Reputation decay check interval (default 604800 = 7 days)"
    )
    faction_dynamic_relationships: bool = Field(
        default=True,
        env="FACTION_DYNAMIC_RELATIONSHIPS",
        description="Faction relationships change based on player/NPC actions"
    )

    # Environment System Configuration
    environment_enabled: bool = Field(default=True, env="ENVIRONMENT_ENABLED")
    environment_update_mode: str = Field(
        default="fixed_interval",
        env="ENVIRONMENT_UPDATE_MODE",
        description="Environment update mode: 'fixed_interval' (periodic), 'real_time' (continuous), 'disabled'"
    )
    environment_update_interval: int = Field(
        default=300,
        env="ENVIRONMENT_UPDATE_INTERVAL",
        description="Interval in seconds for environment updates (default 300 = 5 minutes)"
    )

    # Weather System Configuration
    weather_enabled: bool = Field(default=True, env="WEATHER_ENABLED")
    weather_change_probability: float = Field(
        default=0.1,
        env="WEATHER_CHANGE_PROBABILITY",
        description="Probability of weather change per update cycle (0.0-1.0)"
    )
    weather_types: List[str] = Field(
        default=["clear", "sunny", "cloudy", "rainy", "stormy", "snowy", "foggy"],
        env="WEATHER_TYPES",
        description="Available weather types"
    )

    # Time of Day Cycle Configuration
    time_of_day_enabled: bool = Field(default=True, env="TIME_OF_DAY_ENABLED")
    time_of_day_cycle_duration: int = Field(
        default=1440,
        env="TIME_OF_DAY_CYCLE_DURATION",
        description="Duration of full day cycle in minutes (default 1440 = 24 hours real-time)"
    )
    time_of_day_phases: List[str] = Field(
        default=["dawn", "day", "dusk", "night"],
        env="TIME_OF_DAY_PHASES",
        description="Time of day phases"
    )

    # Season System Configuration
    season_enabled: bool = Field(default=True, env="SEASON_ENABLED")
    season_length_days: int = Field(
        default=30,
        env="SEASON_LENGTH_DAYS",
        description="Length of each season in game days (default 30 days)"
    )
    season_types: List[str] = Field(
        default=["spring", "summer", "autumn", "winter"],
        env="SEASON_TYPES",
        description="Available seasons"
    )

    # Disaster System Configuration
    disaster_enabled: bool = Field(default=True, env="DISASTER_ENABLED")
    disaster_probability: float = Field(
        default=0.01,
        env="DISASTER_PROBABILITY",
        description="Probability of disaster occurrence per update cycle (0.0-1.0)"
    )
    disaster_types: List[str] = Field(
        default=["earthquake", "flood", "drought", "plague", "wildfire", "meteor"],
        env="DISASTER_TYPES",
        description="Available disaster types"
    )
    disaster_duration_min: int = Field(
        default=300,
        env="DISASTER_DURATION_MIN",
        description="Minimum disaster duration in seconds (default 300 = 5 minutes)"
    )
    disaster_duration_max: int = Field(
        default=3600,
        env="DISASTER_DURATION_MAX",
        description="Maximum disaster duration in seconds (default 3600 = 1 hour)"
    )

    # Resource Availability Configuration
    resource_availability_enabled: bool = Field(default=True, env="RESOURCE_AVAILABILITY_ENABLED")
    resource_types: List[str] = Field(
        default=["wood", "stone", "ore", "herbs", "fish", "crops"],
        env="RESOURCE_TYPES",
        description="Available resource types"
    )
    resource_regeneration_rate: float = Field(
        default=0.05,
        env="RESOURCE_REGENERATION_RATE",
        description="Resource regeneration rate per update cycle (0.0-1.0)"
    )
    resource_depletion_rate: float = Field(
        default=0.02,
        env="RESOURCE_DEPLETION_RATE",
        description="Resource depletion rate from harvesting (0.0-1.0)"
    )

    # Agent Learning and Memory Configuration
    agent_learning_enabled: bool = Field(
        default=True,
        env="AGENT_LEARNING_ENABLED",
        description="All agents learn from historical data and adapt"
    )
    agent_memory_retention_days: int = Field(
        default=30,
        env="AGENT_MEMORY_RETENTION_DAYS",
        description="How long to keep decision history in days"
    )
    agent_decision_logging_enabled: bool = Field(
        default=True,
        env="AGENT_DECISION_LOGGING_ENABLED",
        description="Log all agent decisions for analysis and learning"
    )
    agent_context_window_size: int = Field(
        default=10,
        env="AGENT_CONTEXT_WINDOW_SIZE",
        description="Number of past decisions to include in agent context"
    )

    # Phase 1: NPC-to-NPC Interaction Configuration
    npc_to_npc_interactions_enabled: bool = Field(
        default=True,
        env="NPC_TO_NPC_INTERACTIONS_ENABLED",
        description="Enable NPC-to-NPC interactions and relationship building"
    )
    npc_interaction_range: int = Field(
        default=5,
        env="NPC_INTERACTION_RANGE",
        description="Range in cells for NPC proximity detection"
    )
    npc_relationship_enabled: bool = Field(
        default=True,
        env="NPC_RELATIONSHIP_ENABLED",
        description="Enable NPC-to-NPC relationship tracking (friendship, rivalry)"
    )
    npc_information_sharing_enabled: bool = Field(
        default=True,
        env="NPC_INFORMATION_SHARING_ENABLED",
        description="Enable NPCs to share information (gossip, rumors, quest hints)"
    )
    npc_proximity_check_interval: int = Field(
        default=30,
        env="NPC_PROXIMITY_CHECK_INTERVAL",
        description="Interval in seconds to check for nearby NPCs"
    )
    npc_relationship_decay_rate: float = Field(
        default=0.01,
        env="NPC_RELATIONSHIP_DECAY_RATE",
        description="Daily decay rate for NPC relationships (0.01 = 1% per day)"
    )

    # Phase 2: Instant Response System Configuration
    instant_response_enabled: bool = Field(
        default=True,
        env="INSTANT_RESPONSE_ENABLED",
        description="Enable instant/immediate LLM responses for high-priority events"
    )
    instant_response_events: list[str] = Field(
        default=["combat", "urgent_quest", "special_event", "player_death", "boss_spawn"],
        env="INSTANT_RESPONSE_EVENTS",
        description="Event types that trigger instant responses (comma-separated)"
    )
    event_priority_levels: dict[str, str] = Field(
        default={
            "combat": "instant",
            "urgent_quest": "instant",
            "special_event": "instant",
            "player_interaction": "high",
            "npc_interaction": "normal",
            "idle_timeout": "low"
        },
        description="Event priority mapping (instant/high/normal/low)"
    )
    instant_response_max_concurrent: int = Field(
        default=10,
        env="INSTANT_RESPONSE_MAX_CONCURRENT",
        description="Maximum concurrent instant response LLM calls"
    )

    # Phase 3: Universal Consciousness Configuration
    universal_consciousness_enabled: bool = Field(
        default=True,
        env="UNIVERSAL_CONSCIOUSNESS_ENABLED",
        description="Enable consciousness and decision-making for ALL entities"
    )
    reasoning_depth: str = Field(
        default="deep",
        env="REASONING_DEPTH",
        description="Reasoning depth: shallow (current), medium (past+present), deep (past+present+future)"
    )
    reasoning_history_days: int = Field(
        default=7,
        env="REASONING_HISTORY_DAYS",
        description="Number of days of historical data to include in reasoning"
    )
    future_planning_enabled: bool = Field(
        default=True,
        env="FUTURE_PLANNING_ENABLED",
        description="Enable multi-step future planning in decision making"
    )
    future_planning_steps: int = Field(
        default=3,
        env="FUTURE_PLANNING_STEPS",
        description="Number of future steps to plan ahead"
    )
    reasoning_chain_logging_enabled: bool = Field(
        default=True,
        env="REASONING_CHAIN_LOGGING_ENABLED",
        description="Log full reasoning chains for all decisions"
    )
    world_consciousness_enabled: bool = Field(
        default=True,
        env="WORLD_CONSCIOUSNESS_ENABLED",
        description="Enable world agent consciousness for global events"
    )

    # Phase 4: Advanced LLM Optimization Configuration
    llm_optimization_mode: str = Field(
        default="balanced",
        env="LLM_OPTIMIZATION_MODE",
        description="Optimization mode: aggressive (max caching), balanced, quality (prioritize quality)"
    )
    decision_cache_enabled: bool = Field(
        default=True,
        env="DECISION_CACHE_ENABLED",
        description="Enable caching of similar decisions"
    )
    decision_cache_ttl: int = Field(
        default=3600,
        env="DECISION_CACHE_TTL",
        description="Decision cache TTL in seconds (3600 = 1 hour)"
    )
    decision_batch_enabled: bool = Field(
        default=True,
        env="DECISION_BATCH_ENABLED",
        description="Enable batching of similar decisions"
    )
    decision_batch_size: int = Field(
        default=5,
        env="DECISION_BATCH_SIZE",
        description="Number of decisions to batch together"
    )
    decision_batch_timeout: float = Field(
        default=2.0,
        env="DECISION_BATCH_TIMEOUT",
        description="Maximum time to wait for batch to fill (seconds)"
    )
    decision_complexity_threshold: float = Field(
        default=0.5,
        env="DECISION_COMPLEXITY_THRESHOLD",
        description="Minimum complexity score (0-1) to invoke LLM"
    )
    decision_prefilter_enabled: bool = Field(
        default=True,
        env="DECISION_PREFILTER_ENABLED",
        description="Enable pre-filtering of unnecessary decisions"
    )
    decision_embedding_similarity_threshold: float = Field(
        default=0.85,
        env="DECISION_EMBEDDING_SIMILARITY_THRESHOLD",
        description="Similarity threshold (0-1) to reuse cached decisions"
    )
    rule_based_decisions_enabled: bool = Field(
        default=True,
        env="RULE_BASED_DECISIONS_ENABLED",
        description="Enable Tier 1 rule-based decisions (no LLM)"
    )

    # Action Validation Configuration
    max_movement_distance: int = Field(default=50, env="MAX_MOVEMENT_DISTANCE", description="Maximum cells NPC can move in single action")
    max_map_size: int = Field(default=1000, env="MAX_MAP_SIZE", description="Maximum map size (width/height)")
    max_npc_level: int = Field(default=999, env="MAX_NPC_LEVEL", description="Maximum NPC level")
    max_npc_name_length: int = Field(default=100, env="MAX_NPC_NAME_LENGTH", description="Maximum NPC name length")

    # LLM Generation Configuration
    dialogue_temperature: float = Field(default=0.8, env="DIALOGUE_TEMPERATURE", description="Temperature for dialogue generation")
    decision_temperature: float = Field(default=0.6, env="DECISION_TEMPERATURE", description="Temperature for decision making")
    max_player_message_length: int = Field(default=500, env="MAX_PLAYER_MESSAGE_LENGTH", description="Maximum player message length")

    # Retry and Timeout Configuration
    llm_timeout: float = Field(default=60.0, env="LLM_TIMEOUT", description="LLM API timeout in seconds")
    llm_max_retries: int = Field(default=3, env="LLM_MAX_RETRIES", description="Maximum LLM API retry attempts")
    db_connection_max_retries: int = Field(default=5, env="DB_CONNECTION_MAX_RETRIES", description="Maximum database connection retries")
    db_connection_retry_delay: float = Field(default=1.0, env="DB_CONNECTION_RETRY_DELAY", description="Initial database retry delay in seconds")

    @field_validator('service_port', 'redis_port', 'postgres_port', 'rathena_bridge_port')
    @classmethod
    def validate_port(cls, v: int, info) -> int:
        """Validate port numbers are in valid range (1-65535)"""
        if not 1 <= v <= 65535:
            raise ValueError(f"{info.field_name} must be between 1 and 65535, got {v}")
        return v

    @field_validator('redis_db')
    @classmethod
    def validate_redis_db(cls, v: int) -> int:
        """Validate Redis DB number is non-negative"""
        if v < 0:
            raise ValueError(f"redis_db must be non-negative, got {v}")
        return v

    @field_validator('gpu_memory_fraction')
    @classmethod
    def validate_gpu_memory_fraction(cls, v: float) -> float:
        """Validate GPU memory fraction is between 0.0 and 1.0"""
        if not 0.0 <= v <= 1.0:
            raise ValueError(f"gpu_memory_fraction must be between 0.0 and 1.0, got {v}")
        return v

    @field_validator('gpu_batch_size', 'gpu_max_context_length', 'gpu_num_threads', 'gpu_prefetch_batches')
    @classmethod
    def validate_positive_int(cls, v: int, info) -> int:
        """Validate positive integer values"""
        if v <= 0:
            raise ValueError(f"{info.field_name} must be positive, got {v}")
        return v

    @field_validator('npc_movement_personality_influence')
    @classmethod
    def validate_personality_influence(cls, v: float) -> float:
        """Validate personality influence is between 0.0 and 1.0"""
        if not 0.0 <= v <= 1.0:
            raise ValueError(f"npc_movement_personality_influence must be between 0.0 and 1.0, got {v}")
        return v

    @field_validator('reasoning_depth')
    @classmethod
    def validate_reasoning_depth(cls, v: str) -> str:
        """Validate reasoning depth is valid"""
        valid_depths = ["shallow", "medium", "deep"]
        if v not in valid_depths:
            raise ValueError(f"reasoning_depth must be one of {valid_depths}, got {v}")
        return v

    @field_validator('llm_optimization_mode')
    @classmethod
    def validate_optimization_mode(cls, v: str) -> str:
        """Validate LLM optimization mode is valid"""
        valid_modes = ["aggressive", "balanced", "quality"]
        if v not in valid_modes:
            raise ValueError(f"llm_optimization_mode must be one of {valid_modes}, got {v}")
        return v

    @field_validator('npc_relationship_decay_rate', 'decision_complexity_threshold', 'decision_embedding_similarity_threshold')
    @classmethod
    def validate_zero_to_one(cls, v: float, info) -> float:
        """Validate value is between 0.0 and 1.0"""
        if not 0.0 <= v <= 1.0:
            raise ValueError(f"{info.field_name} must be between 0.0 and 1.0, got {v}")
        return v

    @field_validator('npc_movement_mode')
    @classmethod
    def validate_npc_movement_mode(cls, v: str) -> str:
        """Validate NPC movement mode"""
        valid_modes = ['event_driven', 'fixed_interval', 'disabled']
        if v not in valid_modes:
            raise ValueError(f"npc_movement_mode must be one of {valid_modes}, got {v}")
        return v

    @field_validator('economy_update_mode')
    @classmethod
    def validate_economy_update_mode(cls, v: str) -> str:
        """Validate economy update mode"""
        valid_modes = ['daily', 'fixed_interval', 'event_driven', 'disabled']
        if v not in valid_modes:
            raise ValueError(f"economy_update_mode must be one of {valid_modes}, got {v}")
        return v

    @field_validator('shop_restock_mode')
    @classmethod
    def validate_shop_restock_mode(cls, v: str) -> str:
        """Validate shop restock mode"""
        valid_modes = ['npc_driven', 'fixed_interval', 'disabled']
        if v not in valid_modes:
            raise ValueError(f"shop_restock_mode must be one of {valid_modes}, got {v}")
        return v

    @property
    def postgres_connection_string(self) -> str:
        """
        Get PostgreSQL connection string for SQLAlchemy

        Returns:
            PostgreSQL connection string in format:
            postgresql+psycopg2://user:password@host:port/database
        """
        return (
            f"postgresql+asyncpg://{self.postgres_user}:{self.postgres_password}"
            f"@{self.postgres_host}:{self.postgres_port}/{self.postgres_db}"
        )

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        case_sensitive = False
        extra = "ignore"  # Allow extra fields from .env that aren't defined in Settings
        # 'fields' config removed for Pydantic v2 compatibility


def load_yaml_config(config_path: Optional[str] = None) -> Dict[str, Any]:
    """
    Load configuration from YAML file
    
    Args:
        config_path: Path to YAML config file. If None, uses default path.
        
    Returns:
        Dictionary with configuration data
    """
    if config_path is None:
        # Default config path
        base_dir = Path(__file__).parent.parent
        config_path = base_dir / "config" / "ai-service-config.yaml"
    
    config_file = Path(config_path)
    
    if not config_file.exists():
        logger.warning(f"Config file not found: {config_path}. Using defaults.")
        return {}
    
    try:
        with open(config_file, 'r') as f:
            config_data = yaml.safe_load(f)
            logger.info(f"Loaded configuration from {config_path}")
            return config_data or {}
    except Exception as e:
        logger.error(f"Error loading config file {config_path}: {e}")
        return {}


def get_settings(config_path: Optional[str] = None) -> Settings:
    """
    Get application settings
    
    Priority (highest to lowest):
    1. Environment variables
    2. .env file
    3. YAML config file
    4. Default values
    
    Args:
        config_path: Optional path to YAML config file
        
    Returns:
        Settings instance
    """
    # Load YAML config (lowest priority)
    yaml_config = load_yaml_config(config_path)
    
    # Create settings (env vars and .env file override YAML)
    settings = Settings(**yaml_config)
    
    logger.info(f"Settings loaded: {settings.service_name} on {settings.service_host}:{settings.service_port}")
    logger.info(f"Environment: {settings.environment}, Debug: {settings.debug}")
    logger.info(f"Default LLM Provider: {settings.default_llm_provider}")
    logger.info(f"DragonflyDB (cache): {settings.redis_host}:{settings.redis_port}")
    logger.info(f"PostgreSQL (persistent memory): {settings.postgres_host}:{settings.postgres_port}/{settings.postgres_db}")

    # Log Autonomous Features Configuration
    logger.info("=" * 80)
    logger.info("AUTONOMOUS FEATURES CONFIGURATION")
    logger.info("=" * 80)

    # NPC Movement
    logger.info(f"NPC Movement: Enabled={settings.npc_movement_enabled}")
    if settings.npc_movement_enabled:
        logger.info(f"  - Mode: {settings.npc_movement_mode}")
        logger.info(f"  - Update Interval: {settings.npc_movement_update_interval}s (for fixed_interval mode)")
        logger.info(f"  - Idle Timeout: {settings.npc_movement_idle_timeout}s (for event_driven mode)")
        logger.info(f"  - Chain Movement: {settings.npc_movement_chain_enabled}")
        logger.info(f"  - Adaptive Behavior: {settings.npc_adaptive_behavior}")
        logger.info(f"  - Max Distance: {settings.npc_movement_max_distance} cells")
        logger.info(f"  - Advanced Features: Pathfinding={settings.npc_movement_pathfinding_enabled}, "
                   f"Social={settings.npc_movement_social_enabled}, "
                   f"Collision={settings.npc_movement_collision_avoidance_enabled}")

    # Economic Simulation
    logger.info(f"Economic Simulation: Enabled={settings.economy_enabled}")
    if settings.economy_enabled:
        logger.info(f"  - Mode: {settings.economy_update_mode}")
        logger.info(f"  - Update Interval: {settings.economy_update_interval}s")
        logger.info(f"  - Adaptive Pricing: {settings.economy_adaptive_pricing}")
        logger.info(f"  - Learning Enabled: {settings.economy_learning_enabled}")
        logger.info(f"  - Features: Inflation={settings.economy_inflation_enabled}, "
                   f"Supply/Demand={settings.economy_supply_demand_enabled}, "
                   f"Events={settings.economy_events_enabled}")

    # Shop Restocking
    logger.info(f"Shop Restocking: Enabled={settings.shop_restock_enabled}")
    if settings.shop_restock_enabled:
        logger.info(f"  - Mode: {settings.shop_restock_mode}")
        logger.info(f"  - Check Interval: {settings.shop_restock_check_interval}s")
        logger.info(f"  - NPC Learning: {settings.shop_npc_learning_enabled}")
        logger.info(f"  - Default Restock Interval: {settings.shop_default_restock_interval}s")

    # Faction System
    logger.info(f"Faction System: Enabled={settings.faction_enabled}")
    if settings.faction_enabled:
        logger.info(f"  - Reputation Decay: {settings.faction_reputation_decay_enabled}")
        logger.info(f"  - Decay Interval: {settings.faction_reputation_decay_interval}s")
        logger.info(f"  - Dynamic Relationships: {settings.faction_dynamic_relationships}")

    # Agent Learning
    logger.info(f"Agent Learning: Enabled={settings.agent_learning_enabled}")
    if settings.agent_learning_enabled:
        logger.info(f"  - Memory Retention: {settings.agent_memory_retention_days} days")
        logger.info(f"  - Decision Logging: {settings.agent_decision_logging_enabled}")
        logger.info(f"  - Context Window: {settings.agent_context_window_size} decisions")

    # NPC-to-NPC Interactions
    logger.info(f"NPC-to-NPC Interactions: Enabled={settings.npc_to_npc_interactions_enabled}")
    if settings.npc_to_npc_interactions_enabled:
        logger.info(f"  - Interaction Range: {settings.npc_interaction_range} cells")
        logger.info(f"  - Relationship Tracking: {settings.npc_relationship_enabled}")
        logger.info(f"  - Information Sharing: {settings.npc_information_sharing_enabled}")
        logger.info(f"  - Proximity Check Interval: {settings.npc_proximity_check_interval}s")
        logger.info(f"  - Relationship Decay Rate: {settings.npc_relationship_decay_rate}/day")

    # Instant Response System
    logger.info(f"Instant Response System: Enabled={settings.instant_response_enabled}")
    if settings.instant_response_enabled:
        logger.info(f"  - Instant Events: {', '.join(settings.instant_response_events)}")
        logger.info(f"  - Max Concurrent: {settings.instant_response_max_concurrent}")

    # Universal Consciousness
    logger.info(f"Universal Consciousness: Enabled={settings.universal_consciousness_enabled}")
    if settings.universal_consciousness_enabled:
        logger.info(f"  - Reasoning Depth: {settings.reasoning_depth}")
        logger.info(f"  - History Days: {settings.reasoning_history_days}")
        logger.info(f"  - Future Planning: {settings.future_planning_enabled}")
        if settings.future_planning_enabled:
            logger.info(f"    - Planning Steps: {settings.future_planning_steps}")
        logger.info(f"  - Reasoning Chain Logging: {settings.reasoning_chain_logging_enabled}")
        logger.info(f"  - World Consciousness: {settings.world_consciousness_enabled}")

    # LLM Optimization
    logger.info(f"LLM Optimization Mode: {settings.llm_optimization_mode}")
    logger.info(f"  - Decision Cache: {settings.decision_cache_enabled}")
    if settings.decision_cache_enabled:
        logger.info(f"    - Cache TTL: {settings.decision_cache_ttl}s")
        logger.info(f"    - Similarity Threshold: {settings.decision_embedding_similarity_threshold}")
    logger.info(f"  - Decision Batching: {settings.decision_batch_enabled}")
    if settings.decision_batch_enabled:
        logger.info(f"    - Batch Size: {settings.decision_batch_size}")
        logger.info(f"    - Batch Timeout: {settings.decision_batch_timeout}s")
    logger.info(f"  - Pre-filtering: {settings.decision_prefilter_enabled}")
    logger.info(f"  - Rule-Based Decisions: {settings.rule_based_decisions_enabled}")
    logger.info(f"  - Complexity Threshold: {settings.decision_complexity_threshold}")

    logger.info("=" * 80)

    # Log Free-Form Text Input Configuration
    logger.info(f"Free-Form Text Input: Enabled={settings.freeform_text_enabled}")
    if settings.freeform_text_enabled:
        logger.info(f"  - Mode: {settings.freeform_text_mode}")
        logger.info(f"  - Max Length: {settings.freeform_text_max_length} chars")
        logger.info(f"  - Rate Limiting: {settings.freeform_text_rate_limit_enabled}")
        logger.info(f"  - Response Timeout: {settings.freeform_text_response_timeout}s")

    # Log Chat Command Configuration
    logger.info(f"Chat Command Interface: Enabled={settings.chat_command_enabled}")
    if settings.chat_command_enabled:
        logger.info(f"  - Prefix: {settings.chat_command_prefix}")
        logger.info(f"  - Cooldown: {settings.chat_command_cooldown}s")
        logger.info(f"  - Proximity Required: {settings.chat_command_require_npc_proximity}")

    # Log GPU Configuration
    logger.info(f"GPU Acceleration: Enabled={settings.gpu_enabled}")
    if settings.gpu_enabled:
        logger.info(f"  - Device: {settings.gpu_device}")
        logger.info(f"  - Memory Fraction: {settings.gpu_memory_fraction}")
        logger.info(f"  - Allow Growth: {settings.gpu_allow_growth}")
        logger.info(f"  - Fallback to CPU: {settings.gpu_fallback_to_cpu}")
        logger.info(f"  - Features: LLM={settings.llm_use_gpu}, "
                   f"Vector={settings.vector_search_use_gpu}, "
                   f"Pathfinding={settings.pathfinding_use_gpu}")
        logger.info(f"  - Batch Size: {settings.gpu_batch_size}")
        logger.info(f"  - Libraries: PyTorch={settings.gpu_use_pytorch}, "
                   f"FAISS-GPU={settings.gpu_use_faiss_gpu}, "
                   f"vLLM={settings.gpu_use_vllm}")

    return settings


# Agent Mode Synonyms and Parser
AGENT_MODE_SYNONYMS = {
    "llm": "LLM",
    "llm_only": "LLM",
    "llm-only": "LLM",
    "LLM_ONLY": "LLM",
    "cpu": "CPU",
    "cpu_only": "CPU",
    "cpu-only": "CPU",
    "CPU_ONLY": "CPU",
    "gpu": "GPU",
    "gpu_only": "GPU",
    "gpu-only": "GPU",
    "GPU_ONLY": "GPU",
    "hybrid": "hybrid",
    "llm+cpu": "LLM+CPU",
    "llm+gpu": "LLM+GPU",
    "llm+cpu+gpu": "LLM+CPU+GPU",
    "cpu+gpu": "CPU+GPU",
    "llm_cpu": "LLM+CPU",
    "llm_gpu": "LLM+GPU",
    "llm_cpu_gpu": "LLM+CPU+GPU",
    "cpu_gpu": "CPU+GPU",
}

def parse_agent_modes(agent_modes_str):
    """
    Parse AGENT_MODES string into a dict of agent_name:mode, supporting synonyms and custom/unknown agent types.
    Example: "dialogue:LLM,decision:hybrid,quest:LLM,memory:CPU,world:hybrid,economy:CPU,faction:hybrid"
    """
    agent_modes = {}
    for entry in agent_modes_str.split(","):
        if ":" not in entry:
            continue
        agent, mode = entry.split(":", 1)
        agent = agent.strip().lower()
        mode_key = mode.strip().lower().replace("-", "_")
        canonical_mode = AGENT_MODE_SYNONYMS.get(mode_key, mode.upper())
        agent_modes[agent] = canonical_mode
    return agent_modes
# Global settings instance
settings = get_settings()

