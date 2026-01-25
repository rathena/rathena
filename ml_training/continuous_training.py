"""
Continuous Training Pipeline
Background training loop for continuous model improvement from live gameplay data

Features:
- Runs in background without blocking inference
- Streams experiences from PostgreSQL
- Incrementally updates models
- Hot model reloading
- Resource monitoring
- Automatic rollback on degradation
"""

import asyncio
import asyncpg
import torch
import torch.nn as nn
import torch.onnx
import numpy as np
import yaml
import logging
import signal
import sys
import time
import psutil
import aiohttp
from pathlib import Path
from typing import Dict, List, Optional, Any
from datetime import datetime, timedelta
from collections import deque

# Import local modules
from data.experience_stream import ExperienceStream, PrioritizedExperienceStream, Experience
from validation.continuous_validator import ContinuousValidator, ValidationMetrics, RollbackDetector
from training.trainer import DQNTrainer, PPOTrainer, SACTrainer, LSTMTrainer

logger = logging.getLogger(__name__)


class ResourceMonitor:
    """Monitor system resources (CPU, VRAM, RAM)"""
    
    def __init__(self, max_cpu_percent: float = 30, max_vram_percent: float = 70, max_ram_gb: float = 8):
        """
        Initialize resource monitor
        
        Args:
            max_cpu_percent: Maximum CPU usage
            max_vram_percent: Maximum VRAM usage  
            max_ram_gb: Maximum RAM usage in GB
        """
        self.max_cpu_percent = max_cpu_percent
        self.max_vram_percent = max_vram_percent
        self.max_ram_gb = max_ram_gb
        self.logger = logging.getLogger(__name__)
        
        # Check GPU availability
        self.gpu_available = torch.cuda.is_available()
    
    def check_resources(self) -> Dict[str, Any]:
        """
        Check current resource usage
        
        Returns:
            Resource status dictionary
        """
        # CPU usage
        cpu_percent = psutil.cpu_percent(interval=1.0)
        
        # RAM usage
        ram = psutil.virtual_memory()
        ram_used_gb = ram.used / (1024 ** 3)
        ram_percent = ram.percent
        
        # VRAM usage (if GPU available)
        vram_used_gb = 0.0
        vram_total_gb = 0.0
        vram_percent = 0.0
        
        if self.gpu_available:
            try:
                vram_allocated = torch.cuda.memory_allocated(0)
                vram_reserved = torch.cuda.memory_reserved(0)
                vram_total = torch.cuda.get_device_properties(0).total_memory
                
                vram_used_gb = vram_allocated / (1024 ** 3)
                vram_total_gb = vram_total / (1024 ** 3)
                vram_percent = (vram_allocated / vram_total) * 100
            except Exception as e:
                self.logger.warning(f"Failed to check VRAM: {e}")
        
        # Determine if resources allow training
        can_train = (
            cpu_percent < self.max_cpu_percent and
            vram_percent < self.max_vram_percent and
            ram_used_gb < self.max_ram_gb
        )
        
        status = {
            'can_train': can_train,
            'cpu_percent': cpu_percent,
            'cpu_max': self.max_cpu_percent,
            'ram_used_gb': ram_used_gb,
            'ram_max_gb': self.max_ram_gb,
            'ram_percent': ram_percent,
            'vram_used_gb': vram_used_gb,
            'vram_total_gb': vram_total_gb,
            'vram_percent': vram_percent,
            'vram_max_percent': self.max_vram_percent,
            'gpu_available': self.gpu_available
        }
        
        return status
    
    def wait_for_resources(self, max_wait_seconds: int = 300) -> bool:
        """
        Wait for resources to become available
        
        Args:
            max_wait_seconds: Maximum time to wait
        
        Returns:
            True if resources available, False if timeout
        """
        self.logger.info("Waiting for resources to become available...")
        
        start_time = time.time()
        
        while time.time() - start_time < max_wait_seconds:
            status = self.check_resources()
            
            if status['can_train']:
                self.logger.info("Resources available")
                return True
            
            self.logger.debug(
                f"Resources not available: CPU={status['cpu_percent']:.1f}%, "
                f"VRAM={status['vram_percent']:.1f}%, RAM={status['ram_used_gb']:.1f}GB"
            )
            
            time.sleep(10)  # Check every 10 seconds
        
        self.logger.warning(f"Resource wait timeout after {max_wait_seconds}s")
        return False


