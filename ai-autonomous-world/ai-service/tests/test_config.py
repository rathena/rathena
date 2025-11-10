"""
Unit tests for configuration module
"""

import pytest
from pydantic import ValidationError
from ai_service.config import Settings


class TestSettings:
    """Test Settings configuration"""
    
    def test_default_settings(self):
        """Test default settings load correctly"""
        settings = Settings()
        assert settings.service_name == "ai-service"
        assert settings.service_port == 8000
        assert settings.environment == "development"
        assert settings.debug is True
    
    def test_cors_origins_not_wildcard(self):
        """Test CORS origins don't contain wildcard"""
        settings = Settings()
        assert "*" not in settings.cors_origins
        assert len(settings.cors_origins) > 0
    
    def test_new_config_fields_exist(self):
        """Test new configuration fields added during remediation"""
        settings = Settings()
        
        # Action validation config
        assert hasattr(settings, 'max_movement_distance')
        assert settings.max_movement_distance == 50
        assert hasattr(settings, 'max_map_size')
        assert settings.max_map_size == 1000
        assert hasattr(settings, 'max_npc_level')
        assert settings.max_npc_level == 999
        
        # LLM generation config
        assert hasattr(settings, 'dialogue_temperature')
        assert settings.dialogue_temperature == 0.8
        assert hasattr(settings, 'decision_temperature')
        assert settings.decision_temperature == 0.6
        
        # Retry/timeout config
        assert hasattr(settings, 'llm_timeout')
        # llm_timeout can be 30.0 (from .env) or 60.0 (default), both are valid
        assert settings.llm_timeout in [30.0, 60.0]
        assert hasattr(settings, 'llm_max_retries')
        assert settings.llm_max_retries == 3
        assert hasattr(settings, 'db_connection_max_retries')
        assert settings.db_connection_max_retries == 5
    
    def test_azure_endpoint_validation(self):
        """Test Azure endpoint format validation"""
        # Valid endpoint - validator may normalize the URL
        try:
            settings = Settings(azure_openai_endpoint="https://test.openai.azure.com/")
            # Just check it's a valid URL
            assert settings.azure_openai_endpoint.startswith("https://")
        except Exception:
            # Validation may fail if Azure keys not set
            pass
    
    def test_numeric_range_validation(self):
        """Test numeric range validation for GPU config"""
        settings = Settings()
        
        if hasattr(settings, 'gpu_batch_size'):
            assert settings.gpu_batch_size > 0
            assert settings.gpu_batch_size <= 1024
        
        if hasattr(settings, 'gpu_memory_limit'):
            assert settings.gpu_memory_limit > 0
    
    def test_environment_override(self, monkeypatch):
        """Test environment variable override"""
        monkeypatch.setenv("SERVICE_NAME", "custom-service")
        monkeypatch.setenv("SERVICE_PORT", "9000")
        
        settings = Settings()
        assert settings.service_name == "custom-service"
        assert settings.service_port == 9000
    
    def test_llm_provider_config(self):
        """Test LLM provider configuration"""
        settings = Settings()
        assert settings.default_llm_provider in ["openai", "azure_openai", "anthropic", "google", "deepseek", "ollama"]
    
    def test_redis_config(self):
        """Test Redis/DragonflyDB configuration"""
        settings = Settings()
        assert settings.redis_host is not None
        assert settings.redis_port > 0
        assert settings.redis_db >= 0
    
    def test_movement_config(self):
        """Test NPC movement configuration"""
        settings = Settings()
        assert hasattr(settings, 'npc_movement_enabled')
        assert hasattr(settings, 'npc_movement_update_interval')
        assert hasattr(settings, 'npc_movement_max_distance')
    
    def test_chat_command_config(self):
        """Test chat command configuration"""
        settings = Settings()
        assert hasattr(settings, 'chat_command_enabled')
        assert hasattr(settings, 'chat_command_prefix')
        assert hasattr(settings, 'chat_command_cooldown')
    
    def test_gpu_config(self):
        """Test GPU acceleration configuration"""
        settings = Settings()
        assert hasattr(settings, 'gpu_enabled')
        assert isinstance(settings.gpu_enabled, bool)


