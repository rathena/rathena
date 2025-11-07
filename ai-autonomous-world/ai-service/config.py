"""
Configuration management for AI Service
Loads settings from YAML config file and environment variables
"""

import re
from pathlib import Path
from typing import Optional, Dict, Any, List
from pydantic_settings import BaseSettings
from pydantic import Field, field_validator
import yaml
from loguru import logger


class Settings(BaseSettings):
    """Application settings with environment variable support"""
    
    # Service Configuration
    service_name: str = Field(default="ai-service", env="SERVICE_NAME")
    service_host: str = Field(default="0.0.0.0", env="SERVICE_HOST")
    service_port: int = Field(default=8000, env="SERVICE_PORT")
    environment: str = Field(default="development", env="ENVIRONMENT")
    debug: bool = Field(default=True, env="DEBUG")
    
    # DragonflyDB Configuration (for caching and real-time state)
    redis_host: str = Field(default="127.0.0.1", env="REDIS_HOST")
    redis_port: int = Field(default=6379, env="REDIS_PORT")
    redis_db: int = Field(default=0, env="REDIS_DB")
    redis_password: Optional[str] = Field(default=None, env="REDIS_PASSWORD")
    redis_max_connections: int = Field(default=50, env="REDIS_MAX_CONNECTIONS")

    # PostgreSQL Configuration (for persistent memory storage via Memori SDK)
    postgres_host: str = Field(default="localhost", env="POSTGRES_HOST")
    postgres_port: int = Field(default=5432, env="POSTGRES_PORT")
    postgres_db: str = Field(default="ai_world_memory", env="POSTGRES_DB")
    postgres_user: str = Field(default="ai_world_user", env="POSTGRES_USER")
    postgres_password: str = Field(default="ai_world_pass_2025", env="POSTGRES_PASSWORD")
    postgres_pool_size: int = Field(default=10, env="POSTGRES_POOL_SIZE")
    postgres_max_overflow: int = Field(default=20, env="POSTGRES_MAX_OVERFLOW")
    postgres_echo_sql: bool = Field(default=False, env="POSTGRES_ECHO_SQL")

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
    
    # Memori SDK Configuration
    memori_enabled: bool = Field(default=True, env="MEMORI_ENABLED")
    memori_api_key: Optional[str] = Field(default=None, env="MEMORI_API_KEY")
    
    # API Security
    api_key: Optional[str] = Field(default=None, env="API_KEY")
    cors_origins: List[str] = Field(
        default=["http://localhost:8888", "http://127.0.0.1:8888"],
        env="CORS_ORIGINS",
        description="CORS allowed origins - restrict to known domains for security"
    )
    
    # Logging Configuration
    log_level: str = Field(default="INFO", env="LOG_LEVEL")
    log_file: str = Field(default="logs/ai-service.log", env="LOG_FILE")
    
    # Performance Configuration
    max_concurrent_requests: int = Field(default=100, env="MAX_CONCURRENT_REQUESTS")
    request_timeout: int = Field(default=300, env="REQUEST_TIMEOUT")

    # NPC Movement Configuration
    npc_movement_enabled: bool = Field(default=True, env="NPC_MOVEMENT_ENABLED")
    npc_movement_update_interval: int = Field(default=5, env="NPC_MOVEMENT_UPDATE_INTERVAL")  # seconds
    npc_movement_max_distance: int = Field(default=10, env="NPC_MOVEMENT_MAX_DISTANCE")  # cells
    npc_movement_personality_influence: float = Field(default=0.7, env="NPC_MOVEMENT_PERSONALITY_INFLUENCE")  # 0.0-1.0
    npc_movement_require_walking_sprite: bool = Field(default=True, env="NPC_MOVEMENT_REQUIRE_WALKING_SPRITE")

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

    @property
    def postgres_connection_string(self) -> str:
        """
        Get PostgreSQL connection string for SQLAlchemy

        Returns:
            PostgreSQL connection string in format:
            postgresql+psycopg2://user:password@host:port/database
        """
        return (
            f"postgresql+psycopg2://{self.postgres_user}:{self.postgres_password}"
            f"@{self.postgres_host}:{self.postgres_port}/{self.postgres_db}"
        )

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        case_sensitive = False


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

    # Log NPC Movement Configuration
    logger.info(f"NPC Movement: Enabled={settings.npc_movement_enabled}")
    if settings.npc_movement_enabled:
        logger.info(f"  - Update Interval: {settings.npc_movement_update_interval}s")
        logger.info(f"  - Max Distance: {settings.npc_movement_max_distance} cells")
        logger.info(f"  - Advanced Features: Pathfinding={settings.npc_movement_pathfinding_enabled}, "
                   f"Social={settings.npc_movement_social_enabled}, "
                   f"Collision={settings.npc_movement_collision_avoidance_enabled}")

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


# Global settings instance
settings = get_settings()

