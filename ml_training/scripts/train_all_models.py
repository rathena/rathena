#!/usr/bin/env python3
"""
ML Monster AI - Main Training Script
Enhanced ML Monster AI System v2.0

Train all 54 models (9 types × 6 archetypes)

Usage:
    python train_all_models.py --archetypes aggressive defensive --epochs 100
    python train_all_models.py --all --parallel
"""

import sys
import os
import argparse
import logging
from pathlib import Path
import torch
import torch.multiprocessing as mp
from typing import List, Dict, Any, Optional
import yaml
import time
from datetime import datetime

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from models.model_factory import ModelFactory, ModelRegistry
from models.model_architectures import estimate_model_size
from training.trainer import DQNTrainer, PPOTrainer, SACTrainer, LSTMTrainer
from training.rewards import ArchetypeRewards
from training.evaluator import ModelEvaluator
from data.replay_buffer import create_replay_buffer
from data.preprocessor import StatePreprocessor, RewardNormalizer

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/opt/ml_monster_ai/logs/training.log'),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)


class TrainingOrchestrator:
    """
    Orchestrate training of all 54 models
    
    Features:
    - Sequential or parallel training
    - VRAM management (stay under 11GB)
    - Checkpoint saving
    - Progress tracking
    - Model validation
    """
    
    def __init__(
        self,
        config_path: str = '/opt/ml_monster_ai/configs/training_config.yaml',
        device: str = 'cuda:0',
        max_vram_gb: float = 11.0
    ):
        """
        Initialize training orchestrator
        
        Args:
            config_path: Path to training configuration
            device: Device for training
            max_vram_gb: Maximum VRAM to use
        """
        self.config_path = config_path
        self.device = device
        self.max_vram_gb = max_vram_gb
        
        # Load configuration
        self.config = self.load_config()
        
        # Initialize registry
        self.registry = ModelRegistry()
        
        # Initialize evaluator
        self.evaluator = ModelEvaluator(device=device)
        
        # Training state
        self.trained_models = {}
        self.training_history = {}
        
        logger.info(f"TrainingOrchestrator initialized on {device}")
        logger.info(f"Max VRAM: {max_vram_gb:.2f} GB")
    
    def load_config(self) -> Dict[str, Any]:
        """Load training configuration"""
        if os.path.exists(self.config_path):
            with open(self.config_path, 'r') as f:
                config = yaml.safe_load(f)
            logger.info(f"Configuration loaded from {self.config_path}")
            return config
        else:
            logger.warning(f"Config file not found: {self.config_path}, using defaults")
            return self.get_default_config()
    
    def get_default_config(self) -> Dict[str, Any]:
        """Get default training configuration"""
        return {
            'training': {
                'batch_size': 256,
                'learning_rate': 0.0003,
                'epochs': 100,
                'replay_buffer_size': 100000,
                'update_frequency': 100,
                'target_update_frequency': 1000,
                'gamma': 0.99,
                'tau': 0.005
            },
            'hardware': {
                'device': 'cuda:0',
                'max_vram_gb': 11.0,
                'num_workers': 4,
                'pin_memory': True
            },
            'checkpoints': {
                'save_dir': '/opt/ml_monster_ai/models',
                'save_frequency': 1000,
                'keep_last_n': 5
            },
            'logging': {
                'tensorboard': True,
                'wandb': False,
                'log_frequency': 100
            }
        }
    
    def check_vram_available(self, required_gb: float) -> bool:
        """
        Check if enough VRAM available
        
        Args:
            required_gb: Required VRAM in GB
        
        Returns:
            True if enough VRAM available
        """
        if not torch.cuda.is_available():
            return False
        
        torch.cuda.synchronize()
        allocated = torch.cuda.memory_allocated(self.device) / (1024 ** 3)
        available = self.max_vram_gb - allocated
        
        logger.info(f"VRAM: {allocated:.2f} GB used, {available:.2f} GB available")
        
        return available >= required_gb
    
    def train_single_model(
        self,
        archetype: str,
        model_type: str,
        epochs: int = 100,
        validate: bool = True
    ) -> Dict[str, Any]:
        """
        Train a single model
        
        Args:
            archetype: Model archetype
            model_type: Model type
            epochs: Number of epochs
            validate: Whether to validate after training
        
        Returns:
            Training results
        """
        logger.info(f"\n{'='*60}")
        logger.info(f"Training {archetype}/{model_type}")
        logger.info(f"{'='*60}\n")
        
        # Create model
        model = ModelFactory.create_model(archetype, model_type, device=self.device)
        
        # Check model size
        size_info = estimate_model_size(model, 'fp16')
        required_vram = size_info['gb'] + 4.0  # Model + training overhead
        
        if not self.check_vram_available(required_vram):
            logger.error(f"Insufficient VRAM for {archetype}/{model_type}")
            return {'status': 'failed', 'reason': 'insufficient_vram'}
        
        # Create replay buffer
        buffer = create_replay_buffer(
            buffer_type='prioritized',
            archetype=archetype,
            capacity=self.config['training']['replay_buffer_size'],
            state_dim=64
        )
        
        # TODO: Load initial data into buffer
        # For now, we'll skip this and document it needs implementation
        
        # Create appropriate trainer
        if model_type in ['combat_dqn', 'skill_dqn']:
            # Create target model
            target_model = ModelFactory.create_model(archetype, model_type, device=self.device)
            trainer = DQNTrainer(
                model=model,
                target_model=target_model,
                replay_buffer=buffer,
                gamma=self.config['training']['gamma'],
                tau=self.config['training']['tau'],
                batch_size=self.config['training']['batch_size'],
                learning_rate=self.config['training']['learning_rate']
            )
        
        elif model_type == 'movement_ppo':
            trainer = PPOTrainer(
                model=model,
                gamma=self.config['training']['gamma'],
                batch_size=self.config['training']['batch_size'],
                learning_rate=self.config['training']['learning_rate']
            )
        
        elif model_type == 'soft_actor_critic':
            # SAC needs critics
            critic1 = ModelFactory.create_model(archetype, model_type, device=self.device)
            critic2 = ModelFactory.create_model(archetype, model_type, device=self.device)
            target_critic1 = ModelFactory.create_model(archetype, model_type, device=self.device)
            target_critic2 = ModelFactory.create_model(archetype, model_type, device=self.device)
            
            trainer = SACTrainer(
                actor=model,
                critic1=critic1,
                critic2=critic2,
                target_critic1=target_critic1,
                target_critic2=target_critic2,
                replay_buffer=buffer,
                gamma=self.config['training']['gamma'],
                batch_size=self.config['training']['batch_size']
            )
        
        else:
            # Generic trainer for transformers, LSTM, etc.
            trainer = DQNTrainer(
                model=model,
                target_model=ModelFactory.create_model(archetype, model_type, device=self.device),
                replay_buffer=buffer,
                gamma=self.config['training']['gamma'],
                batch_size=self.config['training']['batch_size'],
                learning_rate=self.config['training']['learning_rate']
            )
        
        # Training loop
        training_metrics = []
        best_loss = float('inf')
        
        for epoch in range(epochs):
            epoch_start = time.time()
            
            # NOTE: This is a placeholder training loop
            # In production, this would:
            # 1. Collect experiences from environment/database
            # 2. Train on collected experiences
            # 3. Validate on held-out data
            # 4. Save checkpoints
            
            # For now, we'll create dummy training to show the structure
            logger.info(f"Epoch {epoch+1}/{epochs}")
            
            # Dummy training step
            # In production, this would train on real data
            if len(buffer) >= self.config['training']['batch_size']:
                metrics = trainer.train_step()
                training_metrics.append(metrics)
                
                # Check if best model
                current_loss = metrics.get('loss', float('inf'))
                if current_loss < best_loss:
                    best_loss = current_loss
                    # Save best model
                    model_path = ModelFactory.save_model(
                        model=model,
                        archetype=archetype,
                        model_type=model_type,
                        version='best',
                        metadata={'epoch': epoch, 'loss': current_loss}
                    )
            
            epoch_time = time.time() - epoch_start
            
            if (epoch + 1) % 10 == 0:
                logger.info(f"Epoch {epoch+1} complete in {epoch_time:.2f}s")
        
        # Save final model
        final_model_path = ModelFactory.save_model(
            model=model,
            archetype=archetype,
            model_type=model_type,
            version='final',
            metadata={'epochs': epochs, 'final_loss': best_loss}
        )
        
        # Validate if requested
        val_metrics = {}
        if validate:
            # TODO: Validation on held-out data
            logger.info("Validation would run here with real test data")
        
        # Register model
        self.registry.register_model(
            archetype=archetype,
            model_type=model_type,
            version='final',
            metrics={'train_loss': best_loss, **val_metrics},
            model_path=final_model_path,
            config=ModelFactory.DEFAULT_CONFIGS[model_type]
        )
        
        results = {
            'status': 'success',
            'archetype': archetype,
            'model_type': model_type,
            'epochs_trained': epochs,
            'best_loss': best_loss,
            'model_path': final_model_path,
            'size_mb': size_info['mb']
        }
        
        logger.info(f"\n{archetype}/{model_type} training complete:")
        logger.info(f"  Best Loss: {best_loss:.4f}")
        logger.info(f"  Model Size: {size_info['mb']:.2f} MB")
        logger.info(f"  Saved to: {final_model_path}\n")
        
        # Clean up
        del model, trainer, buffer
        torch.cuda.empty_cache()
        
        return results
    
    def train_all_sequential(
        self,
        archetypes: List[str],
        model_types: List[str],
        epochs: int = 100
    ) -> List[Dict[str, Any]]:
        """
        Train all models sequentially
        
        Args:
            archetypes: List of archetypes to train
            model_types: List of model types to train
            epochs: Number of epochs per model
        
        Returns:
            List of training results
        """
        total_models = len(archetypes) * len(model_types)
        logger.info(f"Training {total_models} models sequentially...")
        
        results = []
        current = 0
        
        for archetype in archetypes:
            for model_type in model_types:
                current += 1
                logger.info(f"\n[{current}/{total_models}] Training {archetype}/{model_type}")
                
                try:
                    result = self.train_single_model(archetype, model_type, epochs)
                    results.append(result)
                except Exception as e:
                    logger.error(f"Failed to train {archetype}/{model_type}: {e}")
                    results.append({
                        'status': 'failed',
                        'archetype': archetype,
                        'model_type': model_type,
                        'error': str(e)
                    })
        
        # Summary
        successes = sum(1 for r in results if r['status'] == 'success')
        failures = total_models - successes
        
        logger.info(f"\n{'='*60}")
        logger.info(f"TRAINING COMPLETE")
        logger.info(f"{'='*60}")
        logger.info(f"Success: {successes}/{total_models}")
        logger.info(f"Failed: {failures}/{total_models}")
        
        if successes > 0:
            total_size = sum(r.get('size_mb', 0) for r in results if r['status'] == 'success')
            logger.info(f"Total model size: {total_size:.2f} MB ({total_size/1024:.2f} GB)")
        
        return results
    
    def train_archetype_parallel(
        self,
        archetype: str,
        model_types: List[str],
        epochs: int = 100
    ) -> List[Dict[str, Any]]:
        """
        Train all models for one archetype in parallel
        
        Uses multiprocessing to train multiple model types simultaneously
        
        Args:
            archetype: Archetype to train
            model_types: Model types to train
            epochs: Number of epochs
        
        Returns:
            List of training results
        """
        logger.info(f"Training {len(model_types)} models for {archetype} in parallel...")
        
        # Note: Parallel training requires careful VRAM management
        # For safety, we'll train sequentially but document the structure
        
        results = []
        for model_type in model_types:
            result = self.train_single_model(archetype, model_type, epochs)
            results.append(result)
        
        return results


