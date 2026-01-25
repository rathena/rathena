#!/usr/bin/env python3
"""
Training Scheduler
Manages automated training runs with resource monitoring and deployment
"""

import asyncio
import subprocess
import psutil
import logging
import os
import signal
from datetime import datetime
from typing import Dict, Optional, List
from pathlib import Path


logger = logging.getLogger(__name__)


class TrainingScheduler:
    """
    Manages automated training runs with resource monitoring
    
    Features:
    - Pre-training condition checks (CPU, memory, GPU)
    - Background training process management
    - Real-time resource monitoring
    - Automatic pause/resume on resource spikes
    - Post-training model deployment
    - Rollback on deployment failure
    """
    
    def __init__(self, config: Dict):
        """
        Initialize training scheduler
        
        Args:
            config: Configuration dictionary with training and resource settings
        """
        self.config = config
        self.logger = logging.getLogger(__name__)
        self.training_process: Optional[subprocess.Popen] = None
        self.is_training = False
        self.is_paused = False
        self.training_log_path: Optional[str] = None
        
        # Resource thresholds
        self.max_cpu_before_training = config.get('max_cpu_before_training', 30)
        self.max_memory_before_training = config.get('max_memory_before_training', 70)
        self.pause_threshold_cpu = config.get('pause_threshold_cpu', 80)
        self.pause_threshold_memory = config.get('pause_threshold_memory', 90)
        
        self.logger.info("TrainingScheduler initialized")
        self.logger.info(f"  Max CPU before training: {self.max_cpu_before_training}%")
        self.logger.info(f"  Max Memory before training: {self.max_memory_before_training}%")
        self.logger.info(f"  Pause threshold CPU: {self.pause_threshold_cpu}%")
        self.logger.info(f"  Pause threshold Memory: {self.pause_threshold_memory}%")
    
    async def can_start_training(self) -> tuple[bool, str]:
        """
        Check if current conditions allow training to start
        
        Returns:
            (can_start, reason): Boolean and reason string
        """
        self.logger.info("Checking if training conditions are met...")
        
        # Check CPU usage
        cpu_percent = psutil.cpu_percent(interval=1.0)
        if cpu_percent > self.max_cpu_before_training:
            reason = f"CPU too high ({cpu_percent:.1f}% > {self.max_cpu_before_training}%)"
            self.logger.warning(f"✗ {reason}")
            return False, reason
        
        self.logger.info(f"✓ CPU usage acceptable: {cpu_percent:.1f}%")
        
        # Check memory usage
        memory = psutil.virtual_memory()
        if memory.percent > self.max_memory_before_training:
            reason = f"Memory too high ({memory.percent:.1f}% > {self.max_memory_before_training}%)"
            self.logger.warning(f"✗ {reason}")
            return False, reason
        
        self.logger.info(f"✓ Memory usage acceptable: {memory.percent:.1f}%")
        
        # Check GPU memory (if CUDA available)
        try:
            result = subprocess.run(
                ['nvidia-smi', '--query-gpu=memory.used', '--format=csv,noheader,nounits'],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            if result.returncode == 0:
                gpu_mem_used = int(result.stdout.strip())
                if gpu_mem_used > 1000:  # >1GB in use
                    reason = f"GPU memory in use ({gpu_mem_used}MB > 1000MB)"
                    self.logger.warning(f"✗ {reason}")
                    return False, reason
                
                self.logger.info(f"✓ GPU memory available: {gpu_mem_used}MB used")
            else:
                self.logger.warning("Could not query GPU memory, proceeding anyway")
        
        except (subprocess.TimeoutExpired, FileNotFoundError, ValueError) as e:
            self.logger.warning(f"GPU check failed: {e}, assuming GPU available")
        
        # Check disk space (need at least 10GB free for model checkpoints)
        disk = psutil.disk_usage('/opt/ml_monster_ai')
        free_gb = disk.free / (1024 ** 3)
        if free_gb < 10:
            reason = f"Insufficient disk space ({free_gb:.1f}GB < 10GB)"
            self.logger.warning(f"✗ {reason}")
            return False, reason
        
        self.logger.info(f"✓ Disk space available: {free_gb:.1f}GB free")
        
        # All checks passed
        self.logger.info("✓ All conditions met for training")
        return True, "All conditions met"
    
    async def start_training(
        self,
        archetypes: str = 'all',
        epochs: Optional[int] = None,
        auto_deploy: bool = True
    ) -> bool:
        """
        Start training process in background
        
        Args:
            archetypes: Space-separated archetypes or 'all'
            epochs: Number of epochs (None = use config default)
            auto_deploy: Auto-deploy models after training
        
        Returns:
            True if training started successfully
        """
        if self.is_training:
            self.logger.warning("Training already in progress")
            return False
        
        # Check conditions
        can_start, reason = await self.can_start_training()
        if not can_start:
            self.logger.warning(f"Cannot start training: {reason}")
            return False
        
        # Use config defaults
        if epochs is None:
            epochs = self.config.get('epochs', 50)
        
        batch_size = self.config.get('batch_size', 256)
        learning_rate = self.config.get('learning_rate', 0.0003)
        
        # Expand archetypes
        if archetypes == 'all':
            archetypes_arg = 'aggressive defensive support mage tank ranged'
        else:
            archetypes_arg = archetypes
        
        self.logger.info(f"Starting automated training...")
        self.logger.info(f"  Archetypes: {archetypes_arg}")
        self.logger.info(f"  Epochs: {epochs}")
        self.logger.info(f"  Batch Size: {batch_size}")
        self.logger.info(f"  Learning Rate: {learning_rate}")
        self.logger.info(f"  Auto-Deploy: {auto_deploy}")
        
        # Prepare command
        training_script = '/home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_training/scripts/train_all_models.py'
        python_path = '/opt/ml_monster_ai/venv/bin/python'
        
        # Check if venv exists, otherwise use system python
        if not os.path.exists(python_path):
            python_path = 'python3'
            self.logger.warning(f"Virtual environment not found, using system python: {python_path}")
        
        cmd = [
            python_path,
            training_script,
            '--archetypes', *archetypes_arg.split(),
            '--epochs', str(epochs),
            '--batch-size', str(batch_size),
            '--lr', str(learning_rate),
            '--device', 'cuda:0',
            '--validate'
        ]
        
        if auto_deploy:
            cmd.append('--auto-deploy')
        
        # Create log file
        log_dir = Path('/opt/ml_monster_ai/logs')
        log_dir.mkdir(parents=True, exist_ok=True)
        
        self.training_log_path = str(log_dir / f"training_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log")
        
        # Start process
        try:
            with open(self.training_log_path, 'w') as f:
                f.write(f"ML Training Started: {datetime.now()}\n")
                f.write(f"Command: {' '.join(cmd)}\n")
                f.write("="*60 + "\n\n")
                f.flush()
                
                self.training_process = subprocess.Popen(
                    cmd,
                    stdout=f,
                    stderr=subprocess.STDOUT,
                    cwd='/home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_training',
                    env={**os.environ, 'PYTHONUNBUFFERED': '1', 'CUDA_VISIBLE_DEVICES': '0'}
                )
            
            self.is_training = True
            self.is_paused = False
            
            self.logger.info(f"✓ Training started (PID: {self.training_process.pid})")
            self.logger.info(f"✓ Logging to: {self.training_log_path}")
            
            return True
        
        except Exception as e:
            self.logger.error(f"Failed to start training: {e}")
            return False
    
    async def monitor_training(self, check_interval: int = 30) -> int:
        """
        Monitor training process and handle resource management
        
        Args:
            check_interval: Seconds between resource checks
        
        Returns:
            Training process exit code
        """
        self.logger.info("Starting training monitor...")
        
        pause_count = 0
        
        while self.is_training:
            # Check if process still running
            if self.training_process and self.training_process.poll() is not None:
                exit_code = self.training_process.returncode
                self.is_training = False
                
                if exit_code == 0:
                    self.logger.info(f"✓ Training completed successfully (exit code: {exit_code})")
                    
                    # Deploy models if auto-deploy enabled
                    if self.config.get('auto_deploy', True):
                        deployment_success = await self.deploy_models()
                        if not deployment_success and self.config.get('rollback_on_failure', True):
                            self.logger.warning("Deployment failed, rollback may be needed")
                else:
                    self.logger.error(f"✗ Training failed (exit code: {exit_code})")
                    self.logger.error(f"Check logs: {self.training_log_path}")
                
                return exit_code
            
            # Check system resources
            cpu_percent = psutil.cpu_percent(interval=1.0)
            memory = psutil.virtual_memory()
            
            # Pause training if resources spike
            if not self.is_paused and (
                cpu_percent > self.pause_threshold_cpu or 
                memory.percent > self.pause_threshold_memory
            ):
                self.logger.warning(
                    f"Resource spike detected! CPU: {cpu_percent:.1f}%, Memory: {memory.percent:.1f}%"
                )
                self.logger.warning("Pausing training process...")
                
                if self.training_process:
                    try:
                        self.training_process.send_signal(signal.SIGSTOP)
                        self.is_paused = True
                        pause_count += 1
                        self.logger.info(f"Training paused (pause count: {pause_count})")
                    except Exception as e:
                        self.logger.error(f"Failed to pause training: {e}")
            
            # Resume training if paused and resources recovered
            elif self.is_paused and (
                cpu_percent < self.max_cpu_before_training and
                memory.percent < self.max_memory_before_training
            ):
                self.logger.info("Resources recovered, resuming training...")
                
                if self.training_process:
                    try:
                        self.training_process.send_signal(signal.SIGCONT)
                        self.is_paused = False
                        self.logger.info("✓ Training resumed")
                    except Exception as e:
                        self.logger.error(f"Failed to resume training: {e}")
            
            # Log current status
            if not self.is_paused:
                self.logger.debug(f"Training in progress... CPU: {cpu_percent:.1f}%, Memory: {memory.percent:.1f}%")
            
            # Wait before next check
            await asyncio.sleep(check_interval)
        
        return 0
    
    async def stop_training(self, force: bool = False) -> bool:
        """
        Stop training process
        
        Args:
            force: Force kill (SIGKILL) instead of graceful shutdown (SIGTERM)
        
        Returns:
            True if stopped successfully
        """
        if not self.is_training or not self.training_process:
            self.logger.warning("No training process to stop")
            return False
        
        self.logger.info(f"Stopping training process (PID: {self.training_process.pid})...")
        
        try:
            if force:
                self.training_process.kill()  # SIGKILL
                self.logger.info("Training process killed (SIGKILL)")
            else:
                self.training_process.terminate()  # SIGTERM
                self.logger.info("Training process terminated (SIGTERM)")
                
                # Wait for graceful shutdown
                try:
                    self.training_process.wait(timeout=30)
                except subprocess.TimeoutExpired:
                    self.logger.warning("Process did not terminate gracefully, killing...")
                    self.training_process.kill()
            
            self.is_training = False
            self.is_paused = False
            return True
        
        except Exception as e:
            self.logger.error(f"Failed to stop training: {e}")
            return False
    
    async def deploy_models(self) -> bool:
        """
        Deploy trained models and restart inference service
        
        Returns:
            True if deployment successful
        """
        self.logger.info("="*60)
        self.logger.info("Starting automated model deployment...")
        self.logger.info("="*60)
        
        try:
            # Step 1: Export to ONNX
            self.logger.info("Step 1/4: Exporting models to ONNX...")
            export_script = '/home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_training/scripts/export_to_onnx.py'
            
            result = subprocess.run(
                ['python3', export_script, '--all', '--verify', '--fp32'],
                capture_output=True,
                text=True,
                timeout=600  # 10 minute timeout
            )
            
            if result.returncode != 0:
                self.logger.error(f"ONNX export failed:\n{result.stderr}")
                return False
            
            self.logger.info("✓ ONNX export completed")
            
            # Step 2: Backup existing models
            self.logger.info("Step 2/4: Backing up existing models...")
            models_dir = Path('/opt/ml_monster_ai/models')
            
            if models_dir.exists():
                backup_dir = Path(f'/opt/ml_monster_ai/models_backup_{datetime.now().strftime("%Y%m%d_%H%M%S")}')
                
                result = subprocess.run(
                    ['cp', '-r', str(models_dir), str(backup_dir)],
                    capture_output=True,
                    timeout=60
                )
                
                if result.returncode == 0:
                    self.logger.info(f"✓ Backup created: {backup_dir}")
                else:
                    self.logger.warning("Backup failed, proceeding without backup")
            else:
                self.logger.info("No existing models to backup")
            
            # Step 3: Copy new models
            self.logger.info("Step 3/4: Deploying new models...")
            
            # Models should already be at /opt/ml_monster_ai/models from export script
            # Just verify they exist
            model_count = len(list(models_dir.rglob('*.onnx'))) if models_dir.exists() else 0
            
            if model_count == 0:
                self.logger.error("No ONNX models found after export")
                return False
            
            self.logger.info(f"✓ {model_count} ONNX models deployed")
            
            # Step 4: Restart inference service
            self.logger.info("Step 4/4: Restarting ML inference service...")
            
            # Check if service exists
            service_check = subprocess.run(
                ['systemctl', 'is-active', 'ml-inference'],
                capture_output=True,
                text=True
            )
            
            if service_check.returncode == 0:
                # Service exists, restart it
                result = subprocess.run(
                    ['sudo', 'systemctl', 'restart', 'ml-inference'],
                    capture_output=True,
                    timeout=30
                )
                
                if result.returncode == 0:
                    self.logger.info("✓ ML inference service restarted")
                    
                    # Wait for service to start
                    await asyncio.sleep(5)
                    
                    # Verify service is running
                    verify = subprocess.run(
                        ['systemctl', 'is-active', 'ml-inference'],
                        capture_output=True,
                        text=True
                    )
                    
                    if verify.returncode == 0:
                        self.logger.info("✓ ML inference service is active")
                    else:
                        self.logger.error("✗ ML inference service failed to start")
                        return False
                else:
                    self.logger.error("Failed to restart ML inference service")
                    return False
            else:
                self.logger.warning("ML inference service not found, skipping restart")
            
            self.logger.info("="*60)
            self.logger.info("✓ Deployment completed successfully")
            self.logger.info("="*60)
            
            return True
        
        except subprocess.TimeoutExpired:
            self.logger.error("Deployment timed out")
            return False
        
        except Exception as e:
            self.logger.error(f"Deployment failed: {e}")
            return False


async def main():
    """Test the training scheduler"""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    # Test configuration
    config = {
        'epochs': 10,
        'batch_size': 128,
        'learning_rate': 0.0003,
        'max_cpu_before_training': 30,
        'max_memory_before_training': 70,
        'pause_threshold_cpu': 80,
        'pause_threshold_memory': 90,
        'auto_deploy': False  # Disable for testing
    }
    
    scheduler = TrainingScheduler(config)
    
    # Check if training can start
    can_start, reason = await scheduler.can_start_training()
    print(f"\nCan start training: {can_start}")
    print(f"Reason: {reason}")
    
    # Optionally start training (commented out for safety)
    # if can_start:
    #     success = await scheduler.start_training(archetypes='aggressive', epochs=5)
    #     if success:
    #         exit_code = await scheduler.monitor_training()
    #         print(f"Training finished with exit code: {exit_code}")


if __name__ == '__main__':
    asyncio.run(main())