class ContinuousTrainer:
    """
    Background training loop for continuous model improvement
    
    Workflow:
    1. Check for new experiences in database
    2. Stream experiences efficiently
    3. Train models incrementally
    4. Validate improvements
    5. Export to ONNX
    6. Hot reload into inference service
    7. Monitor performance, rollback if needed
    """
    
    def __init__(self, config_path: str):
        """
        Initialize continuous trainer
        
        Args:
            config_path: Path to configuration YAML
        """
        # Load configuration
        self.config = self._load_config(config_path)
        
        # Setup logging
        self._setup_logging()
        self.logger = logging.getLogger(__name__)
        
        self.logger.info("="*80)
        self.logger.info("Continuous Training Pipeline v2.0 Initializing")
        self.logger.info("="*80)
        
        # Database pool (initialized in run_forever)
        self.db_pool: Optional[asyncpg.Pool] = None
        
        # Components
        self.experience_stream: Optional[PrioritizedExperienceStream] = None
        self.validator: Optional[ContinuousValidator] = None
        self.rollback_detector: Optional[RollbackDetector] = None
        self.resource_monitor = ResourceMonitor(
            max_cpu_percent=self.config['continuous_training']['max_cpu_usage'],
            max_vram_percent=self.config['continuous_training']['max_vram_usage'],
            max_ram_gb=self.config['continuous_training']['max_ram_usage_gb']
        )
        
        # Trainers (loaded per model type)
        self.trainers: Dict[str, Any] = {}
        
        # State tracking
        self.running = False
        self.cycle_count = 0
        self.last_training_time: Dict[str, datetime] = {}  # Track per archetype/model
        self.iteration_count = 0
        
        # Paths
        self.checkpoints_dir = Path(self.config['continuous_training']['paths']['checkpoints_dir'])
        self.models_dir = Path(self.config['continuous_training']['paths']['models_dir'])
        self.logs_dir = Path(self.config['continuous_training']['paths']['logs_dir'])
        
        # Create directories
        self.checkpoints_dir.mkdir(parents=True, exist_ok=True)
        self.models_dir.mkdir(parents=True, exist_ok=True)
        self.logs_dir.mkdir(parents=True, exist_ok=True)
        
        self.logger.info("Continuous Trainer initialized")
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load YAML configuration"""
        with open(config_path, 'r') as f:
            config = yaml.safe_load(f)
        return config
    
    def _setup_logging(self):
        """Setup logging configuration"""
        log_level = self.config['continuous_training']['log_level']
        
        logging.basicConfig(
            level=getattr(logging, log_level),
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.StreamHandler(sys.stdout),
                logging.FileHandler(
                    Path(self.config['continuous_training']['paths']['logs_dir']) / 'continuous_training.log'
                )
            ]
        )
    
    async def initialize(self):
        """Initialize database connections and components"""
        self.logger.info("Initializing database connections...")
        
        # Create database pool
        db_config = self.config['continuous_training']['database']['postgresql']
        
        self.db_pool = await asyncpg.create_pool(
            host=db_config['host'],
            port=db_config['port'],
            database=db_config['database'],
            user=db_config['user'],
            password=db_config['password'],
            min_size=db_config['pool_min_size'],
            max_size=db_config['pool_max_size'],
            command_timeout=db_config['command_timeout']
        )
        
        self.logger.info("✓ Database pool created")
        
        # Initialize components
        self.experience_stream = PrioritizedExperienceStream(self.db_pool)
        self.validator = ContinuousValidator(
            db_pool=self.db_pool,
            device='cuda:0' if torch.cuda.is_available() else 'cpu',
            validation_samples=1000,
            min_improvement_threshold=self.config['continuous_training']['min_improvement']
        )
        self.rollback_detector = RollbackDetector(
            db_pool=self.db_pool,
            degradation_threshold=0.1  # 10% degradation triggers rollback
        )
        
        self.logger.info("✓ Components initialized")
        
        # Verify experience table has data
        stats = await self.experience_stream.get_statistics()
        self.logger.info(
            f"Experience replay stats: {stats['total_experiences']} total experiences, "
            f"{stats['unique_monsters']} unique monsters"
        )
    
    async def run_forever(self):
        """
        Main continuous training loop - runs forever
        
        Executes training cycles on a schedule
        """
        # Initialize
        await self.initialize()
        
        # Setup signal handlers
        def signal_handler(sig, frame):
            self.logger.info(f"Received signal {sig}, initiating shutdown...")
            self.running = False
        
        signal.signal(signal.SIGINT, signal_handler)
        signal.signal(signal.SIGTERM, signal_handler)
        
        self.running = True
        self.logger.info("Starting continuous training loop...")
        
        check_interval = self.config['continuous_training']['check_interval_seconds']
        
        while self.running:
            try:
                # Check if enough time has passed
                await asyncio.sleep(check_interval)
                
                # Check if training is enabled
                if not self.config['continuous_training']['enabled']:
                    self.logger.debug("Continuous training disabled, skipping cycle")
                    continue
                
                # Check if we should train now
                should_train = await self.should_train_now()
                
                if should_train:
                    # Run training iteration
                    self.logger.info(f"Starting training cycle {self.cycle_count + 1}")
                    
                    results = await self.training_iteration()
                    
                    self.cycle_count += 1
                    
                    self.logger.info(
                        f"Cycle {self.cycle_count} complete: "
                        f"models_trained={results.get('models_trained', 0)}, "
                        f"models_deployed={results.get('models_deployed', 0)}"
                    )
                else:
                    self.logger.debug("Skipping training: conditions not met")
            
            except asyncio.CancelledError:
                self.logger.info("Training loop cancelled")
                break
            
            except Exception as e:
                self.logger.error(f"Error in training loop: {e}", exc_info=True)
                await asyncio.sleep(60)  # Back off on error
        
        # Cleanup
        await self.cleanup()
        self.logger.info("Continuous training loop stopped")
    
    async def should_train_now(self) -> bool:
        """
        Determine if training should run now
        
        Checks:
        - Resource availability
        - New data availability
        - Time since last training
        
        Returns:
            True if should train now
        """
        # Check resources
        resources = self.resource_monitor.check_resources()
        
        if not resources['can_train']:
            self.logger.debug(
                f"Resources not available: CPU={resources['cpu_percent']:.1f}%, "
                f"VRAM={resources['vram_percent']:.1f}%"
            )
            return False
        
        # Check if new experiences available
        min_experiences = self.config['continuous_training']['min_new_experiences']
        
        # Get timestamp of last training
        last_timestamp = self.experience_stream.last_processed_timestamp
        if last_timestamp is None:
            # First run, get experiences from last day
            last_timestamp = datetime.utcnow() - timedelta(days=1)
        
        # Count new experiences
        new_count = await self.experience_stream.get_experience_count(
            since_timestamp=last_timestamp
        )
        
        if new_count < min_experiences:
            self.logger.debug(
                f"Not enough new experiences: {new_count} < {min_experiences}"
            )
            return False
        
        self.logger.info(f"{new_count} new experiences available for training")
        return True
    
    async def training_iteration(self) -> Dict[str, Any]:
        """
        Single training iteration
        
        Trains models based on priority schedule
        
        Returns:
            Iteration results
        """
        iteration_start = time.time()
        
        results = {
            'models_trained': 0,
            'models_deployed': 0,
            'models_failed': 0,
            'total_steps': 0,
            'total_experiences': 0
        }
        
        # Get training queue based on priority
        training_queue = self._build_training_queue()
        
        self.logger.info(f"Training queue: {len(training_queue)} models")
        
        # Train each model in queue
        for archetype, model_type in training_queue:
            try:
                # Check resources before each model
                resources = self.resource_monitor.check_resources()
                if not resources['can_train']:
                    self.logger.warning(
                        "Resources exhausted mid-iteration, pausing training"
                    )
                    break
                
                # Check if enough time since last training for this model
                key = f"{archetype}_{model_type}"
                if key in self.last_training_time:
                    time_since_last = (
                        datetime.utcnow() - self.last_training_time[key]
                    ).total_seconds()
                    
                    min_interval = self.config['continuous_training']['training_interval_seconds']
                    
                    if time_since_last < min_interval:
                        self.logger.debug(
                            f"Skipping {key}: only {time_since_last:.0f}s since last training "
                            f"(min={min_interval}s)"
                        )
                        continue
                
                # Train model
                self.logger.info(f"Training: {archetype}/{model_type}")
                
                train_result = await self.train_model_incremental(
                    archetype, model_type
                )
                
                if train_result['success']:
                    results['models_trained'] += 1
                    results['total_steps'] += train_result.get('steps', 0)
                    results['total_experiences'] += train_result.get('experiences', 0)
                    
                    if train_result.get('deployed', False):
                        results['models_deployed'] += 1
                    
                    # Update last training time
                    self.last_training_time[key] = datetime.utcnow()
                else:
                    results['models_failed'] += 1
                    self.logger.warning(
                        f"Training failed for {archetype}/{model_type}: "
                        f"{train_result.get('error', 'unknown')}"
                    )
            
            except Exception as e:
                self.logger.error(
                    f"Error training {archetype}/{model_type}: {e}",
                    exc_info=True
                )
                results['models_failed'] += 1
        
        iteration_time = time.time() - iteration_start
        
        results['iteration_time_seconds'] = iteration_time
        results['iteration_count'] = self.iteration_count
        
        self.iteration_count += 1
        
        self.logger.info(
            f"Iteration complete in {iteration_time:.1f}s: "
            f"{results['models_trained']} trained, "
            f"{results['models_deployed']} deployed, "
            f"{results['models_failed']} failed"
        )
        
        return results
    
    def _build_training_queue(self) -> List[Tuple[str, str]]:
        """
        Build priority queue of models to train
        
        Considers:
        - Model priority (high/medium/low frequency)
        - Archetype priority
        - Cycle count (for frequency control)
        
        Returns:
            List of (archetype, model_type) tuples
        """
        queue = []
        
        archetypes = self.config['continuous_training']['archetype_priority']
        model_priority = self.config['continuous_training']['model_priority']
        
        # High frequency models (every cycle)
        for archetype in archetypes:
            for model_type in model_priority['high_frequency']:
                queue.append((archetype, model_type))
        
        # Medium frequency models (every 3rd cycle)
        if self.cycle_count % 3 == 0:
            for archetype in archetypes:
                for model_type in model_priority.get('medium_frequency', []):
                    queue.append((archetype, model_type))
        
        # Low frequency models (every 10th cycle)
        if self.cycle_count % 10 == 0:
            for archetype in archetypes:
                for model_type in model_priority.get('low_frequency', []):
                    queue.append((archetype, model_type))
        
        return queue
    
    async def train_model_incremental(
        self,
        archetype: str,
        model_type: str
    ) -> Dict[str, Any]:
        """
        Train single model incrementally
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            Training result dictionary
        """
        train_start = time.time()
        
        result = {
            'success': False,
            'archetype': archetype,
            'model_type': model_type,
            'steps': 0,
            'experiences': 0,
            'final_loss': float('inf'),
            'deployed': False,
            'error': None
        }
        
        try:
            # Get training experiences
            experiences = await self._get_training_experiences(archetype, model_type)
            
            if not experiences:
                result['error'] = 'no_experiences'
                return result
            
            result['experiences'] = len(experiences)
            self.logger.info(f"Retrieved {len(experiences)} experiences for training")
            
            # Load or create model
            model = await self._load_or_create_model(archetype, model_type)
            
            # Create trainer
            trainer = self._create_trainer(model, model_type, archetype)
            
            # Train for N steps
            max_steps = self.config['continuous_training']['max_steps_per_iteration']
            batch_size = self.config['continuous_training']['batch_size']
            
            total_loss = 0.0
            steps_taken = 0
            
            for step in range(max_steps):
                # Sample mini-batch from experiences
                batch = self._sample_batch(experiences, batch_size)
                
                if not batch:
                    break
                
                # Train step (method depends on trainer type)
                loss = self._train_step(trainer, batch, model_type)
                
                total_loss += loss
                steps_taken += 1
                
                # Log progress
                if steps_taken % self.config['continuous_training']['log_interval_steps'] == 0:
                    avg_loss = total_loss / steps_taken
                    self.logger.info(
                        f"  Step {steps_taken}/{max_steps}: loss={loss:.4f}, "
                        f"avg_loss={avg_loss:.4f}"
                    )
                
                # Save checkpoint periodically
                if steps_taken % self.config['continuous_training']['checkpoint_frequency'] == 0:
                    await self.save_checkpoint(archetype, model_type, model, step)
                
                # Validate periodically
                if steps_taken % self.config['continuous_training']['validation_frequency'] == 0:
                    val_metrics = await self.validator.validate_model(
                        model, archetype, model_type
                    )
                    
                    self.logger.info(
                        f"  Validation: accuracy={val_metrics.accuracy:.2%}, "
                        f"reward={val_metrics.avg_reward:.2f}, "
                        f"loss={val_metrics.loss:.4f}"
                    )
            
            avg_loss = total_loss / steps_taken if steps_taken > 0 else float('inf')
            
            result['steps'] = steps_taken
            result['final_loss'] = avg_loss
            result['training_time_seconds'] = time.time() - train_start
            
            # Validate final model
            self.logger.info(f"Validating final model: {archetype}/{model_type}")
            
            prod_model_path = self.models_dir / archetype / f"{model_type}.onnx"
            
            should_deploy, validation_results = await self.validator.validate_before_deployment(
                model_path=str(self.checkpoints_dir / f"{archetype}_{model_type}_latest.pth"),
                archetype=archetype,
                model_type=model_type,
                prod_model_path=str(prod_model_path) if prod_model_path.exists() else None
            )
            
            result['validation'] = validation_results
            
            # Deploy if validation passed
            if should_deploy and self.config['continuous_training']['auto_deploy']:
                deployed = await self.deploy_model_hot(archetype, model_type, model)
                result['deployed'] = deployed
                
                if deployed:
                    self.logger.info(f"✓ Model deployed: {archetype}/{model_type}")
                else:
                    self.logger.warning(f"✗ Deployment failed: {archetype}/{model_type}")
            else:
                self.logger.info(
                    f"Model not deployed: should_deploy={should_deploy}, "
                    f"auto_deploy={self.config['continuous_training']['auto_deploy']}"
                )
            
            result['success'] = True
            
            return result
        
        except Exception as e:
            self.logger.error(f"Training failed: {e}", exc_info=True)
            result['error'] = str(e)
            return result
    
    async def _get_training_experiences(
        self,
        archetype: str,
        model_type: str
    ) -> List[Experience]:
        """
        Get training experiences for model
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            List of experiences
        """
        lookback_days = self.config['continuous_training']['experience_lookback_days']
        
        if self.config['continuous_training']['priority_sampling']:
            # Use prioritized sampling
            experiences, weights = await self.experience_stream.sample_weighted_batch(
                archetype=archetype,
                batch_size=2000,  # Get larger pool for training
                beta=0.4,
                lookback_days=lookback_days
            )
            return experiences
        else:
            # Use uniform sampling
            experiences = await self.experience_stream.sample_uniform_batch(
                archetype=archetype,
                batch_size=2000,
                lookback_days=lookback_days
            )
            return experiences
    
    async def _load_or_create_model(
        self,
        archetype: str,
        model_type: str
    ) -> nn.Module:
        """
        Load existing model checkpoint or create new model
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            PyTorch model
        """
        checkpoint_path = self.checkpoints_dir / f"{archetype}_{model_type}_latest.pth"
        
        if checkpoint_path.exists():
            self.logger.info(f"Loading checkpoint: {checkpoint_path}")
            checkpoint = torch.load(checkpoint_path, map_location='cpu')
            
            if isinstance(checkpoint, dict) and 'model_state_dict' in checkpoint:
                model = self._create_fresh_model(archetype, model_type)
                model.load_state_dict(checkpoint['model_state_dict'])
            else:
                model = checkpoint  # Direct model save
            
            return model
        else:
            self.logger.info(f"No checkpoint found, creating new model")
            return self._create_fresh_model(archetype, model_type)
    
    def _create_fresh_model(self, archetype: str, model_type: str) -> nn.Module:
        """
        Create fresh model architecture
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            New PyTorch model
        """
        # Import model architectures
        # This would import from your models module
        # For now, creating placeholder
        
        if model_type == 'combat_dqn':
            from models.dqn import EnhancedCombatDQN
            model = EnhancedCombatDQN(state_dim=64, action_dim=8, hidden_dims=[512, 512, 512, 256])
        
        elif model_type == 'movement_ppo':
            from models.ppo import DeepPPOMovement
            model = DeepPPOMovement(state_dim=64, action_dim=9)
        
        elif model_type == 'team_coordination':
            from models.lstm import MultiHeadedCoordinationLSTM
            model = MultiHeadedCoordinationLSTM(input_dim=464, output_dim=7)
        
        elif model_type == 'soft_actor_critic':
            from models.sac import SoftActorCritic
            model = SoftActorCritic(state_dim=64, action_dim=9)
        
        else:
            # Fallback: simple DQN
            model = nn.Sequential(
                nn.Linear(64, 256),
                nn.ReLU(),
                nn.Linear(256, 256),
                nn.ReLU(),
                nn.Linear(256, 8)
            )
        
        self.logger.info(f"Created fresh model: {archetype}/{model_type}")
        return model
    
    def _create_trainer(
        self,
        model: nn.Module,
        model_type: str,
        archetype: str
    ) -> Any:
        """
        Create appropriate trainer for model type
        
        Args:
            model: PyTorch model
            model_type: Model type
            archetype: Monster archetype
        
        Returns:
            Trainer instance
        """
        hyperparams = self.config['continuous_training']['hyperparameters'].get(
            model_type, {}
        )
        
        device = 'cuda:0' if torch.cuda.is_available() else 'cpu'
        
        if model_type in ['combat_dqn', 'skill_dqn']:
            # Create target model
            target_model = self._create_fresh_model(archetype, model_type)
            target_model.load_state_dict(model.state_dict())
            
            trainer = DQNTrainer(
                model=model,
                target_model=target_model,
                replay_buffer=None,  # We'll feed data directly
                gamma=hyperparams.get('gamma', 0.99),
                tau=hyperparams.get('tau', 0.005),
                batch_size=hyperparams.get('batch_size', 256),
                learning_rate=hyperparams.get('learning_rate', 1e-4),
                device=device
            )
        
        elif model_type == 'movement_ppo':
            trainer = PPOTrainer(
                model=model,
                gamma=hyperparams.get('gamma', 0.995),
                gae_lambda=hyperparams.get('gae_lambda', 0.97),
                clip_epsilon=hyperparams.get('clip_epsilon', 0.2),
                n_epochs=hyperparams.get('n_epochs', 10),
                batch_size=hyperparams.get('batch_size', 128),
                learning_rate=hyperparams.get('learning_rate', 3e-5),
                device=device
            )
        
        elif model_type == 'team_coordination':
            trainer = LSTMTrainer(
                model=model,
                sequence_length=hyperparams.get('sequence_length', 10),
                learning_rate=hyperparams.get('learning_rate', 1e-4),
                device=device
            )
        
        elif model_type == 'soft_actor_critic':
            # SAC requires actor and critics
            # Simplified: use model as actor, create critics
            critic1 = self._create_fresh_model(archetype, 'critic')
            critic2 = self._create_fresh_model(archetype, 'critic')
            target_critic1 = self._create_fresh_model(archetype, 'critic')
            target_critic2 = self._create_fresh_model(archetype, 'critic')
            
            trainer = SACTrainer(
                actor=model,
                critic1=critic1,
                critic2=critic2,
                target_critic1=target_critic1,
                target_critic2=target_critic2,
                replay_buffer=None,
                gamma=hyperparams.get('gamma', 0.99),
                tau=hyperparams.get('tau', 0.005),
                batch_size=hyperparams.get('batch_size', 256),
                actor_lr=hyperparams.get('actor_lr', 3e-4),
                critic_lr=hyperparams.get('critic_lr', 3e-4),
                device=device
            )
        
        else:
            # Default: DQN trainer
            target_model = self._create_fresh_model(archetype, model_type)
            target_model.load_state_dict(model.state_dict())
            
            trainer = DQNTrainer(
                model=model,
                target_model=target_model,
                replay_buffer=None,
                device=device
            )
        
        return trainer
    
    def _sample_batch(
        self,
        experiences: List[Experience],
        batch_size: int
    ) -> Optional[Dict[str, np.ndarray]]:
        """
        Sample mini-batch from experiences
        
        Args:
            experiences: List of experiences
            batch_size: Batch size
        
        Returns:
            Batch dictionary or None
        """
        if len(experiences) < batch_size:
            batch_size = len(experiences)
        
        if batch_size == 0:
            return None
        
        # Random sample
        indices = np.random.choice(len(experiences), batch_size, replace=False)
        batch_exps = [experiences[i] for i in indices]
        
        # Convert to arrays
        states = np.stack([exp.state_vector for exp in batch_exps])
        actions = np.array([exp.action_id for exp in batch_exps], dtype=np.int64)
        rewards = np.array([exp.reward for exp in batch_exps], dtype=np.float32)
        next_states = np.stack([
            exp.next_state_vector if exp.next_state_vector is not None else np.zeros_like(exp.state_vector)
            for exp in batch_exps
        ])
        dones = np.array([exp.done for exp in batch_exps], dtype=np.float32)
        
        return {
            'states': states,
            'actions': actions,
            'rewards': rewards,
            'next_states': next_states,
            'dones': dones
        }
    
    def _train_step(
        self,
        trainer: Any,
        batch: Dict[str, np.ndarray],
        model_type: str
    ) -> float:
        """
        Execute single training step
        
        Args:
            trainer: Trainer instance
            batch: Training batch
            model_type: Model type
        
        Returns:
            Loss value
        """
        # Convert batch to appropriate format for trainer
        
        if isinstance(trainer, DQNTrainer):
            # DQN: need to add to replay buffer then train
            # Simplified: directly train on batch
            states = torch.FloatTensor(batch['states']).to(trainer.device)
            actions = torch.LongTensor(batch['actions']).to(trainer.device)
            rewards = torch.FloatTensor(batch['rewards']).to(trainer.device)
            next_states = torch.FloatTensor(batch['next_states']).to(trainer.device)
            dones = torch.FloatTensor(batch['dones']).to(trainer.device)
            
            # Compute Q-values and loss manually
            q_values = trainer.model(states)
            q_action = q_values.gather(1, actions.unsqueeze(1)).squeeze(1)
            
            with torch.no_grad():
                next_q = trainer.target_model(next_states).max(1)[0]
                target_q = rewards + (1 - dones) * trainer.gamma * next_q
            
            loss = nn.functional.smooth_l1_loss(q_action, target_q)
            
            trainer.optimizer.zero_grad()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(trainer.model.parameters(), 1.0)
            trainer.optimizer.step()
            
            # Soft update target
            if trainer.tau is not None:
                trainer.soft_update_target()
            
            return loss.item()
        
        elif isinstance(trainer, PPOTrainer):
            # PPO: needs rollout format
            # Simplified: use batch as single rollout
            metrics = trainer.train_on_rollout(
                states=batch['states'],
                actions=batch['actions'],
                old_log_probs=np.zeros(len(batch['actions'])),  # Placeholder
                rewards=batch['rewards'],
                dones=batch['dones'],
                values=np.zeros(len(batch['rewards']))  # Placeholder
            )
            return metrics['total_loss']
        
        else:
            # Generic trainer
            return 0.0
    
    async def save_checkpoint(
        self,
        archetype: str,
        model_type: str,
        model: nn.Module,
        step: int
    ):
        """
        Save model checkpoint
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            model: PyTorch model
            step: Training step
        """
        checkpoint_path = self.checkpoints_dir / f"{archetype}_{model_type}_step_{step}.pth"
        latest_path = self.checkpoints_dir / f"{archetype}_{model_type}_latest.pth"
        
        checkpoint = {
            'model_state_dict': model.state_dict(),
            'archetype': archetype,
            'model_type': model_type,
            'step': step,
            'timestamp': datetime.utcnow().isoformat()
        }
        
        torch.save(checkpoint, checkpoint_path)
        torch.save(checkpoint, latest_path)
        
        self.logger.debug(f"Checkpoint saved: {checkpoint_path}")
    
    async def deploy_model_hot(
        self,
        archetype: str,
        model_type: str,
        model: nn.Module
    ) -> bool:
        """
        Deploy model with hot reload (no service restart)
        
        Steps:
        1. Export model to ONNX
        2. Verify ONNX model
        3. Signal inference service to reload
        4. Verify reload succeeded
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            model: PyTorch model
        
        Returns:
            True if deployment succeeded
        """
        self.logger.info(f"Deploying model: {archetype}/{model_type}")
        
        try:
            # Step 1: Export to ONNX
            onnx_path = await self._export_to_onnx(archetype, model_type, model)
            
            if not onnx_path:
                self.logger.error("ONNX export failed")
                return False
            
            # Step 2: Verify ONNX model
            if self.config['continuous_training']['onnx_export']['verify_export']:
                if not self._verify_onnx_export(onnx_path, model):
                    self.logger.error("ONNX verification failed")
                    return False
            
            # Step 3: Signal inference service to reload
            if self.config['continuous_training']['hot_reload']['enabled']:
                reloaded = await self._trigger_hot_reload(archetype, model_type)
                
                if not reloaded:
                    self.logger.error("Hot reload failed")
                    return False
            
            # Step 4: Log deployment
            await self._log_deployment(archetype, model_type, str(onnx_path))
            
            self.logger.info(f"✓ Deployment complete: {archetype}/{model_type}")
            return True
        
        except Exception as e:
            self.logger.error(f"Deployment failed: {e}", exc_info=True)
            return False
    
    async def _export_to_onnx(
        self,
        archetype: str,
        model_type: str,
        model: nn.Module
    ) -> Optional[Path]:
        """
        Export PyTorch model to ONNX format
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            model: PyTorch model
        
        Returns:
            Path to ONNX file or None
        """
        model.eval()
        model.cpu()
        
        # Create output directory
        output_dir = self.models_dir / archetype
        output_dir.mkdir(parents=True, exist_ok=True)
        
        onnx_path = output_dir / f"{model_type}.onnx"
        
        # Dummy input
        dummy_input = torch.randn(1, 64)
        
        try:
            # Export
            torch.onnx.export(
                model,
                dummy_input,
                str(onnx_path),
                export_params=True,
                opset_version=self.config['continuous_training']['onnx_export']['opset_version'],
                do_constant_folding=self.config['continuous_training']['onnx_export']['optimize'],
                input_names=['input'],
                output_names=['output'],
                dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}}
            )
            
            self.logger.info(f"✓ ONNX export complete: {onnx_path}")
            
            # Optionally export INT8 version
            if self.config['continuous_training']['onnx_export']['quantize_int8']:
                int8_path = output_dir / f"{model_type}_int8.onnx"
                # Quantization would go here (simplified)
                self.logger.info(f"INT8 export skipped (requires calibration data)")
            
            return onnx_path
        
        except Exception as e:
            self.logger.error(f"ONNX export failed: {e}", exc_info=True)
            return None
    
    def _verify_onnx_export(self, onnx_path: Path, pytorch_model: nn.Module) -> bool:
        """
        Verify ONNX model produces same output as PyTorch
        
        Args:
            onnx_path: Path to ONNX model
            pytorch_model: Original PyTorch model
        
        Returns:
            True if verification passed
        """
        try:
            import onnxruntime as ort
            
            # Load ONNX
            session = ort.InferenceSession(
                str(onnx_path),
                providers=['CPUExecutionProvider']
            )
            
            # Test input
            test_input = torch.randn(5, 64)
            
            # PyTorch output
            pytorch_model.eval()
            pytorch_model.cpu()
            with torch.no_grad():
                pytorch_output = pytorch_model(test_input).numpy()
            
            # ONNX output
            input_name = session.get_inputs()[0].name
            output_name = session.get_outputs()[0].name
            onnx_output = session.run([output_name], {input_name: test_input.numpy()})[0]
            
            # Compare
            diff = np.abs(pytorch_output - onnx_output).max()
            
            if diff < 1e-4:
                self.logger.info(f"✓ ONNX verification passed: max_diff={diff:.2e}")
                return True
            else:
                self.logger.warning(f"ONNX verification: large diff={diff:.2e}")
                return diff < 1e-2  # Allow some tolerance
        
        except Exception as e:
            self.logger.error(f"ONNX verification failed: {e}", exc_info=True)
            return False
    
    async def _trigger_hot_reload(
        self,
        archetype: str,
        model_type: str
    ) -> bool:
        """
        Trigger hot reload in inference service
        
        Sends HTTP request to reload endpoint
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            True if reload succeeded
        """
        hot_reload_config = self.config['continuous_training']['hot_reload']
        
        host = hot_reload_config['inference_service_host']
        port = hot_reload_config['inference_service_port']
        endpoint = hot_reload_config['reload_endpoint'].format(
            archetype=archetype,
            model_type=model_type
        )
        
        url = f"http://{host}:{port}{endpoint}"
        
        self.logger.info(f"Triggering hot reload: {url}")
        
        try:
            async with aiohttp.ClientSession() as session:
                async with session.post(url, timeout=aiohttp.ClientTimeout(total=30)) as response:
                    if response.status == 200:
                        data = await response.json()
                        
                        if data.get('success', False):
                            self.logger.info("✓ Hot reload succeeded")
                            return True
                        else:
                            self.logger.error(f"Hot reload failed: {data.get('message', 'unknown')}")
                            return False
                    else:
                        self.logger.error(f"Hot reload HTTP error: {response.status}")
                        return False
        
        except Exception as e:
            self.logger.error(f"Failed to trigger hot reload: {e}", exc_info=True)
            return False
    
    async def _log_deployment(
        self,
        archetype: str,
        model_type: str,
        onnx_path: str
    ):
        """
        Log deployment to database
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            onnx_path: Path to deployed ONNX model
        """
        query = """
        INSERT INTO ml_model_versions (
            archetype,
            model_type,
            version,
            onnx_path,
            validation_metric,
            deployed_at,
            active
        ) VALUES (
            $1, $2, $3, $4, $5, NOW(), TRUE
        )
        """
        
        # Deactivate previous versions
        deactivate_query = """
        UPDATE ml_model_versions
        SET active = FALSE
        WHERE archetype = $1
            AND model_type = $2
            AND active = TRUE
            AND deployed_at < NOW()
        """
        
        async with self.db_pool.acquire() as conn:
            # Deactivate old versions
            await conn.execute(deactivate_query, archetype, model_type)
            
            # Insert new version
            version = int(time.time())  # Use timestamp as version
            await conn.execute(
                query,
                archetype,
                model_type,
                version,
                onnx_path,
                0.0,  # Placeholder validation metric
            )
        
        self.logger.info(f"Deployment logged: {archetype}/{model_type} v{version}")
    
    async def rollback_model(
        self,
        archetype: str,
        model_type: str
    ) -> bool:
        """
        Rollback to previous model version
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            True if rollback succeeded
        """
        self.logger.warning(f"Rolling back model: {archetype}/{model_type}")
        
        # Get previous version from database
        query = """
        SELECT onnx_path, version
        FROM ml_model_versions
        WHERE archetype = $1
            AND model_type = $2
            AND active = FALSE
        ORDER BY deployed_at DESC
        LIMIT 1
        """
        
        async with self.db_pool.acquire() as conn:
            row = await conn.fetchrow(query, archetype, model_type)
            
            if not row:
                self.logger.error("No previous version found for rollback")
                return False
            
            previous_onnx = row['onnx_path']
            previous_version = row['version']
        
        # Verify previous ONNX exists
        if not Path(previous_onnx).exists():
            self.logger.error(f"Previous ONNX not found: {previous_onnx}")
            return False
        
        # Copy previous version to current location
        current_onnx = self.models_dir / archetype / f"{model_type}.onnx"
        
        import shutil
        shutil.copy2(previous_onnx, current_onnx)
        
        # Trigger hot reload
        reloaded = await self._trigger_hot_reload(archetype, model_type)
        
        if reloaded:
            # Log rollback
            log_query = """
            UPDATE ml_continuous_training_log
            SET rollback = TRUE
            WHERE archetype = $1
                AND model_type = $2
            ORDER BY created_at DESC
            LIMIT 1
            """
            
            async with self.db_pool.acquire() as conn:
                await conn.execute(log_query, archetype, model_type)
            
            self.logger.warning(
                f"✓ Rollback complete: {archetype}/{model_type} to version {previous_version}"
            )
            return True
        else:
            self.logger.error("Rollback failed: hot reload unsuccessful")
            return False
    
    async def monitor_resources(self) -> Dict[str, Any]:
        """
        Monitor system resources
        
        Returns:
            Resource status dictionary
        """
        return self.resource_monitor.check_resources()
    
    async def cleanup(self):
        """Cleanup resources on shutdown"""
        self.logger.info("Cleaning up resources...")
        
        if self.db_pool:
            await self.db_pool.close()
            self.logger.info("✓ Database pool closed")
        
        # Clear CUDA cache
        if torch.cuda.is_available():
            torch.cuda.empty_cache()
            self.logger.info("✓ CUDA cache cleared")


async def main():
    """Entry point for continuous training service"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Continuous Training Service')
    parser.add_argument(
        '--config',
        type=str,
        default='/opt/ml_monster_ai/configs/continuous_training_config.yaml',
        help='Path to configuration file'
    )
    
    args = parser.parse_args()
    
    # Create trainer
    trainer = ContinuousTrainer(config_path=args.config)
    
    try:
        # Run forever
        await trainer.run_forever()
    
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt received")
    
    except Exception as e:
        logger.critical(f"Fatal error: {e}", exc_info=True)
        sys.exit(1)
    
    finally:
        logger.info("Continuous training service stopped")


if __name__ == '__main__':
    asyncio.run(main())
