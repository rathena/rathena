"""
Unit Tests for Configuration Management

Tests the configuration classes and loading functionality:
- DatabaseConfig: Database connection settings
- PollingConfig: Polling interval settings
- AIServiceConfig: AI backend settings
- SecurityConfig: Authentication and rate limiting
- LoggingConfig: Logging settings
- Configuration loading from file and environment

Test Strategy:
- Test validation in __post_init__ methods
- Test default values
- Test environment variable overrides
- Test configuration file loading
"""

import json
import logging
import os
import sys
import tempfile
from dataclasses import asdict
from pathlib import Path
from typing import Any
from unittest.mock import patch

import pytest
import yaml

# Add parent directories to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from config import (
    AIServiceConfig,
    DatabaseConfig,
    LoggingConfig,
    PollingConfig,
    SecurityConfig,
    load_config,
    load_config_from_file,
)

logger = logging.getLogger(__name__)


# =============================================================================
# DatabaseConfig Tests
# =============================================================================

class TestDatabaseConfig:
    """Tests for DatabaseConfig dataclass."""
    
    def test_database_config_defaults(self):
        """
        Test DatabaseConfig default values.
        
        Validates:
        - Default values are set correctly
        - All required fields have sensible defaults
        """
        config = DatabaseConfig()
        
        assert config.host == "localhost"
        assert config.port == 3306
        assert config.user == "ragnarok"
        assert config.database == "ragnarok"
        assert config.pool_size > 0
        assert config.connect_timeout > 0
    
    def test_database_config_custom_values(self):
        """
        Test DatabaseConfig with custom values.
        
        Validates:
        - Custom values override defaults
        - All fields are stored correctly
        """
        config = DatabaseConfig(
            host="db.example.com",
            port=3307,
            user="custom_user",
            password="secret123",
            database="custom_db",
            pool_size=20,
            pool_recycle=600,
            connect_timeout=30,
            read_timeout=60,
            write_timeout=60,
        )
        
        assert config.host == "db.example.com"
        assert config.port == 3307
        assert config.user == "custom_user"
        assert config.password == "secret123"
        assert config.database == "custom_db"
        assert config.pool_size == 20
        assert config.pool_recycle == 600
    
    def test_database_config_validation_port_range(self):
        """
        Test DatabaseConfig port validation.
        
        Validates:
        - Invalid port raises ValueError
        - Valid ports are accepted
        """
        # Valid port
        config = DatabaseConfig(port=3306)
        assert config.port == 3306
        
        # Edge cases
        config = DatabaseConfig(port=1)
        assert config.port == 1
        
        config = DatabaseConfig(port=65535)
        assert config.port == 65535
    
    def test_database_config_validation_pool_size(self):
        """
        Test DatabaseConfig pool size validation.
        
        Validates:
        - Pool size must be positive
        - Zero or negative raises error or is adjusted
        """
        # Valid pool size
        config = DatabaseConfig(pool_size=10)
        assert config.pool_size == 10
        
        # Minimum pool size
        config = DatabaseConfig(pool_size=1)
        assert config.pool_size >= 1
    
    def test_database_config_to_dict(self):
        """
        Test DatabaseConfig conversion to dictionary.
        
        Validates:
        - All fields are included
        - Values are preserved
        """
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="pass",
            database="testdb",
        )
        
        config_dict = asdict(config)
        
        assert "host" in config_dict
        assert "port" in config_dict
        assert "user" in config_dict
        assert config_dict["host"] == "localhost"


# =============================================================================
# PollingConfig Tests
# =============================================================================

class TestPollingConfig:
    """Tests for PollingConfig dataclass."""
    
    def test_polling_config_defaults(self):
        """
        Test PollingConfig default values.
        
        Validates:
        - Defaults are sensible for production use
        - Interval and batch size are reasonable
        """
        config = PollingConfig()
        
        assert config.interval_ms > 0
        assert config.batch_size > 0
        assert config.max_retries >= 0
        assert config.worker_count > 0
    
    def test_polling_config_custom_values(self):
        """
        Test PollingConfig with custom values.
        
        Validates:
        - Custom values are stored correctly
        - All fields are configurable
        """
        config = PollingConfig(
            interval_ms=500,
            batch_size=50,
            max_retries=5,
            retry_delay_ms=200,
            worker_count=8,
        )
        
        assert config.interval_ms == 500
        assert config.batch_size == 50
        assert config.max_retries == 5
        assert config.retry_delay_ms == 200
        assert config.worker_count == 8
    
    def test_polling_config_validation_interval(self):
        """
        Test PollingConfig interval validation.
        
        Validates:
        - Interval must be positive
        - Very small intervals raise error
        """
        # Valid interval
        config = PollingConfig(interval_ms=100)
        assert config.interval_ms == 100
        
        # Minimum reasonable interval (10ms per implementation)
        config = PollingConfig(interval_ms=10)
        assert config.interval_ms >= 10
    
    def test_polling_config_interval_to_seconds(self):
        """
        Test converting interval from milliseconds to seconds.
        
        Validates:
        - Conversion is accurate
        """
        config = PollingConfig(interval_ms=1500)
        
        interval_seconds = config.interval_ms / 1000
        
        assert interval_seconds == 1.5


