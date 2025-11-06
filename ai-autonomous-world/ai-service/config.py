"""
Configuration management for AI Service
Loads settings from YAML config file and environment variables
"""

from pathlib import Path
from typing import Optional, Dict, Any
from pydantic_settings import BaseSettings
from pydantic import Field
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
    
    # DragonflyDB Configuration
    redis_host: str = Field(default="127.0.0.1", env="REDIS_HOST")
    redis_port: int = Field(default=6379, env="REDIS_PORT")
    redis_db: int = Field(default=0, env="REDIS_DB")
    redis_password: Optional[str] = Field(default=None, env="REDIS_PASSWORD")
    redis_max_connections: int = Field(default=50, env="REDIS_MAX_CONNECTIONS")
    
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
    
    # Azure OpenAI Configuration
    azure_openai_api_key: Optional[str] = Field(default=None, env="AZURE_OPENAI_API_KEY")
    azure_openai_endpoint: Optional[str] = Field(default=None, env="AZURE_OPENAI_ENDPOINT")
    azure_openai_deployment: str = Field(default="gpt-4", env="AZURE_OPENAI_DEPLOYMENT")
    azure_openai_api_version: str = Field(default="2024-02-15-preview", env="AZURE_OPENAI_API_VERSION")
    
    # CrewAI Configuration
    crewai_verbose: bool = Field(default=True, env="CREWAI_VERBOSE")
    crewai_max_iterations: int = Field(default=15, env="CREWAI_MAX_ITERATIONS")
    
    # Memori SDK Configuration
    memori_enabled: bool = Field(default=True, env="MEMORI_ENABLED")
    memori_api_key: Optional[str] = Field(default=None, env="MEMORI_API_KEY")
    
    # API Security
    api_key: Optional[str] = Field(default=None, env="API_KEY")
    cors_origins: list = Field(default=["*"], env="CORS_ORIGINS")
    
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
    logger.info(f"Redis: {settings.redis_host}:{settings.redis_port}")
    
    return settings


# Global settings instance
settings = get_settings()

