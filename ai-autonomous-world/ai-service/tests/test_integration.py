"""
Integration tests for AI Autonomous World Service
Tests actual module imports and basic functionality
"""

import pytest
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


class TestModuleImports:
    """Test that all modules can be imported"""
    
    def test_config_import(self):
        """Test config module imports"""
        from ai_service.config import settings, Settings
        assert settings is not None
        assert Settings is not None
    
    def test_database_import(self):
        """Test database module imports"""
        from database import Database, db
        assert Database is not None
        assert db is not None
    
    def test_llm_factory_import(self):
        """Test LLM factory imports"""
        from llm.factory import LLMProviderFactory
        assert LLMProviderFactory is not None
    
    def test_llm_base_import(self):
        """Test LLM base imports"""
        from llm.base import BaseLLMProvider, LLMResponse
        assert BaseLLMProvider is not None
        assert LLMResponse is not None
    
    def test_azure_provider_import(self):
        """Test Azure OpenAI provider imports"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        assert AzureOpenAIProvider is not None
    
    def test_agents_import(self):
        """Test agent modules import"""
        from agents.dialogue_agent import DialogueAgent
        from agents.decision_agent import DecisionAgent
        from agents.orchestrator import AgentOrchestrator
        
        assert DialogueAgent is not None
        assert DecisionAgent is not None
        assert AgentOrchestrator is not None
    
    def test_main_app_import(self):
        """Test main application imports"""
        from main import app
        assert app is not None


class TestConfigurationValues:
    """Test configuration values are set correctly"""
    
    def test_remediation_config_fields(self):
        """Test new configuration fields from remediation"""
        from ai_service.config import settings
        
        # Action validation config
        assert hasattr(settings, 'max_movement_distance')
        assert settings.max_movement_distance > 0
        
        # LLM generation config
        assert hasattr(settings, 'dialogue_temperature')
        assert 0.0 <= settings.dialogue_temperature <= 2.0
        
        assert hasattr(settings, 'decision_temperature')
        assert 0.0 <= settings.decision_temperature <= 2.0
        
        # Retry/timeout config
        assert hasattr(settings, 'llm_timeout')
        assert settings.llm_timeout > 0
        
        assert hasattr(settings, 'llm_max_retries')
        assert settings.llm_max_retries > 0
        
        assert hasattr(settings, 'db_connection_max_retries')
        assert settings.db_connection_max_retries > 0
    
    def test_cors_security(self):
        """Test CORS is not set to wildcard"""
        from ai_service.config import settings
        
        assert "*" not in settings.cors_origins
        assert len(settings.cors_origins) > 0
    
    def test_llm_provider_config(self):
        """Test LLM provider configuration"""
        from ai_service.config import settings
        
        assert settings.default_llm_provider in [
            "openai", "azure_openai", "anthropic", 
            "google", "deepseek", "ollama"
        ]


class TestDatabaseConnection:
    """Test database connection functionality"""
    
    def test_database_instance(self):
        """Test database instance exists"""
        from database import db
        
        assert db is not None
        assert hasattr(db, 'client')
    
    def test_database_class(self):
        """Test Database class can be instantiated"""
        from database import Database
        
        test_db = Database()
        assert test_db is not None
        assert hasattr(test_db, 'connect')
        assert hasattr(test_db, 'disconnect')


class TestLLMProviders:
    """Test LLM provider functionality"""
    
    def test_azure_provider_initialization(self):
        """Test Azure provider can be initialized"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        
        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }
        
        provider = AzureOpenAIProvider(config)
        
        assert provider is not None
        assert provider.deployment == "gpt-4"
        assert provider.api_version == "2024-02-15-preview"
    
    def test_llm_response_model(self):
        """Test LLM response model"""
        from llm.base import LLMResponse

        response = LLMResponse(
            content="Test response",
            model="gpt-4",
            provider="azure_openai",
            tokens_used=30
        )

        assert response.content == "Test response"
        assert response.model == "gpt-4"
        assert response.provider == "azure_openai"
        assert response.tokens_used == 30


class TestAgents:
    """Test agent functionality"""
    
    def test_dialogue_agent_initialization(self):
        """Test dialogue agent can be initialized"""
        from agents.dialogue_agent import DialogueAgent
        from unittest.mock import MagicMock
        
        mock_llm = MagicMock()
        agent = DialogueAgent(
            agent_id="test_dialogue",
            llm_provider=mock_llm,
            config={}
        )
        
        assert agent is not None
        assert agent.agent_id == "test_dialogue"
        assert agent.agent_type == "dialogue"
    
    def test_decision_agent_initialization(self):
        """Test decision agent can be initialized"""
        from agents.decision_agent import DecisionAgent
        from unittest.mock import MagicMock
        
        mock_llm = MagicMock()
        agent = DecisionAgent(
            agent_id="test_decision",
            llm_provider=mock_llm,
            config={}
        )
        
        assert agent is not None
        assert agent.agent_id == "test_decision"
        assert agent.agent_type == "decision"

