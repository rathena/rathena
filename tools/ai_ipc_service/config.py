"""
Configuration Management for AI IPC Service

Supports loading configuration from:
1. YAML configuration file (default: config.yaml)
2. Environment variables (override YAML values)
3. Default values for missing configuration

Environment Variable Mapping:
    AI_IPC_CONFIG -> Path to YAML config file
    DB_HOST -> database.host
    DB_PORT -> database.port
    DB_USER -> database.user
    DB_PASSWORD -> database.password
    DB_NAME -> database.database
    DB_POOL_SIZE -> database.pool_size
    POLL_INTERVAL_MS -> polling.interval_ms
    BATCH_SIZE -> polling.batch_size
    WORKER_COUNT -> polling.worker_count
    AI_SERVICE_BASE_URL -> ai_service.base_url
    AI_SERVICE_TIMEOUT -> ai_service.timeout_seconds
    LOG_LEVEL -> logging.level
    LOG_FORMAT -> logging.format
"""

from __future__ import annotations

import os
import logging
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml

logger = logging.getLogger(__name__)


@dataclass
class DatabaseConfig:
    """Database connection configuration."""
    host: str = "localhost"
    port: int = 3306
    user: str = "ragnarok"
    password: str = ""
    database: str = "ragnarok"
    pool_size: int = 5
    pool_recycle: int = 3600  # Recycle connections after 1 hour
    connect_timeout: int = 10
    read_timeout: int = 30
    write_timeout: int = 30
    charset: str = "utf8mb4"
    
    def __post_init__(self) -> None:
        """Validate database configuration after initialization."""
        if not self.password:
            logger.warning(
                "Database password is empty. Set DB_PASSWORD environment variable."
            )
        if self.pool_size < 1:
            raise ValueError("Database pool_size must be at least 1")
        if self.port < 1 or self.port > 65535:
            raise ValueError("Database port must be between 1 and 65535")


@dataclass
class PollingConfig:
    """Polling behavior configuration."""
    interval_ms: int = 100  # Polling interval in milliseconds
    batch_size: int = 50    # Number of requests to fetch per poll
    worker_count: int = 4   # Number of concurrent workers
    max_retries: int = 3    # Maximum retries for failed requests
    retry_delay_ms: int = 1000  # Delay between retries
    
    def __post_init__(self) -> None:
        """Validate polling configuration after initialization."""
        if self.interval_ms < 10:
            raise ValueError("Polling interval_ms must be at least 10ms")
        if self.batch_size < 1:
            raise ValueError("Batch size must be at least 1")
        if self.batch_size > 1000:
            logger.warning(
                f"Batch size {self.batch_size} is very large. "
                "Consider reducing for better responsiveness."
            )
        if self.worker_count < 1:
            raise ValueError("Worker count must be at least 1")


@dataclass
class AIServiceConfig:
    """External AI service configuration."""
    base_url: str = "http://localhost:8000"
    timeout_seconds: int = 10
    max_retries: int = 2
    retry_backoff_factor: float = 0.5
    
    def __post_init__(self) -> None:
        """Validate AI service configuration after initialization."""
        if self.timeout_seconds < 1:
            raise ValueError("AI service timeout must be at least 1 second")
        if not self.base_url:
            raise ValueError("AI service base_url is required")


@dataclass 
class LoggingConfig:
    """Logging configuration."""
    level: str = "INFO"
    format: str = "json"  # 'json' or 'text'
    include_timestamp: bool = True
    include_request_id: bool = True
    
    def __post_init__(self) -> None:
        """Validate logging configuration after initialization."""
        valid_levels = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"}
        if self.level.upper() not in valid_levels:
            raise ValueError(
                f"Invalid log level: {self.level}. Must be one of: {valid_levels}"
            )
        valid_formats = {"json", "text"}
        if self.format.lower() not in valid_formats:
            raise ValueError(
                f"Invalid log format: {self.format}. Must be one of: {valid_formats}"
            )