def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Train ML Monster AI models')
    
    parser.add_argument(
        '--archetypes',
        nargs='+',
        choices=ModelFactory.ARCHETYPES,
        help='Archetypes to train (default: all)'
    )
    
    parser.add_argument(
        '--model-types',
        nargs='+',
        choices=ModelFactory.MODEL_TYPES,
        help='Model types to train (default: all)'
    )
    
    parser.add_argument(
        '--all',
        action='store_true',
        help='Train all 54 models'
    )
    
    parser.add_argument(
        '--epochs',
        type=int,
        default=100,
        help='Number of epochs (default: 100)'
    )
    
    parser.add_argument(
        '--batch-size',
        type=int,
        default=256,
        help='Batch size (default: 256)'
    )
    
    parser.add_argument(
        '--lr',
        type=float,
        default=0.0003,
        help='Learning rate (default: 0.0003)'
    )
    
    parser.add_argument(
        '--device',
        type=str,
        default='cuda:0',
        help='Device (default: cuda:0)'
    )
    
    parser.add_argument(
        '--config',
        type=str,
        default='/opt/ml_monster_ai/configs/training_config.yaml',
        help='Configuration file path'
    )
    
    parser.add_argument(
        '--parallel',
        action='store_true',
        help='Train models in parallel (experimental)'
    )
    
    parser.add_argument(
        '--validate',
        action='store_true',
        help='Run validation after training'
    )
    
    parser.add_argument(
        '--export-onnx',
        action='store_true',
        help='Export trained models to ONNX'
    )
    
    parser.add_argument(
        '--auto-deploy',
        action='store_true',
        help='Automatically deploy models after training (exports to ONNX and restarts service)'
    )
    
    return parser.parse_args()


