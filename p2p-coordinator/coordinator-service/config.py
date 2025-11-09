"""
P2P Coordinator Service Configuration

Production-grade configuration management using Pydantic v2 Settings.
"""

from typing import List
from pydantic import Field, field_validator
from pydantic_settings import BaseSettings, SettingsConfigDict
from loguru import logger


class ServiceConfig(BaseSettings):
    """Service-level configuration"""

    host: str = Field(default="192.168.0.100", alias="P2P_SERVICE_HOST")
    port: int = Field(default=8001, alias="P2P_SERVICE_PORT")
    env: str = Field(default="development", alias="P2P_SERVICE_ENV")
    name: str = Field(default="p2p-coordinator", alias="P2P_SERVICE_NAME")
    version: str = Field(default="1.0.0", alias="P2P_SERVICE_VERSION")
    
    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class DatabaseConfig(BaseSettings):
    """Database configuration for PostgreSQL and DragonflyDB (Redis-compatible)"""

    # DragonflyDB Configuration (Redis-compatible protocol)
    redis_host: str = Field(default="192.168.0.100", alias="REDIS_HOST")
    redis_port: int = Field(default=6379, alias="REDIS_PORT")
    redis_db: int = Field(default=1, alias="REDIS_DB")
    redis_password: str = Field(default="", alias="REDIS_PASSWORD")

    # PostgreSQL Configuration
    postgres_host: str = Field(default="192.168.0.100", alias="POSTGRES_HOST")
    postgres_port: int = Field(default=5432, alias="POSTGRES_PORT")
    postgres_db: str = Field(default="p2p_coordinator", alias="POSTGRES_DB")
    postgres_user: str = Field(default="p2p_user", alias="POSTGRES_USER")
    postgres_password: str = Field(default="p2p_pass_2025", alias="POSTGRES_PASSWORD")

    @property
    def postgres_url(self) -> str:
        """Generate PostgreSQL connection URL"""
        return f"postgresql+asyncpg://{self.postgres_user}:{self.postgres_password}@{self.postgres_host}:{self.postgres_port}/{self.postgres_db}"

    @property
    def redis_url(self) -> str:
        """Generate DragonflyDB connection URL (using Redis protocol)"""
        if self.redis_password:
            return f"redis://:{self.redis_password}@{self.redis_host}:{self.redis_port}/{self.redis_db}"
        return f"redis://{self.redis_host}:{self.redis_port}/{self.redis_db}"
    
    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class WebRTCConfig(BaseSettings):
    """WebRTC configuration for P2P connections"""
    
    stun_server: str = Field(default="stun:stun.l.google.com:19302", alias="STUN_SERVER")
    turn_server: str = Field(default="", alias="TURN_SERVER", description="TURN server URL (optional, leave empty if not using TURN)")
    turn_username: str = Field(default="", alias="TURN_USERNAME", description="TURN username (required if using TURN)")
    turn_password: str = Field(default="", alias="TURN_PASSWORD", description="TURN password (required if using TURN)")
    
    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class HostRequirementsConfig(BaseSettings):
    """Minimum requirements for P2P hosts"""
    
    min_cpu_cores: int = Field(default=4, alias="MIN_CPU_CORES")
    min_cpu_frequency_mhz: int = Field(default=3000, alias="MIN_CPU_FREQUENCY_MHZ")
    min_memory_mb: int = Field(default=8192, alias="MIN_MEMORY_MB")
    min_network_speed_mbps: int = Field(default=100, alias="MIN_NETWORK_SPEED_MBPS")
    max_latency_ms: int = Field(default=100, alias="MAX_LATENCY_MS")
    
    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class P2PFeaturesConfig(BaseSettings):
    """P2P feature flags and settings"""
    
    enable_p2p: bool = Field(default=True, alias="ENABLE_P2P")
    p2p_enabled_zones: str = Field(default="prontera,geffen,payon", alias="P2P_ENABLED_ZONES")
    p2p_fallback_enabled: bool = Field(default=True, alias="P2P_FALLBACK_ENABLED")
    p2p_max_players_per_host: int = Field(default=50, alias="P2P_MAX_PLAYERS_PER_HOST")
    p2p_session_timeout_seconds: int = Field(default=300, alias="P2P_SESSION_TIMEOUT_SECONDS")
    p2p_host_health_check_interval_seconds: int = Field(default=30, alias="P2P_HOST_HEALTH_CHECK_INTERVAL_SECONDS")
    
    @property
    def enabled_zones_list(self) -> List[str]:
        """Parse comma-separated zones into list"""
        return [zone.strip() for zone in self.p2p_enabled_zones.split(",") if zone.strip()]
    
    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class AIServiceConfig(BaseSettings):
    """AI Service integration configuration"""

    ai_service_url: str = Field(default="http://192.168.0.100:8000", alias="AI_SERVICE_URL")
    ai_service_api_key: str = Field(default="your-api-key-here", alias="AI_SERVICE_API_KEY")
    ai_service_enabled: bool = Field(default=True, alias="AI_SERVICE_ENABLED")

    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class MonitoringConfig(BaseSettings):
    """Monitoring and logging configuration"""

    prometheus_enabled: bool = Field(default=True, alias="PROMETHEUS_ENABLED")
    prometheus_port: int = Field(default=9091, alias="PROMETHEUS_PORT")
    log_level: str = Field(default="INFO", alias="LOG_LEVEL")
    log_format: str = Field(default="json", alias="LOG_FORMAT")

    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class SecurityConfig(BaseSettings):
    """Security configuration"""

    # API Key Settings
    api_key_header: str = Field(default="X-API-Key", alias="API_KEY_HEADER")
    coordinator_api_key: str = Field(..., alias="COORDINATOR_API_KEY", description="REQUIRED: Coordinator API key (set via environment variable)")
    api_key_validation_enabled: bool = Field(default=True, alias="API_KEY_VALIDATION_ENABLED")

    # CORS Settings
    cors_origins: str = Field(default="http://192.168.0.100:3000,http://192.168.0.100:8000,http://192.168.0.100:8001,http://localhost:3000,http://localhost:8000,http://localhost:8001", alias="CORS_ORIGINS")

    # JWT Settings
    jwt_secret_key: str = Field(..., alias="JWT_SECRET_KEY", description="REQUIRED: JWT secret key (set via environment variable)")
    jwt_algorithm: str = Field(default="HS256", alias="JWT_ALGORITHM")
    jwt_expiration_seconds: int = Field(default=3600, alias="JWT_EXPIRATION_SECONDS")  # 1 hour
    jwt_validation_enabled: bool = Field(default=True, alias="JWT_VALIDATION_ENABLED")

    @property
    def cors_origins_list(self) -> List[str]:
        """Parse comma-separated CORS origins into list"""
        return [origin.strip() for origin in self.cors_origins.split(",") if origin.strip()]

    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )


