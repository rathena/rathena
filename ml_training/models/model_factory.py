"""
ML Monster AI - Model Factory and Registry
Enhanced ML Monster AI System v2.0

Factory for creating and managing all 54 models (9 types × 6 archetypes)
"""

import torch
import torch.nn as nn
from typing import Dict, Any, Optional, List
import os
import logging
from pathlib import Path

from .model_architectures import (
    CombatDQN,
    MovementPPO,
    SkillDQN,
    ThreatAssessment,
    TeamCoordinationLSTM,
    SpatialViT,
    TemporalTransformerXL,
    PatternRecognitionTransformer,
    SoftActorCritic
)

logger = logging.getLogger(__name__)


class ModelFactory:
    """
    Factory for creating models by archetype and type
    
    Manages all 54 models:
    - 6 archetypes: aggressive, defensive, support, mage, tank, ranged
    - 9 model types per archetype
    
    Total: 54 models
    """
    
    ARCHETYPES = ['aggressive', 'defensive', 'support', 'mage', 'tank', 'ranged']
    
    MODEL_TYPES = [
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
    
    # Default hyperparameters per model type
    DEFAULT_CONFIGS = {
        'combat_dqn': {
            'state_dim': 64,
            'action_dim': 10,
            'hidden_dims': [1024, 1024, 512, 512, 256, 256, 128, 128, 64, 64],
            'dropout': 0.2,
            'use_dueling': True
        },
        'movement_ppo': {
            'state_dim': 64,
            'action_dim': 10,
            'hidden_dim': 512,
            'shared_layers': 6,
            'head_layers': 3,
            'dropout': 0.1
        },
        'skill_dqn': {
            'state_dim': 64,
            'num_skills': 20,
            'embedding_dim': 32,
            'action_dim': 10,
            'hidden_dims': [512, 256, 128],
            'dropout': 0.2
        },
        'threat_assessment': {
            'state_dim': 64,
            'hidden_dim': 256,
            'output_dim': 1
        },
        'team_coordination': {
            'state_dim': 64,
            'hidden_dim': 256,
            'num_layers': 3,
            'action_dim': 10,
            'bidirectional': True,
            'attention_heads': 8,
            'dropout': 0.2
        },
        'spatial_vit': {
            'state_dim': 64,
            'patch_size': 8,
            'embed_dim': 192,
            'depth': 12,
            'num_heads': 3,
            'mlp_ratio': 4,
            'action_dim': 10,
            'dropout': 0.1
        },
        'temporal_transformer': {
            'state_dim': 64,
            'd_model': 256,
            'nhead': 8,
            'num_layers': 4,
            'action_dim': 10,
            'dropout': 0.1
        },
        'pattern_recognition': {
            'state_dim': 64,
            'd_model': 512,
            'nhead': 16,
            'num_layers': 6,
            'action_dim': 10,
            'dropout': 0.1
        },
        'soft_actor_critic': {
            'state_dim': 64,
            'action_dim': 10,
            'hidden_dim': 256,
            'num_layers': 3
        }
    }
    
    # Archetype-specific modifications
    ARCHETYPE_MODIFICATIONS = {
        'aggressive': {
            'combat_dqn': {'dropout': 0.15},  # Less dropout = more aggressive
            'movement_ppo': {'dropout': 0.05}  # More risk-taking
        },
        'defensive': {
            'combat_dqn': {'dropout': 0.25},  # More conservative
            'threat_assessment': {'hidden_dim': 384}  # Better threat assessment
        },
        'support': {
            'team_coordination': {'attention_heads': 12},  # More attention to team
            'movement_ppo': {'hidden_dim': 768}  # Better positioning
        },
        'mage': {
            'skill_dqn': {'embedding_dim': 48},  # More skill representation
            'combat_dqn': {'action_dim': 12}  # More spell options
        },
        'tank': {
            'combat_dqn': {'hidden_dims': [1024] * 12},  # Deeper for complexity
            'threat_assessment': {'hidden_dim': 512}  # Better threat evaluation
        },
        'ranged': {
            'spatial_vit': {'depth': 16},  # Better spatial awareness
            'movement_ppo': {'action_dim': 12}  # More movement options
        }
    }
    
    @classmethod
    def create_model(
        cls, 
        archetype: str, 
        model_type: str, 
        custom_config: Optional[Dict[str, Any]] = None,
        device: str = 'cuda'
    ) -> nn.Module:
        """
        Create a model instance
        
        Args:
            archetype: One of ARCHETYPES
            model_type: One of MODEL_TYPES
            custom_config: Optional custom configuration (overrides defaults)
            device: Device to create model on ('cuda' or 'cpu')
        
        Returns:
            Initialized PyTorch model
        
        Raises:
            ValueError: If archetype or model_type is invalid
        """
        if archetype not in cls.ARCHETYPES:
            raise ValueError(f"Unknown archetype: {archetype}. Must be one of {cls.ARCHETYPES}")
        
        if model_type not in cls.MODEL_TYPES:
            raise ValueError(f"Unknown model type: {model_type}. Must be one of {cls.MODEL_TYPES}")
        
        # Get default config
        config = cls.DEFAULT_CONFIGS[model_type].copy()
        
        # Apply archetype-specific modifications
        if archetype in cls.ARCHETYPE_MODIFICATIONS:
            if model_type in cls.ARCHETYPE_MODIFICATIONS[archetype]:
                config.update(cls.ARCHETYPE_MODIFICATIONS[archetype][model_type])
        
        # Apply custom config
        if custom_config:
            config.update(custom_config)
        
        # Model class mapping
        model_classes = {
            'combat_dqn': CombatDQN,
            'movement_ppo': MovementPPO,
            'skill_dqn': SkillDQN,
            'threat_assessment': ThreatAssessment,
            'team_coordination': TeamCoordinationLSTM,
            'spatial_vit': SpatialViT,
            'temporal_transformer': TemporalTransformerXL,
            'pattern_recognition': PatternRecognitionTransformer,
            'soft_actor_critic': SoftActorCritic
        }
        
        model_class = model_classes[model_type]
        
        # Create model
        logger.info(f"Creating {archetype}/{model_type} with config: {config}")
        model = model_class(**config)
        
        # Move to device
        model = model.to(device)
        
        # Log model info
        logger.info(f"Model {archetype}/{model_type} created successfully on {device}")
        logger.info(f"  Parameters: {model.count_parameters():,}")
        
        from .model_architectures import estimate_model_size
        size_fp16 = estimate_model_size(model, 'fp16')
        logger.info(f"  Size (FP16): {size_fp16['mb']:.2f} MB")
        
        return model
    
    @classmethod
    def create_all_models(
        cls,
        archetypes: Optional[List[str]] = None,
        model_types: Optional[List[str]] = None,
        device: str = 'cuda'
    ) -> Dict[str, Dict[str, nn.Module]]:
        """
        Create all models or subset of models
        
        Args:
            archetypes: List of archetypes to create (default: all)
            model_types: List of model types to create (default: all)
            device: Device to create models on
        
        Returns:
            Nested dict: {archetype: {model_type: model}}
        """
        if archetypes is None:
            archetypes = cls.ARCHETYPES
        
        if model_types is None:
            model_types = cls.MODEL_TYPES
        
        all_models = {}
        total_count = 0
        total_size_mb = 0.0
        
        logger.info(f"Creating {len(archetypes)} × {len(model_types)} = {len(archetypes) * len(model_types)} models...")
        
        for archetype in archetypes:
            all_models[archetype] = {}
            
            for model_type in model_types:
                model = cls.create_model(archetype, model_type, device=device)
                all_models[archetype][model_type] = model
                
                total_count += 1
                
                from .model_architectures import estimate_model_size
                size_info = estimate_model_size(model, 'fp16')
                total_size_mb += size_info['mb']
                
                logger.info(f"  [{total_count}/{len(archetypes) * len(model_types)}] "
                          f"{archetype}/{model_type}: {size_info['mb']:.2f} MB")
        
        logger.info(f"\n{'='*60}")
        logger.info(f"All {total_count} models created successfully")
        logger.info(f"Total size (FP16): {total_size_mb:.2f} MB ({total_size_mb/1024:.2f} GB)")
        logger.info(f"{'='*60}\n")
        
        return all_models
    
    @classmethod
    def get_model_path(
        cls, 
        archetype: str, 
        model_type: str, 
        version: str = 'latest',
        base_dir: str = '/opt/ml_monster_ai/models'
    ) -> str:
        """
        Get path to saved model
        
        Args:
            archetype: Model archetype
            model_type: Model type
            version: Model version ('latest' or specific version string)
            base_dir: Base directory for models
        
        Returns:
            Full path to model file
        """
        if archetype not in cls.ARCHETYPES:
            raise ValueError(f"Unknown archetype: {archetype}")
        
        if model_type not in cls.MODEL_TYPES:
            raise ValueError(f"Unknown model type: {model_type}")
        
        model_dir = Path(base_dir) / archetype
        model_dir.mkdir(parents=True, exist_ok=True)
        
        if version == 'latest':
            # Find latest version
            pattern = f"{model_type}_v*.pth"
            existing = list(model_dir.glob(pattern))
            
            if existing:
                # Extract version numbers and find max
                versions = []
                for p in existing:
                    try:
                        ver_str = p.stem.split('_v')[1]
                        versions.append((int(ver_str), p))
                    except (IndexError, ValueError):
                        continue
                
                if versions:
                    latest_path = max(versions, key=lambda x: x[0])[1]
                    return str(latest_path)
            
            # No existing version, return v1 path
            version = 'v1'
        
        model_path = model_dir / f"{model_type}_{version}.pth"
        return str(model_path)
    
    @classmethod
    def save_model(
        cls,
        model: nn.Module,
        archetype: str,
        model_type: str,
        version: str,
        metadata: Optional[Dict[str, Any]] = None,
        base_dir: str = '/opt/ml_monster_ai/models'
    ) -> str:
        """
        Save model checkpoint
        
        Args:
            model: Model to save
            archetype: Model archetype
            model_type: Model type
            version: Version string
            metadata: Optional metadata to save with model
            base_dir: Base directory for models
        
        Returns:
            Path to saved model
        """
        model_path = cls.get_model_path(archetype, model_type, version, base_dir)
        
        # Prepare checkpoint
        checkpoint = {
            'model_state_dict': model.state_dict(),
            'archetype': archetype,
            'model_type': model_type,
            'version': version,
            'parameters': sum(p.numel() for p in model.parameters()),
            'architecture_config': cls.DEFAULT_CONFIGS.get(model_type, {}),
            'metadata': metadata or {}
        }
        
        # Save checkpoint
        torch.save(checkpoint, model_path)
        
        logger.info(f"Model saved: {model_path}")
        logger.info(f"  Archetype: {archetype}")
        logger.info(f"  Type: {model_type}")
        logger.info(f"  Version: {version}")
        logger.info(f"  Size: {os.path.getsize(model_path) / (1024**2):.2f} MB")
        
        return model_path
    
    @classmethod
    def load_model(
        cls,
        archetype: str,
        model_type: str,
        version: str = 'latest',
        device: str = 'cuda',
        base_dir: str = '/opt/ml_monster_ai/models'
    ) -> nn.Module:
        """
        Load model checkpoint
        
        Args:
            archetype: Model archetype
            model_type: Model type
            version: Version to load
            device: Device to load model on
            base_dir: Base directory for models
        
        Returns:
            Loaded model
        
        Raises:
            FileNotFoundError: If model file doesn't exist
        """
        model_path = cls.get_model_path(archetype, model_type, version, base_dir)
        
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Model not found: {model_path}")
        
        # Load checkpoint
        checkpoint = torch.load(model_path, map_location=device)
        
        logger.info(f"Loading model from: {model_path}")
        logger.info(f"  Archetype: {checkpoint.get('archetype', 'unknown')}")
        logger.info(f"  Type: {checkpoint.get('model_type', 'unknown')}")
        logger.info(f"  Version: {checkpoint.get('version', 'unknown')}")
        logger.info(f"  Parameters: {checkpoint.get('parameters', 'unknown'):,}")
        
        # Create model with saved config
        config = checkpoint.get('architecture_config', cls.DEFAULT_CONFIGS.get(model_type, {}))
        model = cls.create_model(archetype, model_type, custom_config=config, device=device)
        
        # Load state dict
        model.load_state_dict(checkpoint['model_state_dict'])
        
        logger.info(f"Model loaded successfully")
        
        return model
    
    @classmethod
    def load_all_models(
        cls,
        archetypes: Optional[List[str]] = None,
        model_types: Optional[List[str]] = None,
        version: str = 'latest',
        device: str = 'cuda',
        base_dir: str = '/opt/ml_monster_ai/models'
    ) -> Dict[str, Dict[str, nn.Module]]:
        """
        Load all models from checkpoints
        
        Args:
            archetypes: List of archetypes to load (default: all)
            model_types: List of model types to load (default: all)
            version: Version to load
            device: Device to load models on
            base_dir: Base directory for models
        
        Returns:
            Nested dict: {archetype: {model_type: model}}
        """
        if archetypes is None:
            archetypes = cls.ARCHETYPES
        
        if model_types is None:
            model_types = cls.MODEL_TYPES
        
        all_models = {}
        loaded_count = 0
        failed_count = 0
        total_size_mb = 0.0
        
        logger.info(f"Loading {len(archetypes)} × {len(model_types)} = {len(archetypes) * len(model_types)} models...")
        
        for archetype in archetypes:
            all_models[archetype] = {}
            
            for model_type in model_types:
                try:
                    model = cls.load_model(archetype, model_type, version, device, base_dir)
                    all_models[archetype][model_type] = model
                    loaded_count += 1
                    
                    from .model_architectures import estimate_model_size
                    size_info = estimate_model_size(model, 'fp16')
                    total_size_mb += size_info['mb']
                    
                    logger.info(f"  [{loaded_count}/{len(archetypes) * len(model_types)}] "
                              f"✓ {archetype}/{model_type}")
                
                except FileNotFoundError as e:
                    logger.warning(f"  [{loaded_count + failed_count + 1}/{len(archetypes) * len(model_types)}] "
                                 f"✗ {archetype}/{model_type} - Not found")
                    failed_count += 1
                
                except Exception as e:
                    logger.error(f"  [{loaded_count + failed_count + 1}/{len(archetypes) * len(model_types)}] "
                               f"✗ {archetype}/{model_type} - Error: {e}")
                    failed_count += 1
        
        logger.info(f"\n{'='*60}")
        logger.info(f"Models loaded: {loaded_count} succeeded, {failed_count} failed")
        logger.info(f"Total size (FP16): {total_size_mb:.2f} MB ({total_size_mb/1024:.2f} GB)")
        logger.info(f"{'='*60}\n")
        
        return all_models
    
    @classmethod
    def get_model_info(cls, archetype: str, model_type: str) -> Dict[str, Any]:
        """
        Get model configuration and metadata
        
        Args:
            archetype: Model archetype
            model_type: Model type
        
        Returns:
            Dictionary with model information
        """
        if archetype not in cls.ARCHETYPES:
            raise ValueError(f"Unknown archetype: {archetype}")
        
        if model_type not in cls.MODEL_TYPES:
            raise ValueError(f"Unknown model type: {model_type}")
        
        # Get default config
        config = cls.DEFAULT_CONFIGS[model_type].copy()
        
        # Apply archetype modifications
        if archetype in cls.ARCHETYPE_MODIFICATIONS:
            if model_type in cls.ARCHETYPE_MODIFICATIONS[archetype]:
                config.update(cls.ARCHETYPE_MODIFICATIONS[archetype][model_type])
        
        return {
            'archetype': archetype,
            'model_type': model_type,
            'config': config,
            'description': cls._get_model_description(model_type)
        }
    
    @classmethod
    def _get_model_description(cls, model_type: str) -> str:
        """Get human-readable model description"""
        descriptions = {
            'combat_dqn': 'Deep Q-Network for tactical combat decisions (10-12 layers)',
            'movement_ppo': 'Proximal Policy Optimization for movement (12 layers, actor-critic)',
            'skill_dqn': 'DQN with skill embeddings for skill selection',
            'threat_assessment': 'Ensemble model for threat evaluation',
            'team_coordination': 'LSTM for pack coordination (3 layers, bidirectional)',
            'spatial_vit': 'Vision Transformer for spatial awareness (ViT-tiny adapted)',
            'temporal_transformer': 'Transformer-XL for long-term temporal modeling (4 layers)',
            'pattern_recognition': 'Large transformer for pattern detection (6 layers, 512-dim)',
            'soft_actor_critic': 'Soft Actor-Critic for continuous control'
        }
        
        return descriptions.get(model_type, 'Unknown model type')
    
    @classmethod
    def list_available_models(cls, base_dir: str = '/opt/ml_monster_ai/models') -> Dict[str, List[str]]:
        """
        List all available trained models
        
        Args:
            base_dir: Base directory for models
        
        Returns:
            Dictionary mapping archetype to list of available model types
        """
        available = {}
        
        for archetype in cls.ARCHETYPES:
            archetype_dir = Path(base_dir) / archetype
            
            if not archetype_dir.exists():
                available[archetype] = []
                continue
            
            # Find all model files
            model_files = list(archetype_dir.glob('*.pth'))
            
            # Extract model types
            model_types_found = set()
            for model_file in model_files:
                # Parse filename: {model_type}_v{version}.pth
                parts = model_file.stem.split('_v')
                if parts:
                    model_types_found.add(parts[0])
            
            available[archetype] = sorted(list(model_types_found))
        
        return available
    
    @classmethod
    def verify_all_models_exist(cls, base_dir: str = '/opt/ml_monster_ai/models') -> bool:
        """
        Verify that all 54 models exist
        
        Args:
            base_dir: Base directory for models
        
        Returns:
            True if all models exist, False otherwise
        """
        available = cls.list_available_models(base_dir)
        
        missing = []
        for archetype in cls.ARCHETYPES:
            for model_type in cls.MODEL_TYPES:
                if model_type not in available.get(archetype, []):
                    missing.append(f"{archetype}/{model_type}")
        
        if missing:
            logger.warning(f"Missing {len(missing)} models:")
            for model_name in missing:
                logger.warning(f"  - {model_name}")
            return False
        
        logger.info(f"✓ All {len(cls.ARCHETYPES) * len(cls.MODEL_TYPES)} models exist")
        return True


class ModelRegistry:
    """
    Registry for tracking model versions and metadata
    
    Stores information about trained models:
    - Training metrics (loss, accuracy, win rate)
    - Training configuration
    - Version history
    - Deployment status
    """
    
    def __init__(self, registry_path: str = '/opt/ml_monster_ai/models/registry.json'):
        self.registry_path = registry_path
        self.registry: Dict[str, Dict[str, List[Dict]]] = {}
        
        # Load existing registry
        self.load()
    
    def load(self):
        """Load registry from disk"""
        if os.path.exists(self.registry_path):
            import json
            with open(self.registry_path, 'r') as f:
                self.registry = json.load(f)
            logger.info(f"Registry loaded from {self.registry_path}")
        else:
            logger.info(f"No existing registry found, starting fresh")
            self.registry = {archetype: {model_type: [] for model_type in ModelFactory.MODEL_TYPES} 
                           for archetype in ModelFactory.ARCHETYPES}
    
    def save(self):
        """Save registry to disk"""
        import json
        with open(self.registry_path, 'w') as f:
            json.dump(self.registry, f, indent=2)
        logger.info(f"Registry saved to {self.registry_path}")
    
    def register_model(
        self,
        archetype: str,
        model_type: str,
        version: str,
        metrics: Dict[str, float],
        model_path: str,
        config: Dict[str, Any]
    ):
        """
        Register a trained model
        
        Args:
            archetype: Model archetype
            model_type: Model type
            version: Version string
            metrics: Training metrics
            model_path: Path to saved model
            config: Training configuration
        """
        import time
        
        entry = {
            'version': version,
            'model_path': model_path,
            'metrics': metrics,
            'config': config,
            'registered_at': time.time(),
            'deployed': False
        }
        
        if archetype not in self.registry:
            self.registry[archetype] = {}
        
        if model_type not in self.registry[archetype]:
            self.registry[archetype][model_type] = []
        
        self.registry[archetype][model_type].append(entry)
        
        # Save updated registry
        self.save()
        
        logger.info(f"Registered {archetype}/{model_type} version {version}")
        logger.info(f"  Metrics: {metrics}")
    
    def get_best_model(
        self,
        archetype: str,
        model_type: str,
        metric: str = 'val_loss'
    ) -> Optional[Dict[str, Any]]:
        """
        Get best model version based on metric
        
        Args:
            archetype: Model archetype
            model_type: Model type
            metric: Metric to optimize ('val_loss', 'val_accuracy', 'win_rate')
        
        Returns:
            Best model entry or None
        """
        if archetype not in self.registry or model_type not in self.registry[archetype]:
            return None
        
        models = self.registry[archetype][model_type]
        
        if not models:
            return None
        
        # Determine if higher or lower is better
        higher_is_better = metric in ['val_accuracy', 'win_rate', 'reward']
        
        # Find best model
        valid_models = [m for m in models if metric in m.get('metrics', {})]
        
        if not valid_models:
            return models[-1]  # Return latest if no metric available
        
        if higher_is_better:
            best = max(valid_models, key=lambda m: m['metrics'][metric])
        else:
            best = min(valid_models, key=lambda m: m['metrics'][metric])
        
        return best
    
    def mark_deployed(self, archetype: str, model_type: str, version: str):
        """Mark a model version as deployed"""
        if archetype in self.registry and model_type in self.registry[archetype]:
            for entry in self.registry[archetype][model_type]:
                if entry['version'] == version:
                    entry['deployed'] = True
                    self.save()
                    logger.info(f"Marked {archetype}/{model_type} version {version} as deployed")
                    return
        
        logger.warning(f"Model {archetype}/{model_type} version {version} not found in registry")
    
    def get_deployed_models(self) -> Dict[str, Dict[str, str]]:
        """
        Get all currently deployed models
        
        Returns:
            Dict mapping archetype -> model_type -> version
        """
        deployed = {}
        
        for archetype in self.registry:
            deployed[archetype] = {}
            for model_type in self.registry[archetype]:
                for entry in self.registry[archetype][model_type]:
                    if entry.get('deployed', False):
                        deployed[archetype][model_type] = entry['version']
        
        return deployed
    
    def print_summary(self):
        """Print registry summary"""
        logger.info(f"\n{'='*60}")
        logger.info("Model Registry Summary")
        logger.info(f"{'='*60}")
        
        for archetype in ModelFactory.ARCHETYPES:
            logger.info(f"\n{archetype.upper()}:")
            
            for model_type in ModelFactory.MODEL_TYPES:
                if archetype in self.registry and model_type in self.registry[archetype]:
                    models = self.registry[archetype][model_type]
                    logger.info(f"  {model_type}: {len(models)} version(s)")
                    
                    if models:
                        latest = models[-1]
                        logger.info(f"    Latest: {latest['version']}")
                        if 'metrics' in latest:
                            logger.info(f"    Metrics: {latest['metrics']}")
                else:
                    logger.info(f"  {model_type}: 0 versions")
        
        logger.info(f"\n{'='*60}\n")


# Utility functions for model management

def count_total_parameters(models: Dict[str, Dict[str, nn.Module]]) -> int:
    """Count total parameters across all models"""
    total = 0
    for archetype_models in models.values():
        for model in archetype_models.values():
            total += sum(p.numel() for p in model.parameters() if p.requires_grad)
    return total


def estimate_total_vram(models: Dict[str, Dict[str, nn.Module]], precision: str = 'fp16') -> float:
    """
    Estimate total VRAM usage for all models
    
    Args:
        models: Nested dict of models
        precision: 'fp32', 'fp16', or 'int8'
    
    Returns:
        Total VRAM in GB
    """
    from .model_architectures import estimate_model_size
    
    total_mb = 0.0
    
    for archetype_models in models.values():
        for model in archetype_models.values():
            size_info = estimate_model_size(model, precision)
            total_mb += size_info['mb']
    
    return total_mb / 1024  # Convert to GB


def set_model_precision(model: nn.Module, precision: str = 'fp16') -> nn.Module:
    """
    Convert model to specified precision
    
    Args:
        model: PyTorch model
        precision: 'fp32', 'fp16'
    
    Returns:
        Model in specified precision
    """
    if precision == 'fp16':
        model = model.half()
        logger.info(f"Model converted to FP16")
    elif precision == 'fp32':
        model = model.float()
        logger.info(f"Model converted to FP32")
    else:
        raise ValueError(f"Unsupported precision: {precision}")
    
    return model