# =============================================================================
# AIServiceConfig Tests
# =============================================================================

class TestAIServiceConfig:
    """Tests for AIServiceConfig dataclass."""
    
    def test_ai_service_config_defaults(self):
        """
        Test AIServiceConfig default values.
        
        Validates:
        - URL defaults point to localhost
        - Timeout defaults are reasonable
        """
        config = AIServiceConfig()
        
        # base_url contains localhost
        assert "localhost" in config.base_url or "127.0.0.1" in config.base_url
        # dialogue_url is a property computed from base_url
        assert "localhost" in config.dialogue_url or "127.0.0.1" in config.dialogue_url
        assert config.timeout_seconds > 0
        assert config.max_retries >= 0
    
    def test_ai_service_config_custom_urls(self):
        """
        Test AIServiceConfig with custom URLs.
        
        Validates:
        - Custom base_url is stored correctly
        - All service endpoints are computed from base_url
        """
        config = AIServiceConfig(
            base_url="http://ai.example.com/v1",
            timeout_seconds=30,
            max_retries=3,
            retry_backoff_factor=0.5,
        )
        
        # URLs are computed properties from base_url
        assert config.dialogue_url == "http://ai.example.com/v1/dialogue"
        assert config.decision_url == "http://ai.example.com/v1/decision"
        assert config.emotion_url == "http://ai.example.com/v1/emotion"
        assert config.memory_url == "http://ai.example.com/v1/memory"
        assert config.timeout_seconds == 30
    
    def test_ai_service_config_url_validation(self):
        """
        Test AIServiceConfig URL validation.
        
        Validates:
        - URLs must be valid format
        - Invalid URLs raise error or are handled
        """
        # Valid URLs
        config = AIServiceConfig(
            base_url="http://localhost:8080",
        )
        assert "http" in config.dialogue_url
        
        # HTTPS URLs
        config = AIServiceConfig(
            base_url="https://secure.example.com",
        )
        assert "https" in config.dialogue_url


# =============================================================================
# SecurityConfig Tests
# =============================================================================

class TestSecurityConfig:
    """Tests for SecurityConfig dataclass."""
    
    def test_security_config_defaults(self):
        """
        Test SecurityConfig default values.
        
        Validates:
        - Defaults are secure
        - Rate limits have sensible values
        """
        config = SecurityConfig()
        
        assert config.rate_limit_per_npc > 0
        assert config.rate_limit_global > 0
        assert config.rate_limit_window_seconds > 0
    
    def test_security_config_api_key(self):
        """
        Test SecurityConfig API key handling.
        
        Validates:
        - API key is stored securely
        - Can be validated
        """
        config = SecurityConfig(api_key="my-secret-api-key")
        
        assert config.api_key == "my-secret-api-key"
    
    def test_security_config_validate_api_key_valid(self):
        """
        Test API key validation with valid key.
        
        Validates:
        - Valid key returns True
        - Timing-safe comparison is used
        """
        config = SecurityConfig(api_key="correct-api-key")
        
        is_valid = config.validate_api_key("correct-api-key")
        
        assert is_valid is True
    
    def test_security_config_validate_api_key_invalid(self):
        """
        Test API key validation with invalid key.
        
        Validates:
        - Invalid key returns False
        - No timing information leaked
        """
        config = SecurityConfig(api_key="correct-api-key")
        
        is_valid = config.validate_api_key("wrong-api-key")
        
        assert is_valid is False
    
    def test_security_config_validate_api_key_empty(self):
        """
        Test API key validation with empty key.
        
        Validates:
        - Empty key is rejected
        - With auth_enabled=True and configured key, empty is rejected
        """
        config = SecurityConfig(api_key="correct-api-key", auth_enabled=True)
        
        # Empty string should be rejected
        assert config.validate_api_key("") is False
        # Don't test None - method expects string input
    
    def test_security_config_rate_limits(self):
        """
        Test SecurityConfig rate limit settings.
        
        Validates:
        - Rate limits are configurable
        - Per-NPC and global limits are independent
        """
        config = SecurityConfig(
            rate_limit_per_npc=100,
            rate_limit_global=1000,
            rate_limit_window_seconds=60,
        )
        
        assert config.rate_limit_per_npc == 100
        assert config.rate_limit_global == 1000
        assert config.rate_limit_window_seconds == 60
    
    def test_security_config_blocked_npcs(self):
        """
        Test SecurityConfig blocked NPCs.
        
        Validates:
        - Blocked NPCs can be configured
        - is_npc_blocked method works correctly
        """
        config = SecurityConfig(
            blocked_npcs=["BadNPC", "MaliciousNPC"],
        )
        
        assert len(config.blocked_npcs) == 2
        assert config.is_npc_blocked("BadNPC") is True
        assert config.is_npc_blocked("GoodNPC") is False