@dataclass
class Config:
    """
    Main configuration class for the AI IPC Service.
    
    Combines all sub-configurations and provides loading from
    YAML files and environment variables.
    """
    database: DatabaseConfig = field(default_factory=DatabaseConfig)
    polling: PollingConfig = field(default_factory=PollingConfig)
    ai_service: AIServiceConfig = field(default_factory=AIServiceConfig)
    logging: LoggingConfig = field(default_factory=LoggingConfig)
    
    # Service metadata
    service_name: str = "ai_ipc_service"
    version: str = "1.0.0"
    
    @classmethod
    def load(cls, config_path: str | Path | None = None) -> Config:
        """
        Load configuration from YAML file and environment variables.
        
        Priority (highest to lowest):
        1. Environment variables
        2. YAML configuration file
        3. Default values
        
        Args:
            config_path: Path to YAML config file. If None, uses AI_IPC_CONFIG
                        environment variable or default 'config.yaml'
        
        Returns:
            Fully initialized Config instance
        """
        # Determine config file path
        if config_path is None:
            config_path = os.getenv("AI_IPC_CONFIG", "config.yaml")
        
        config_path = Path(config_path)
        
        # Load YAML configuration if file exists
        yaml_config: dict[str, Any] = {}
        if config_path.exists():
            logger.info(f"Loading configuration from {config_path}")
            try:
                with open(config_path, "r", encoding="utf-8") as f:
                    yaml_config = yaml.safe_load(f) or {}
            except yaml.YAMLError as e:
                logger.error(f"Failed to parse YAML config: {e}")
                raise ValueError(f"Invalid YAML configuration: {e}") from e
        else:
            logger.info(
                f"Config file {config_path} not found. Using defaults and environment."
            )
        
        # Build configuration with environment overrides
        return cls._build_config(yaml_config)
    
    @classmethod
    def _build_config(cls, yaml_config: dict[str, Any]) -> Config:
        """
        Build configuration from YAML dict with environment overrides.
        
        Args:
            yaml_config: Dictionary from YAML file
            
        Returns:
            Configured Config instance
        """
        # Database configuration
        db_yaml = yaml_config.get("database", {})
        database = DatabaseConfig(
            host=os.getenv("DB_HOST", db_yaml.get("host", "localhost")),
            port=int(os.getenv("DB_PORT", db_yaml.get("port", 3306))),
            user=os.getenv("DB_USER", db_yaml.get("user", "ragnarok")),
            password=os.getenv("DB_PASSWORD", db_yaml.get("password", "")),
            database=os.getenv("DB_NAME", db_yaml.get("database", "ragnarok")),
            pool_size=int(os.getenv("DB_POOL_SIZE", db_yaml.get("pool_size", 5))),
            pool_recycle=int(db_yaml.get("pool_recycle", 3600)),
            connect_timeout=int(db_yaml.get("connect_timeout", 10)),
            read_timeout=int(db_yaml.get("read_timeout", 30)),
            write_timeout=int(db_yaml.get("write_timeout", 30)),
            charset=db_yaml.get("charset", "utf8mb4"),
        )
        
        # Polling configuration
        poll_yaml = yaml_config.get("polling", {})
        polling = PollingConfig(
            interval_ms=int(os.getenv("POLL_INTERVAL_MS", poll_yaml.get("interval_ms", 100))),
            batch_size=int(os.getenv("BATCH_SIZE", poll_yaml.get("batch_size", 50))),
            worker_count=int(os.getenv("WORKER_COUNT", poll_yaml.get("worker_count", 4))),
            max_retries=int(poll_yaml.get("max_retries", 3)),
            retry_delay_ms=int(poll_yaml.get("retry_delay_ms", 1000)),
        )
        
        # AI Service configuration
        ai_yaml = yaml_config.get("ai_service", {})
        ai_service = AIServiceConfig(
            base_url=os.getenv("AI_SERVICE_BASE_URL", ai_yaml.get("base_url", "http://localhost:8000")),
            timeout_seconds=int(os.getenv("AI_SERVICE_TIMEOUT", ai_yaml.get("timeout_seconds", 10))),
            max_retries=int(ai_yaml.get("max_retries", 2)),
            retry_backoff_factor=float(ai_yaml.get("retry_backoff_factor", 0.5)),
        )
        
        # Logging configuration
        log_yaml = yaml_config.get("logging", {})
        logging_config = LoggingConfig(
            level=os.getenv("LOG_LEVEL", log_yaml.get("level", "INFO")),
            format=os.getenv("LOG_FORMAT", log_yaml.get("format", "json")),
            include_timestamp=log_yaml.get("include_timestamp", True),
            include_request_id=log_yaml.get("include_request_id", True),
        )
        
        return cls(
            database=database,
            polling=polling,
            ai_service=ai_service,
            logging=logging_config,
            service_name=yaml_config.get("service_name", "ai_ipc_service"),
            version=yaml_config.get("version", "1.0.0"),
        )
    
    def validate(self) -> list[str]:
        """
        Validate the entire configuration and return any warnings.
        
        Returns:
            List of warning messages (empty if all OK)
        """
        warnings: list[str] = []
        
        # Check database password
        if not self.database.password:
            warnings.append(
                "Database password is not set. "
                "Set DB_PASSWORD environment variable for production."
            )
        
        # Check worker/pool ratio
        if self.polling.worker_count > self.database.pool_size:
            warnings.append(
                f"Worker count ({self.polling.worker_count}) exceeds "
                f"database pool size ({self.database.pool_size}). "
                "Consider increasing pool_size."
            )
        
        # Check AI service URL
        if "localhost" in self.ai_service.base_url:
            warnings.append(
                "AI service base_url points to localhost. "
                "Update for production deployment."
            )
        
        return warnings
    
    def to_dict(self) -> dict[str, Any]:
        """
        Convert configuration to dictionary (for logging/debugging).
        
        Note: Password is masked for security.
        
        Returns:
            Dictionary representation of configuration
        """
        return {
            "service_name": self.service_name,
            "version": self.version,
            "database": {
                "host": self.database.host,
                "port": self.database.port,
                "user": self.database.user,
                "password": "***" if self.database.password else "(not set)",
                "database": self.database.database,
                "pool_size": self.database.pool_size,
            },
            "polling": {
                "interval_ms": self.polling.interval_ms,
                "batch_size": self.polling.batch_size,
                "worker_count": self.polling.worker_count,
                "max_retries": self.polling.max_retries,
            },
            "ai_service": {
                "base_url": self.ai_service.base_url,
                "timeout_seconds": self.ai_service.timeout_seconds,
            },
            "logging": {
                "level": self.logging.level,
                "format": self.logging.format,
            },
        }
    
    def __repr__(self) -> str:
        """Return string representation with masked password."""
        return f"Config({self.to_dict()})"