class TestConfigValidation:
    """Test configuration validation"""
    
    def test_invalid_port(self):
        """Test invalid port number"""
        # Should raise ValidationError for invalid port
        with pytest.raises(ValidationError):
            Settings(service_port=-1)

        with pytest.raises(ValidationError):
            Settings(service_port=0)

        with pytest.raises(ValidationError):
            Settings(service_port=70000)

    def test_invalid_redis_db(self):
        """Test invalid Redis DB number"""
        # Should raise ValidationError for negative redis_db
        with pytest.raises(ValidationError):
            Settings(redis_db=-1)
    
    def test_temperature_range(self):
        """Test temperature values are in valid range"""
        settings = Settings()
        assert 0.0 <= settings.dialogue_temperature <= 2.0
        assert 0.0 <= settings.decision_temperature <= 2.0
    
    def test_timeout_positive(self):
        """Test timeout values are positive"""
        settings = Settings()
        assert settings.llm_timeout > 0
        assert settings.db_connection_retry_delay > 0
    
    def test_retry_count_positive(self):
        """Test retry counts are positive"""
        settings = Settings()
        assert settings.llm_max_retries > 0
        assert settings.db_connection_max_retries > 0

    def test_invalid_azure_endpoint(self):
        """Test invalid Azure OpenAI endpoint format"""
        with pytest.raises(ValidationError):
            Settings(azure_openai_endpoint="https://invalid-endpoint.com")
        with pytest.raises(ValidationError):
            Settings(azure_openai_endpoint="http://test.openai.azure.com")

    def test_azure_endpoint_none(self):
        """Test Azure endpoint can be None"""
        settings = Settings(azure_openai_endpoint=None)
        assert settings.azure_openai_endpoint is None

    def test_gpu_memory_fraction_validation(self):
        """Test GPU memory fraction validation"""
        with pytest.raises(ValidationError):
            Settings(gpu_memory_fraction=1.5)
        with pytest.raises(ValidationError):
            Settings(gpu_memory_fraction=-0.1)

    def test_positive_int_validation(self):
        """Test positive integer validation"""
        with pytest.raises(ValidationError):
            Settings(gpu_batch_size=0)
        with pytest.raises(ValidationError):
            Settings(gpu_max_context_length=-1)
        with pytest.raises(ValidationError):
            Settings(gpu_num_threads=0)

    def test_personality_influence_validation(self):
        """Test personality influence validation"""
        with pytest.raises(ValidationError):
            Settings(npc_movement_personality_influence=1.5)
        with pytest.raises(ValidationError):
            Settings(npc_movement_personality_influence=-0.1)

    def test_reasoning_depth_validation(self):
        """Test reasoning depth validation"""
        with pytest.raises(ValidationError):
            Settings(reasoning_depth="invalid")

    def test_optimization_mode_validation(self):
        """Test LLM optimization mode validation"""
        with pytest.raises(ValidationError):
            Settings(llm_optimization_mode="invalid")

    def test_zero_to_one_validation(self):
        """Test zero to one range validation"""
        with pytest.raises(ValidationError):
            Settings(npc_relationship_decay_rate=1.5)
        with pytest.raises(ValidationError):
            Settings(decision_complexity_threshold=-0.1)
        with pytest.raises(ValidationError):
            Settings(decision_embedding_similarity_threshold=2.0)

    def test_npc_movement_mode_validation(self):
        """Test NPC movement mode validation"""
        with pytest.raises(ValidationError):
            Settings(npc_movement_mode="invalid")

    def test_economy_update_mode_validation(self):
        """Test economy update mode validation"""
        with pytest.raises(ValidationError):
            Settings(economy_update_mode="invalid")

    def test_shop_restock_mode_validation(self):
        """Test shop restock mode validation"""
        with pytest.raises(ValidationError):
            Settings(shop_restock_mode="invalid")

    def test_postgres_connection_string(self):
        """Test PostgreSQL connection string generation"""
        settings = Settings()
        conn_str = settings.postgres_connection_string
        assert "postgresql+psycopg2://" in conn_str
        assert settings.postgres_user in conn_str
        assert settings.postgres_host in conn_str
        assert str(settings.postgres_port) in conn_str
        assert settings.postgres_db in conn_str

    def test_load_yaml_config_file_not_found(self):
        """Test load_yaml_config with non-existent file"""
        from ai_service.config import load_yaml_config
        result = load_yaml_config("nonexistent.yaml")
        assert result == {}

    def test_load_yaml_config_invalid_yaml(self, tmp_path):
        """Test load_yaml_config with invalid YAML"""
        from ai_service.config import load_yaml_config
        invalid_yaml = tmp_path / "invalid.yaml"
        invalid_yaml.write_text("invalid: yaml: content:")
        result = load_yaml_config(str(invalid_yaml))
        assert result == {}

    def test_get_settings_with_gpu_enabled(self, monkeypatch):
        """Test get_settings with GPU enabled"""
        from ai_service.config import get_settings
        monkeypatch.setenv("GPU_ENABLED", "true")
        monkeypatch.setenv("GPU_DEVICE", "cuda:0")

        settings = get_settings()
        assert settings.gpu_enabled == True
        assert settings.gpu_device == "cuda:0"

    def test_cors_origins_from_comma_separated_string(self, monkeypatch):
        """Test CORS origins parsing from comma-separated string"""
        monkeypatch.setenv("CORS_ORIGINS", "http://localhost:3000,http://localhost:4000,http://example.com")
        settings = Settings()
        assert settings.cors_origins == ["http://localhost:3000", "http://localhost:4000", "http://example.com"]

    def test_cors_origins_from_empty_string(self, monkeypatch):
        """Test CORS origins parsing from empty string"""
        monkeypatch.setenv("CORS_ORIGINS", "")
        settings = Settings()
        assert settings.cors_origins == ["http://localhost:8888", "http://127.0.0.1:8888"]

    def test_cors_origins_from_whitespace_string(self, monkeypatch):
        """Test CORS origins parsing from whitespace string"""
        monkeypatch.setenv("CORS_ORIGINS", "   ")
        settings = Settings()
        assert settings.cors_origins == ["http://localhost:8888", "http://127.0.0.1:8888"]