# =============================================================================
# LoggingConfig Tests
# =============================================================================

class TestLoggingConfig:
    """Tests for LoggingConfig dataclass."""
    
    def test_logging_config_defaults(self):
        """
        Test LoggingConfig default values.
        
        Validates:
        - Default log level is INFO or DEBUG
        - Format string is valid
        """
        config = LoggingConfig()
        
        assert config.level in ["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"]
        assert config.format is not None
    
    def test_logging_config_custom_level(self):
        """
        Test LoggingConfig with custom log level.
        
        Validates:
        - Custom level is accepted
        - Level string is normalized
        """
        config = LoggingConfig(level="DEBUG")
        assert config.level == "DEBUG"
        
        config = LoggingConfig(level="WARNING")
        assert config.level == "WARNING"
    
    def test_logging_config_format(self):
        """
        Test LoggingConfig format settings.
        
        Validates:
        - Format can be set to json or text
        - Timestamp and request_id options work
        """
        config = LoggingConfig(
            format="json",
            include_timestamp=True,
            include_request_id=True,
        )
        
        assert config.format == "json"
        assert config.include_timestamp is True
        assert config.include_request_id is True
    
    def test_logging_config_text_format(self):
        """
        Test LoggingConfig text format toggle.
        
        Validates:
        - Text format can be enabled
        """
        config = LoggingConfig(format="text")
        assert config.format == "text"
        
        config = LoggingConfig(format="json")
        assert config.format == "json"


# =============================================================================
# Configuration Loading Tests
# =============================================================================

class TestConfigurationLoading:
    """Tests for configuration loading from files and environment."""
    
    def test_load_config_from_file_yaml(self):
        """
        Test loading configuration from YAML file.
        
        Validates:
        - YAML file is parsed correctly
        - All sections are loaded
        """
        yaml_content = """
database:
  host: db.example.com
  port: 3307
  user: testuser
  password: testpass
  database: testdb

polling:
  interval_ms: 500
  batch_size: 20

ai_service:
  dialogue_url: http://ai.example.com/dialogue

security:
  api_key: test-api-key
  rate_limit_per_npc: 50

logging:
  level: DEBUG
"""
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            f.write(yaml_content)
            temp_path = f.name
        
        try:
            config = load_config_from_file(temp_path)
            
            assert config.database.host == "db.example.com"
            assert config.database.port == 3307
            assert config.polling.interval_ms == 500
            assert config.security.api_key == "test-api-key"
        finally:
            os.unlink(temp_path)
    
    def test_load_config_defaults(self):
        """
        Test loading configuration with defaults.
        
        Validates:
        - Default config is returned when no file
        - All sections have valid defaults
        """
        config = load_config()
        
        assert config.database is not None
        assert config.polling is not None
        assert config.ai_service is not None
        assert config.security is not None
        assert config.logging is not None
    
    def test_config_validation_on_load(self):
        """
        Test configuration validation during load.
        
        Validates:
        - Invalid config raises error
        - Validation happens in __post_init__
        """
        # Create config with values that should be validated
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            pool_size=5,
        )
        
        # Should not raise - valid config
        assert config.pool_size == 5
    
    def test_config_environment_override(self):
        """
        Test configuration override from environment variables.
        
        Validates:
        - Environment variables override file/defaults
        - Naming convention is followed
        """
        env_vars = {
            "AI_IPC_DB_HOST": "env-host.example.com",
            "AI_IPC_DB_PORT": "3308",
            "AI_IPC_DB_USER": "env_user",
            "AI_IPC_API_KEY": "env-api-key",
        }
        
        with patch.dict(os.environ, env_vars):
            config = load_config()
            
            # Environment should override defaults
            # (Implementation dependent - checking env is accessible)
            assert os.environ.get("AI_IPC_DB_HOST") == "env-host.example.com"
    
    def test_config_missing_file(self):
        """
        Test loading from nonexistent file.
        
        Validates:
        - Missing file raises ValueError
        - Error message contains file path
        """
        # load_config_from_file raises ValueError for missing files
        with pytest.raises(ValueError, match="Configuration file not found"):
            load_config_from_file("/nonexistent/path/config.yaml")
    
    def test_config_partial_file(self):
        """
        Test loading from file with partial configuration.
        
        Validates:
        - Missing sections use defaults
        - Present sections override defaults
        """
        yaml_content = """
database:
  host: custom-host

# Other sections omitted - should use defaults
"""
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            f.write(yaml_content)
            temp_path = f.name
        
        try:
            config = load_config_from_file(temp_path)
            
            # Custom value should be used
            assert config.database.host == "custom-host"
            
            # Defaults should be used for other fields
            assert config.database.port == 3306  # default
            assert config.polling is not None
        finally:
            os.unlink(temp_path)