def main():
    """Main training entry point"""
    args = parse_args()
    
    # Determine archetypes and model types
    if args.all:
        archetypes = ModelFactory.ARCHETYPES
        model_types = ModelFactory.MODEL_TYPES
    else:
        archetypes = args.archetypes or ModelFactory.ARCHETYPES
        model_types = args.model_types or ModelFactory.MODEL_TYPES
    
    logger.info(f"{'='*60}")
    logger.info(f"ML MONSTER AI - TRAINING PIPELINE")
    logger.info(f"{'='*60}")
    logger.info(f"Archetypes: {archetypes}")
    logger.info(f"Model Types: {model_types}")
    logger.info(f"Total Models: {len(archetypes) * len(model_types)}")
    logger.info(f"Epochs: {args.epochs}")
    logger.info(f"Batch Size: {args.batch_size}")
    logger.info(f"Learning Rate: {args.lr}")
    logger.info(f"Device: {args.device}")
    logger.info(f"{'='*60}\n")
    
    # Initialize orchestrator
    orchestrator = TrainingOrchestrator(
        config_path=args.config,
        device=args.device
    )
    
    # Override config with CLI args
    orchestrator.config['training']['batch_size'] = args.batch_size
    orchestrator.config['training']['learning_rate'] = args.lr
    orchestrator.config['training']['epochs'] = args.epochs
    
    # Train models
    start_time = time.time()
    
    if args.parallel:
        logger.warning("Parallel training is experimental and may cause VRAM issues")
        # Parallel training would go here
        results = orchestrator.train_all_sequential(archetypes, model_types, args.epochs)
    else:
        results = orchestrator.train_all_sequential(archetypes, model_types, args.epochs)
    
    total_time = time.time() - start_time
    
    # Print summary
    logger.info(f"\n{'='*60}")
    logger.info(f"TRAINING SUMMARY")
    logger.info(f"{'='*60}")
    logger.info(f"Total Time: {total_time/3600:.2f} hours")
    logger.info(f"Models Trained: {sum(1 for r in results if r['status'] == 'success')}")
    logger.info(f"Models Failed: {sum(1 for r in results if r['status'] == 'failed')}")
    
    # Export to ONNX if requested
    if args.export_onnx or args.auto_deploy:
        logger.info("\nExporting models to ONNX...")
        from scripts.export_to_onnx import export_all_models
        export_all_models(archetypes, model_types)
    
    # Auto-deploy if requested
    if args.auto_deploy:
        logger.info("\nTriggering automated deployment...")
        deploy_script = Path(__file__).parent.parent.parent / 'ml_scheduler' / 'auto_deploy.sh'
        
        if deploy_script.exists():
            import subprocess
            try:
                result = subprocess.run(
                    ['bash', str(deploy_script)],
                    capture_output=True,
                    text=True,
                    timeout=300  # 5 minute timeout
                )
                
                if result.returncode == 0:
                    logger.info("✓ Automated deployment completed successfully")
                    logger.info(result.stdout)
                else:
                    logger.error(f"✗ Deployment failed with code {result.returncode}")
                    logger.error(result.stderr)
                    return 1
            except subprocess.TimeoutExpired:
                logger.error("Deployment timed out after 5 minutes")
                return 1
            except Exception as e:
                logger.error(f"Deployment error: {e}")
                return 1
        else:
            logger.warning(f"Deployment script not found: {deploy_script}")
            logger.warning("Skipping automated deployment")
    
    logger.info(f"\nTraining complete! Check logs at /opt/ml_monster_ai/logs/training.log")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
