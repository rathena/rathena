"""
Configuration Management for ML Inference Service
Loads and validates configuration from YAML file
"""

import os
import yaml
from pathlib import Path
from typing import Dict, Any, Optional
import logging

logger = logging.getLogger(__name__)


class ConfigLoader:
    """Load and validate inference service configuration"""
    
    def __init__(self, config_path: Optional[str] = None):
        """
        Initialize configuration loader
        
        Args:
            config_path: Path to YAML config file. If None, uses default location.
        """
        if config_path is None:
            config_path = "/opt/ml_monster_ai/configs/inference_config.yaml"
        
        self.config_path = Path(config_path)
        self.config: Dict[str, Any] = {}
        
    def load(self) -> Dict[str, Any]:
        """
        Load configuration from YAML file
        
        Returns:
            Loaded configuration dictionary
        
        Raises:
            FileNotFoundError: If config file doesn't exist
            yaml.YAMLError: If config is invalid YAML
            ValueError: If required fields are missing
        """
        if not self.config_path.exists():
            raise FileNotFoundError(f"Config file not found: {self.config_path}")
        
        logger.info(f"Loading configuration from {self.config_path}")
        
        with open(self.config_path, 'r') as f:
            self.config = yaml.safe_load(f)
        
        # Substitute environment variables
        self._substitute_env_vars(self.config)
        
        # Validate configuration
        self._validate()
        
        logger.info("Configuration loaded successfully")
        return self.config
    
    def _substitute_env_vars(self, config: Dict[str, Any]) -> None:
        """
        Recursively substitute environment variables in config
        
        Replaces ${VAR_NAME} with os.environ.get('VAR_NAME')
        """
        for key, value in config.items():
            if isinstance(value, dict):
                self._substitute_env_vars(value)
            elif isinstance(value, str):
                # Check for ${VAR} pattern
                if '${' in value and '}' in value:
                    start = value.find('${')
                    end = value.find('}', start)
                    var_name = value[start+2:end]
                    env_value = os.environ.get(var_name, '')
                    
                    if not env_value:
                        logger.warning(f"Environment variable {var_name} not set, using empty string")
                    
                    config[key] = value[:start] + env_value + value[end+1:]
    
    def _validate(self) -> None:
        """
        Validate configuration has all required fields
        
        Raises:
            ValueError: If required fields are missing
        """
        required_sections = [
            'models',
            'inference',
            'hardware',
            'database',
            'caching',
            'fallback',
            'monitoring',
            'logging'
        ]
        
        for section in required_sections:
            if section not in self.config:
                raise ValueError(f"Missing required config section: {section}")
        
        # Validate database subsections
        if 'postgresql' not in self.config['database']:
            raise ValueError("Missing database.postgresql configuration")
        
        if 'redis' not in self.config['database']:
            raise ValueError("Missing database.redis configuration")
        
        # Validate paths exist
        model_dir = Path(self.config['models']['directory'])
        if not model_dir.exists():
            logger.warning(f"Model directory does not exist: {model_dir}")
            logger.warning("Service will start in fallback mode until models are available")
        
        logger.info("Configuration validation passed")
    
    def get(self, key_path: str, default: Any = None) -> Any:
        """
        Get configuration value by dot-separated key path
        
        Args:
            key_path: Dot-separated path (e.g., 'database.postgresql.host')
            default: Default value if key doesn't exist
        
        Returns:
            Configuration value or default
        """
        keys = key_path.split('.')
        value = self.config
        
        for key in keys:
            if isinstance(value, dict) and key in value:
                value = value[key]
            else:
                return default
        
        return value


def load_config(config_path: Optional[str] = None) -> Dict[str, Any]:
    """
    Load configuration (convenience function)
    
    Args:
        config_path: Path to YAML config file
    
    Returns:
        Loaded configuration dictionary
    """
    loader = ConfigLoader(config_path)
    return loader.load()


# Default configuration (used if file doesn't exist)
DEFAULT_CONFIG = {
    'models': {
        'directory': '/opt/ml_monster_ai/models',
        'format': 'onnx',
        'precision': 'fp16',
        'archetypes': ['aggressive', 'defensive', 'support', 'mage', 'tank', 'ranged'],
        'types': [
            'combat_dqn',
            'movement_ppo',
            'skill_dqn',
            'threat_assessment',
            'team_coordination',
            'spatial_vit',
            'temporal_transformer',
            'pattern_recognition',
            'soft_actor_critic'
        ]
    },
    'inference': {
        'batch_size': 128,
        'poll_interval_ms': 10,
        'max_latency_ms': 15,
        'timeout_ms': 100,
        'primary_model': 'combat_dqn'
    },
    'hardware': {
        'device': 'cuda:0',
        'max_vram_gb': 6.0,
        'use_tensorrt': False,
        'num_workers': 4,
        'cpu_threads': 16
    },
    'database': {
        'postgresql': {
            'host': 'localhost',
            'port': 5432,
            'database': 'ragnarok_ml',
            'user': 'ml_user',
            'password': '',
            'min_pool_size': 10,
            'max_pool_size': 20,
            'command_timeout': 60
        },
        'redis': {
            'host': 'localhost',
            'port': 6379,
            'password': None,
            'db': 0,
            'socket_timeout': 5
        }
    },
    'caching': {
        'enabled': True,
        'l2_ttl_seconds': 10,
        'l3_ttl_seconds': 10,
        'max_cache_size_mb': 4096
    },
    'fallback': {
        'enabled': True,
        'max_consecutive_errors': 3,
        'recovery_check_interval_seconds': 60,
        'auto_recovery': True
    },
    'monitoring': {
        'prometheus_port': 9090,
        'metrics_enabled': True,
        'log_level': 'INFO'
    },
    'logging': {
        'level': 'INFO',
        'format': 'json',
        'file': '/opt/ml_monster_ai/logs/inference_service.log',
        'max_bytes': 100 * 1024 * 1024,  # 100MB
        'backup_count': 10
    }
}


def save_default_config(path: str) -> None:
    """
    Save default configuration to file
    
    Args:
        path: Path where to save config
    """
    path_obj = Path(path)
    path_obj.parent.mkdir(parents=True, exist_ok=True)
    
    with open(path, 'w') as f:
        yaml.dump(DEFAULT_CONFIG, f, default_flow_style=False, indent=2)
    
    logger.info(f"Default configuration saved to {path}")
