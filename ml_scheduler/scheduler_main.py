#!/usr/bin/env python3
"""
ML Training Scheduler - Main Entry Point
Orchestrates automated training based on usage pattern analysis
"""

import asyncio
import logging
import sys
import signal
from datetime import datetime
from pathlib import Path
from typing import Optional
import yaml

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from ml_scheduler.usage_analyzer import UsagePatternAnalyzer
from ml_scheduler.training_scheduler import TrainingScheduler


class SchedulerOrchestrator:
    """
    Main orchestrator for ML training scheduler
    
    Workflow:
    1. Load configuration
    2. Analyze historical usage patterns
    3. Determine if current time is suitable for training
    4. Start training if conditions met
    5. Monitor training and handle resources
    6. Deploy models on success
    """
    
    def __init__(self, config_path: str):
        """
        Initialize scheduler orchestrator
        
        Args:
            config_path: Path to YAML configuration file
        """
        self.config_path = config_path
        self.config = self.load_config()
        self.logger = logging.getLogger(__name__)
        
        # Initialize components
        self.analyzer = UsagePatternAnalyzer(self.config['database'])
        self.scheduler = TrainingScheduler(self.config['training'])
        
        # Signal handling for graceful shutdown
        self.shutdown_requested = False
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def load_config(self) -> dict:
        """Load configuration from YAML file"""
        try:
            with open(self.config_path, 'r') as f:
                config = yaml.safe_load(f)
            
            # Validate required sections
            required = ['database', 'training', 'analysis', 'scheduling']
            for section in required:
                if section not in config:
                    raise ValueError(f"Missing required config section: {section}")
            
            return config
        
        except FileNotFoundError:
            print(f"ERROR: Configuration file not found: {self.config_path}")
            print(f"Please create the configuration file or use --config to specify location")
            sys.exit(1)
        
        except Exception as e:
            print(f"ERROR: Failed to load configuration: {e}")
            sys.exit(1)
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals gracefully"""
        self.logger.warning(f"Received signal {signum}, initiating graceful shutdown...")
        self.shutdown_requested = True
        
        # Stop training if in progress
        if self.scheduler.is_training:
            self.logger.info("Stopping training process...")
            asyncio.create_task(self.scheduler.stop_training(force=False))
    
    async def run(self) -> int:
        """
        Main execution flow
        
        Returns:
            Exit code (0 = success, 1 = error)
        """
        self.logger.info("="*70)
        self.logger.info("ML TRAINING SCHEDULER STARTED")
        self.logger.info("="*70)
        self.logger.info(f"Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        self.logger.info(f"Config: {self.config_path}")
        self.logger.info("="*70)
        
        try:
            # Step 1: Analyze usage patterns
            self.logger.info("\n[Step 1/5] Analyzing historical usage patterns...")
            
            history_days = self.config['analysis'].get('history_days', 30)
            
            player_df = await self.analyzer.get_historical_player_counts(days=history_days)
            system_df = await self.analyzer.get_system_load_history(days=history_days)
            
            # Verify we have enough data
            min_data_points = self.config['analysis'].get('min_data_points', 100)
            if len(player_df) < min_data_points:
                self.logger.warning(
                    f"Insufficient historical data: {len(player_df)} < {min_data_points} points"
                )
                self.logger.warning("Training may proceed but window selection may be suboptimal")
            
            # Step 2: Find off-peak windows
            self.logger.info("\n[Step 2/5] Identifying off-peak training windows...")
            
            window_hours = self.config['scheduling'].get('window_hours', 8)
            top_n_windows = self.config['scheduling'].get('top_n_windows', 3)
            
            windows = self.analyzer.find_off_peak_windows(
                player_df,
                system_df,
                window_hours=window_hours,
                top_n=top_n_windows
            )
            
            if not windows:
                self.logger.error("No suitable training windows found")
                return 1
            
            # Step 3: Check if current time is in an off-peak window
            self.logger.info("\n[Step 3/5] Checking if current time is suitable for training...")
            
            current_time = datetime.now()
            force_training = self.config['scheduling'].get('force_training', False)
            
            if force_training:
                self.logger.warning("⚠️  FORCE_TRAINING enabled - bypassing time window check")
                in_window = True
            else:
                in_window = self.analyzer.is_in_window(windows, current_time)
            
            if not in_window and not force_training:
                self.logger.info("✗ Current time is NOT in an off-peak window")
                self.logger.info(f"  Current: {current_time.strftime('%A %H:%M')}")
                
                # Show next available window
                next_window = windows[0]
                day_names = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
                self.logger.info(
                    f"  Next window: {day_names[next_window['day']]} "
                    f"{next_window['start_hour']:02d}:00-{next_window['end_hour']:02d}:00"
                )
                
                self.logger.info("\nScheduled training deferred to next off-peak window")
                return 0  # Success (no action needed)
            
            self.logger.info(f"✓ Current time IS in an off-peak window")
            
            # Step 4: Start training
            self.logger.info("\n[Step 4/5] Starting automated training...")
            
            archetypes = self.config['training'].get('archetypes', 'all')
            epochs = self.config['training'].get('epochs', 50)
            auto_deploy = self.config['training'].get('auto_deploy', True)
            
            training_started = await self.scheduler.start_training(
                archetypes=archetypes,
                epochs=epochs,
                auto_deploy=auto_deploy
            )
            
            if not training_started:
                self.logger.error("Failed to start training")
                return 1
            
            # Step 5: Monitor training
            self.logger.info("\n[Step 5/5] Monitoring training progress...")
            
            check_interval = self.config['training'].get('monitor_interval_seconds', 30)
            
            exit_code = await self.scheduler.monitor_training(check_interval=check_interval)
            
            if exit_code == 0:
                self.logger.info("\n" + "="*70)
                self.logger.info("✓ TRAINING COMPLETED SUCCESSFULLY")
                self.logger.info("="*70)
                return 0
            else:
                self.logger.error("\n" + "="*70)
                self.logger.error(f"✗ TRAINING FAILED (exit code: {exit_code})")
                self.logger.error("="*70)
                return 1
        
        except KeyboardInterrupt:
            self.logger.warning("\nInterrupted by user")
            return 130
        
        except Exception as e:
            self.logger.error(f"\nScheduler error: {e}", exc_info=True)
            return 1
        
        finally:
            # Cleanup
            self.logger.info("\nCleaning up resources...")
            await self.analyzer.close()
            
            if self.scheduler.is_training:
                self.logger.info("Stopping training process...")
                await self.scheduler.stop_training(force=False)
            
            self.logger.info("ML Training Scheduler finished\n")


def setup_logging(log_level: str = 'INFO', log_file: Optional[str] = None):
    """
    Setup logging configuration
    
    Args:
        log_level: Logging level (DEBUG, INFO, WARNING, ERROR)
        log_file: Optional log file path
    """
    handlers = [logging.StreamHandler(sys.stdout)]
    
    if log_file:
        log_path = Path(log_file)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        handlers.append(logging.FileHandler(log_file))
    
    logging.basicConfig(
        level=getattr(logging, log_level.upper()),
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=handlers
    )


async def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='ML Training Scheduler - Automated training based on usage patterns'
    )
    
    parser.add_argument(
        '--config',
        type=str,
        default='/opt/ml_monster_ai/configs/scheduler_config.yaml',
        help='Path to configuration file'
    )
    
    parser.add_argument(
        '--log-level',
        type=str,
        default='INFO',
        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR'],
        help='Logging level'
    )
    
    parser.add_argument(
        '--log-file',
        type=str,
        default='/opt/ml_monster_ai/logs/scheduler.log',
        help='Log file path'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Analyze patterns but do not start training'
    )
    
    args = parser.parse_args()
    
    # Setup logging
    setup_logging(args.log_level, args.log_file)
    logger = logging.getLogger(__name__)
    
    # Handle dry-run mode
    if args.dry_run:
        logger.info("DRY-RUN MODE: Will analyze patterns but not start training")
        
        try:
            with open(args.config, 'r') as f:
                config = yaml.safe_load(f)
            
            analyzer = UsagePatternAnalyzer(config['database'])
            
            player_df = await analyzer.get_historical_player_counts(days=30)
            system_df = await analyzer.get_system_load_history(days=30)
            
            windows = analyzer.find_off_peak_windows(player_df, system_df, window_hours=8)
            
            is_offpeak = analyzer.is_in_window(windows)
            
            logger.info(f"\n{'='*70}")
            logger.info(f"DRY-RUN RESULTS")
            logger.info(f"{'='*70}")
            logger.info(f"Current time: {datetime.now().strftime('%A %H:%M')}")
            logger.info(f"In off-peak window: {'YES' if is_offpeak else 'NO'}")
            logger.info(f"Found {len(windows)} suitable training windows")
            logger.info(f"{'='*70}")
            
            await analyzer.close()
            
            return 0
        
        except Exception as e:
            logger.error(f"Dry-run failed: {e}", exc_info=True)
            return 1
    
    # Normal execution
    orchestrator = SchedulerOrchestrator(args.config)
    exit_code = await orchestrator.run()
    
    return exit_code


if __name__ == '__main__':
    exit_code = asyncio.run(main())
    sys.exit(exit_code)