class Settings(BaseSettings):
    """Main settings class aggregating all configuration"""

    service: ServiceConfig = Field(default_factory=ServiceConfig)
    database: DatabaseConfig = Field(default_factory=DatabaseConfig)
    webrtc: WebRTCConfig = Field(default_factory=WebRTCConfig)
    host_requirements: HostRequirementsConfig = Field(default_factory=HostRequirementsConfig)
    p2p_features: P2PFeaturesConfig = Field(default_factory=P2PFeaturesConfig)
    ai_service: AIServiceConfig = Field(default_factory=AIServiceConfig)
    monitoring: MonitoringConfig = Field(default_factory=MonitoringConfig)
    security: SecurityConfig = Field(default_factory=SecurityConfig)

    model_config = SettingsConfigDict(
        env_file="../.env",
        env_file_encoding="utf-8",
        case_sensitive=False,
        extra="ignore"
    )

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Validate security settings in production
        if self.service.env == "production":
            self._validate_production_security()

        logger.info(f"Configuration loaded for environment: {self.service.env}")
        logger.info(f"P2P enabled: {self.p2p_features.enable_p2p}")
        logger.info(f"Enabled zones: {self.p2p_features.enabled_zones_list}")

    def _validate_production_security(self):
        """Validate that security settings are properly configured for production"""
        errors = []

        # Check JWT secret key
        if not self.security.jwt_secret_key or len(self.security.jwt_secret_key) < 32:
            errors.append("JWT_SECRET_KEY must be at least 32 characters long")

        if "CHANGE_THIS" in self.security.jwt_secret_key.upper() or \
           "your-secret" in self.security.jwt_secret_key.lower():
            errors.append("JWT_SECRET_KEY must be changed from default value")

        # Check coordinator API key
        if not self.security.coordinator_api_key or len(self.security.coordinator_api_key) < 32:
            errors.append("COORDINATOR_API_KEY must be at least 32 characters long")

        if "CHANGE_THIS" in self.security.coordinator_api_key.upper() or \
           "your-coordinator" in self.security.coordinator_api_key.lower():
            errors.append("COORDINATOR_API_KEY must be changed from default value")

        # Check that validation is enabled
        if not self.security.api_key_validation_enabled:
            logger.warning("API key validation is disabled in production - this is insecure!")

        if not self.security.jwt_validation_enabled:
            logger.warning("JWT validation is disabled in production - this is insecure!")

        if errors:
            error_msg = "Production security validation failed:\n" + "\n".join(f"  - {e}" for e in errors)
            logger.error(error_msg)
            raise ValueError(error_msg)


# Global settings instance
settings = Settings()