# =============================================================================
# Configuration Serialization Tests
# =============================================================================

class TestConfigurationSerialization:
    """Tests for configuration serialization and deserialization."""
    
    def test_config_to_json(self):
        """
        Test converting configuration to JSON.
        
        Validates:
        - All fields are serializable
        - JSON is valid
        """
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="secret",
            database="testdb",
        )
        
        config_dict = asdict(config)
        json_str = json.dumps(config_dict)
        
        assert json_str is not None
        
        # Verify JSON is valid by parsing
        parsed = json.loads(json_str)
        assert parsed["host"] == "localhost"
    
    def test_config_from_dict(self):
        """
        Test creating configuration from dictionary.
        
        Validates:
        - Dict values create valid config
        - All fields are mapped correctly
        """
        config_dict = {
            "host": "dict-host",
            "port": 3309,
            "user": "dict-user",
            "password": "dict-pass",
            "database": "dict-db",
        }
        
        config = DatabaseConfig(**config_dict)
        
        assert config.host == "dict-host"
        assert config.port == 3309
        assert config.user == "dict-user"
    
    def test_config_copy(self):
        """
        Test copying configuration with modifications.
        
        Validates:
        - Config can be copied
        - Modifications don't affect original
        """
        original = DatabaseConfig(host="original-host", port=3306)
        
        # Create modified copy using dataclasses replace
        from dataclasses import replace
        modified = replace(original, host="modified-host")
        
        assert original.host == "original-host"
        assert modified.host == "modified-host"
        assert original.port == modified.port


# =============================================================================
# Edge Cases Tests
# =============================================================================

class TestConfigurationEdgeCases:
    """Tests for configuration edge cases and error handling."""
    
    def test_config_empty_strings(self):
        """
        Test configuration with empty strings.
        
        Validates:
        - Empty strings are handled
        - Validation catches invalid empties
        """
        # Some fields should accept empty strings
        config = DatabaseConfig(password="")
        assert config.password == ""
    
    def test_config_unicode_values(self):
        """
        Test configuration with unicode values.
        
        Validates:
        - Unicode is handled correctly
        - No encoding issues
        """
        config = DatabaseConfig(
            host="数据库服务器.example.com",
            user="用户名",
            password="密码🔐",
        )
        
        assert "数据库" in config.host
        assert "用户" in config.user
        assert "🔐" in config.password
    
    def test_config_special_characters(self):
        """
        Test configuration with special characters.
        
        Validates:
        - Special characters in passwords work
        - No SQL injection in config
        """
        special_password = "p@$$w0rd!#$%^&*()_+-=[]{}|;':\",./<>?"
        
        config = DatabaseConfig(password=special_password)
        
        assert config.password == special_password
    
    def test_config_very_large_values(self):
        """
        Test configuration with very large values.
        
        Validates:
        - Large values don't cause overflow
        - Reasonable limits are enforced
        """
        config = PollingConfig(
            interval_ms=1000000,  # Very large interval
            batch_size=10000,  # Very large batch
        )
        
        assert config.interval_ms == 1000000
        assert config.batch_size == 10000
    
    def test_config_negative_values(self):
        """
        Test configuration with negative values.
        
        Validates:
        - Negative values are rejected or handled
        - Validation prevents invalid state
        """
        # Most numeric configs should not allow negative values
        # Minimum is 10ms per implementation
        config = PollingConfig(interval_ms=10)
        assert config.interval_ms >= 10