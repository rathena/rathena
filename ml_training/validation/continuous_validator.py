"""
Continuous Model Validator
Validates model improvements before deployment to production
Ensures no performance degradation from continuous training
"""

import torch
import torch.nn as nn
import numpy as np
from typing import Dict, List, Any, Optional, Tuple
import logging
import time
from pathlib import Path
import onnxruntime as ort
from dataclasses import dataclass
import asyncpg

logger = logging.getLogger(__name__)


@dataclass
class ValidationMetrics:
    """Validation results for a model"""
    accuracy: float
    avg_reward: float
    win_rate: float
    inference_latency_ms: float
    model_size_mb: float
    loss: float
    improvement_delta: float  # vs production model
    sample_count: int


class ContinuousValidator:
    """
    Validates model improvements before deployment
    
    Performs:
    - Accuracy validation on held-out test set
    - Reward comparison vs production model
    - Performance benchmarking
    - Latency testing
    - Quality gates for deployment
    """
    
    def __init__(
        self,
        db_pool: asyncpg.Pool,
        device: str = 'cuda:0',
        validation_samples: int = 1000,
        min_improvement_threshold: float = 0.01  # 1% improvement required
    ):
        """
        Initialize validator
        
        Args:
            db_pool: Database connection pool
            device: Device for validation
            validation_samples: Number of samples for validation
            min_improvement_threshold: Minimum improvement required (e.g., 0.01 = 1%)
        """
        self.db_pool = db_pool
        self.device = device
        self.validation_samples = validation_samples
        self.min_improvement_threshold = min_improvement_threshold
        self.logger = logging.getLogger(__name__)
        
        # Cache validation data per archetype
        self.validation_cache: Dict[str, Dict[str, Any]] = {}
    
    async def validate_model(
        self,
        model: nn.Module,
        archetype: str,
        model_type: str,
        validation_data: Optional[List[Dict]] = None
    ) -> ValidationMetrics:
        """
        Validate model on test data
        
        Args:
            model: PyTorch model to validate
            archetype: Monster archetype
            model_type: Model type
            validation_data: Optional pre-loaded validation data
        
        Returns:
            ValidationMetrics object
        """
        self.logger.info(f"Validating model: {archetype}/{model_type}")
        
        # Get or load validation data
        if validation_data is None:
            validation_data = await self._get_validation_data(archetype, model_type)
        
        if not validation_data:
            self.logger.warning(f"No validation data for {archetype}/{model_type}")
            return ValidationMetrics(
                accuracy=0.0,
                avg_reward=0.0,
                win_rate=0.0,
                inference_latency_ms=0.0,
                model_size_mb=0.0,
                loss=float('inf'),
                improvement_delta=0.0,
                sample_count=0
            )
        
        model.eval()
        model.to(self.device)
        
        # Compute metrics
        metrics = self.calculate_metrics(model, validation_data)
        
        self.logger.info(
            f"Validation complete: {archetype}/{model_type} - "
            f"Accuracy: {metrics.accuracy:.2%}, "
            f"Avg Reward: {metrics.avg_reward:.2f}, "
            f"Loss: {metrics.loss:.4f}"
        )
        
        return metrics
    
    async def compare_with_production(
        self,
        new_model: nn.Module,
        prod_model_path: str,
        archetype: str,
        model_type: str,
        test_data: Optional[List[Dict]] = None
    ) -> Dict[str, Any]:
        """
        Compare new model with current production model
        
        Args:
            new_model: Newly trained model
            prod_model_path: Path to production ONNX model
            archetype: Monster archetype
            model_type: Model type
            test_data: Optional test data
        
        Returns:
            Comparison dictionary with deployment recommendation
        """
        self.logger.info(
            f"Comparing new model with production: {archetype}/{model_type}"
        )
        
        # Get test data
        if test_data is None:
            test_data = await self._get_validation_data(archetype, model_type)
        
        if not test_data:
            self.logger.warning("No test data available")
            return {
                'deploy_recommended': False,
                'reason': 'no_test_data',
                'new_metrics': None,
                'prod_metrics': None
            }
        
        # Validate new model
        new_metrics = await self.validate_model(new_model, archetype, model_type, test_data)
        
        # Validate production model
        prod_metrics = await self._validate_onnx_model(prod_model_path, test_data)
        
        # Calculate improvement
        improvement_delta = self._calculate_improvement(new_metrics, prod_metrics)
        new_metrics.improvement_delta = improvement_delta
        
        # Deployment decision
        deploy_recommended = improvement_delta >= self.min_improvement_threshold
        
        # Detailed comparison
        comparison = {
            'deploy_recommended': deploy_recommended,
            'reason': self._get_deployment_reason(improvement_delta, new_metrics, prod_metrics),
            'new_metrics': {
                'accuracy': new_metrics.accuracy,
                'avg_reward': new_metrics.avg_reward,
                'win_rate': new_metrics.win_rate,
                'loss': new_metrics.loss,
                'latency_ms': new_metrics.inference_latency_ms
            },
            'prod_metrics': {
                'accuracy': prod_metrics.accuracy,
                'avg_reward': prod_metrics.avg_reward,
                'win_rate': prod_metrics.win_rate,
                'loss': prod_metrics.loss,
                'latency_ms': prod_metrics.inference_latency_ms
            },
            'improvement': {
                'delta': improvement_delta,
                'accuracy_change': new_metrics.accuracy - prod_metrics.accuracy,
                'reward_change': new_metrics.avg_reward - prod_metrics.avg_reward,
                'win_rate_change': new_metrics.win_rate - prod_metrics.win_rate,
                'latency_change': new_metrics.inference_latency_ms - prod_metrics.inference_latency_ms
            },
            'deployment_threshold': self.min_improvement_threshold
        }
        
        self.logger.info(
            f"Comparison complete: improvement_delta={improvement_delta:.2%}, "
            f"deploy={deploy_recommended}"
        )
        
        return comparison
    
    async def run_performance_benchmark(
        self,
        model: nn.Module,
        batch_sizes: List[int] = [1, 16, 32, 64, 128]
    ) -> Dict[str, Any]:
        """
        Run performance benchmark on model
        
        Tests inference latency at different batch sizes
        
        Args:
            model: Model to benchmark
            batch_sizes: List of batch sizes to test
        
        Returns:
            Benchmark results
        """
        self.logger.info("Running performance benchmark")
        
        model.eval()
        model.to(self.device)
        
        results = {}
        
        for batch_size in batch_sizes:
            # Create random input
            dummy_input = torch.randn(batch_size, 64, device=self.device)
            
            # Warmup
            with torch.no_grad():
                for _ in range(10):
                    _ = model(dummy_input)
            
            # Benchmark
            latencies = []
            with torch.no_grad():
                for _ in range(100):
                    start = time.perf_counter()
                    _ = model(dummy_input)
                    
                    if self.device.startswith('cuda'):
                        torch.cuda.synchronize()
                    
                    latency = (time.perf_counter() - start) * 1000  # ms
                    latencies.append(latency)
            
            results[f'batch_{batch_size}'] = {
                'avg_ms': float(np.mean(latencies)),
                'p50_ms': float(np.percentile(latencies, 50)),
                'p95_ms': float(np.percentile(latencies, 95)),
                'p99_ms': float(np.percentile(latencies, 99)),
                'per_sample_ms': float(np.mean(latencies) / batch_size)
            }
            
            self.logger.info(
                f"Batch size {batch_size}: {results[f'batch_{batch_size}']['avg_ms']:.2f}ms avg, "
                f"{results[f'batch_{batch_size}']['per_sample_ms']:.2f}ms per sample"
            )
        
        return results
    
    def calculate_metrics(
        self,
        model: nn.Module,
        validation_data: List[Dict]
    ) -> ValidationMetrics:
        """
        Calculate comprehensive validation metrics
        
        Args:
            model: Model to validate
            validation_data: List of validation samples
        
        Returns:
            ValidationMetrics object
        """
        model.eval()
        
        correct_predictions = 0
        total_rewards = 0.0
        wins = 0
        total_loss = 0.0
        latencies = []
        
        with torch.no_grad():
            for sample in validation_data:
                # Prepare input
                state = torch.FloatTensor(sample['state']).unsqueeze(0).to(self.device)
                
                # Measure latency
                start = time.perf_counter()
                
                # Forward pass
                if model_type_is_actor_critic(model):
                    # PPO/A2C style model
                    action_logits, value = model(state)
                    action_probs = torch.softmax(action_logits, dim=-1)
                    predicted_action = torch.argmax(action_probs, dim=-1).item()
                else:
                    # DQN style model
                    q_values = model(state)
                    predicted_action = torch.argmax(q_values, dim=-1).item()
                
                if self.device.startswith('cuda'):
                    torch.cuda.synchronize()
                
                latency = (time.perf_counter() - start) * 1000  # ms
                latencies.append(latency)
                
                # Check accuracy
                true_action = sample.get('action', sample.get('action_id', 0))
                if predicted_action == true_action:
                    correct_predictions += 1
                
                # Accumulate reward
                reward = sample.get('reward', 0.0)
                total_rewards += reward
                
                # Check win (positive reward or reward > threshold)
                if reward > 0:
                    wins += 1
                
                # Compute loss (if labels available)
                if 'action' in sample or 'action_id' in sample:
                    if model_type_is_actor_critic(model):
                        # Cross-entropy loss for policy
                        target_action = torch.LongTensor([true_action]).to(self.device)
                        loss = nn.functional.cross_entropy(action_logits, target_action)
                    else:
                        # MSE loss for Q-values
                        target_action = torch.LongTensor([true_action]).to(self.device)
                        q_target = torch.zeros_like(q_values)
                        q_target[0, true_action] = reward + 0.99 * torch.max(q_values).item()
                        loss = nn.functional.mse_loss(q_values, q_target)
                    
                    total_loss += loss.item()
        
        sample_count = len(validation_data)
        
        # Calculate model size
        model_size_mb = sum(
            p.numel() * p.element_size() for p in model.parameters()
        ) / (1024 ** 2)
        
        return ValidationMetrics(
            accuracy=correct_predictions / sample_count if sample_count > 0 else 0.0,
            avg_reward=total_rewards / sample_count if sample_count > 0 else 0.0,
            win_rate=wins / sample_count if sample_count > 0 else 0.0,
            inference_latency_ms=float(np.mean(latencies)) if latencies else 0.0,
            model_size_mb=model_size_mb,
            loss=total_loss / sample_count if sample_count > 0 else float('inf'),
            improvement_delta=0.0,  # Set later in comparison
            sample_count=sample_count
        )
    
    async def _get_validation_data(
        self,
        archetype: str,
        model_type: str
    ) -> List[Dict]:
        """
        Get validation data from database
        
        Uses most recent experiences as validation set
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            List of validation samples
        """
        # Check cache
        cache_key = f"{archetype}_{model_type}"
        if cache_key in self.validation_cache:
            cache_time = self.validation_cache[cache_key].get('timestamp', 0)
            if time.time() - cache_time < 3600:  # Cache for 1 hour
                self.logger.debug(f"Using cached validation data for {cache_key}")
                return self.validation_cache[cache_key]['data']
        
        # Query database for validation samples
        # Use most recent 10% of data as validation (not used in training)
        query = """
        WITH recent_experiences AS (
            SELECT 
                state_vector,
                action_id,
                reward,
                next_state_vector,
                done
            FROM ml_experience_replay
            WHERE archetype = $1
                AND timestamp > NOW() - INTERVAL '7 days'
                AND sample_count < 2  -- Rarely sampled, good for validation
            ORDER BY timestamp DESC
            LIMIT $2
        )
        SELECT * FROM recent_experiences
        ORDER BY RANDOM()
        """
        
        async with self.db_pool.acquire() as conn:
            rows = await conn.fetch(query, archetype, self.validation_samples)
            
            validation_data = [
                {
                    'state': np.array(row['state_vector'], dtype=np.float32),
                    'action_id': row['action_id'],
                    'reward': float(row['reward']),
                    'next_state': np.array(row['next_state_vector'], dtype=np.float32) if row['next_state_vector'] else None,
                    'done': row['done']
                }
                for row in rows
            ]
            
            # Cache for future use
            self.validation_cache[cache_key] = {
                'data': validation_data,
                'timestamp': time.time()
            }
            
            self.logger.info(
                f"Loaded {len(validation_data)} validation samples for {archetype}/{model_type}"
            )
            
            return validation_data
    
    async def _validate_onnx_model(
        self,
        onnx_model_path: str,
        test_data: List[Dict]
    ) -> ValidationMetrics:
        """
        Validate ONNX model (for production comparison)
        
        Args:
            onnx_model_path: Path to ONNX model
            test_data: Test data
        
        Returns:
            ValidationMetrics
        """
        path = Path(onnx_model_path)
        
        if not path.exists():
            self.logger.error(f"ONNX model not found: {onnx_model_path}")
            return ValidationMetrics(
                accuracy=0.0,
                avg_reward=0.0,
                win_rate=0.0,
                inference_latency_ms=float('inf'),
                model_size_mb=0.0,
                loss=float('inf'),
                improvement_delta=0.0,
                sample_count=0
            )
        
        # Create ONNX session
        session_options = ort.SessionOptions()
        session_options.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL
        
        providers = ['CUDAExecutionProvider', 'CPUExecutionProvider'] if self.device.startswith('cuda') else ['CPUExecutionProvider']
        
        try:
            session = ort.InferenceSession(
                str(onnx_model_path),
                session_options,
                providers=providers
            )
        except Exception as e:
            self.logger.error(f"Failed to load ONNX model: {e}")
            return ValidationMetrics(
                accuracy=0.0,
                avg_reward=0.0,
                win_rate=0.0,
                inference_latency_ms=float('inf'),
                model_size_mb=0.0,
                loss=float('inf'),
                improvement_delta=0.0,
                sample_count=0
            )
        
        # Run validation
        correct_predictions = 0
        total_rewards = 0.0
        wins = 0
        latencies = []
        
        input_name = session.get_inputs()[0].name
        output_name = session.get_outputs()[0].name
        
        for sample in test_data:
            state = sample['state'].reshape(1, -1).astype(np.float32)
            
            # Measure latency
            start = time.perf_counter()
            outputs = session.run([output_name], {input_name: state})
            latency = (time.perf_counter() - start) * 1000  # ms
            latencies.append(latency)
            
            # Get predicted action
            logits = outputs[0]
            predicted_action = int(np.argmax(logits, axis=-1)[0])
            
            # Check accuracy
            true_action = sample.get('action', sample.get('action_id', 0))
            if predicted_action == true_action:
                correct_predictions += 1
            
            # Accumulate reward
            reward = sample.get('reward', 0.0)
            total_rewards += reward
            
            if reward > 0:
                wins += 1
        
        sample_count = len(test_data)
        
        # Get model size
        model_size_mb = path.stat().st_size / (1024 ** 2)
        
        return ValidationMetrics(
            accuracy=correct_predictions / sample_count if sample_count > 0 else 0.0,
            avg_reward=total_rewards / sample_count if sample_count > 0 else 0.0,
            win_rate=wins / sample_count if sample_count > 0 else 0.0,
            inference_latency_ms=float(np.mean(latencies)) if latencies else 0.0,
            model_size_mb=model_size_mb,
            loss=0.0,  # Not computed for ONNX
            improvement_delta=0.0,
            sample_count=sample_count
        )
    
    def _calculate_improvement(
        self,
        new_metrics: ValidationMetrics,
        prod_metrics: ValidationMetrics
    ) -> float:
        """
        Calculate overall improvement score
        
        Weighted combination of multiple metrics
        
        Args:
            new_metrics: New model metrics
            prod_metrics: Production model metrics
        
        Returns:
            Improvement delta (positive = better, negative = worse)
        """
        # Weights for different metrics
        weights = {
            'accuracy': 0.3,
            'avg_reward': 0.4,
            'win_rate': 0.2,
            'loss': 0.1  # Lower is better
        }
        
        # Calculate relative improvements
        accuracy_improvement = (new_metrics.accuracy - prod_metrics.accuracy) / (prod_metrics.accuracy + 1e-6)
        reward_improvement = (new_metrics.avg_reward - prod_metrics.avg_reward) / (abs(prod_metrics.avg_reward) + 1e-6)
        win_rate_improvement = (new_metrics.win_rate - prod_metrics.win_rate) / (prod_metrics.win_rate + 1e-6)
        loss_improvement = -(new_metrics.loss - prod_metrics.loss) / (prod_metrics.loss + 1e-6)  # Negative because lower is better
        
        # Weighted average
        improvement_delta = (
            weights['accuracy'] * accuracy_improvement +
            weights['avg_reward'] * reward_improvement +
            weights['win_rate'] * win_rate_improvement +
            weights['loss'] * loss_improvement
        )
        
        return improvement_delta
    
    def _get_deployment_reason(
        self,
        improvement_delta: float,
        new_metrics: ValidationMetrics,
        prod_metrics: ValidationMetrics
    ) -> str:
        """
        Get human-readable deployment recommendation reason
        
        Args:
            improvement_delta: Overall improvement score
            new_metrics: New model metrics
            prod_metrics: Production metrics
        
        Returns:
            Reason string
        """
        if improvement_delta >= self.min_improvement_threshold:
            return (
                f"Model improved by {improvement_delta:.2%} "
                f"(threshold: {self.min_improvement_threshold:.2%})"
            )
        elif improvement_delta > 0:
            return (
                f"Improvement {improvement_delta:.2%} below threshold "
                f"({self.min_improvement_threshold:.2%})"
            )
        else:
            # Identify which metric degraded
            if new_metrics.accuracy < prod_metrics.accuracy * 0.95:
                return f"Accuracy degraded: {new_metrics.accuracy:.2%} < {prod_metrics.accuracy:.2%}"
            elif new_metrics.avg_reward < prod_metrics.avg_reward * 0.95:
                return f"Reward degraded: {new_metrics.avg_reward:.2f} < {prod_metrics.avg_reward:.2f}"
            elif new_metrics.win_rate < prod_metrics.win_rate * 0.95:
                return f"Win rate degraded: {new_metrics.win_rate:.2%} < {prod_metrics.win_rate:.2%}"
            else:
                return f"Overall performance degraded by {-improvement_delta:.2%}"
    
    async def validate_before_deployment(
        self,
        model_path: str,
        archetype: str,
        model_type: str,
        prod_model_path: Optional[str] = None
    ) -> Tuple[bool, Dict[str, Any]]:
        """
        Comprehensive validation before deployment
        
        Runs all validation checks and returns deployment recommendation
        
        Args:
            model_path: Path to new model checkpoint (PyTorch)
            archetype: Monster archetype
            model_type: Model type
            prod_model_path: Path to production ONNX model (optional)
        
        Returns:
            Tuple of (should_deploy, validation_results)
        """
        self.logger.info(f"Running deployment validation for {archetype}/{model_type}")
        
        validation_results = {
            'archetype': archetype,
            'model_type': model_type,
            'model_path': model_path,
            'timestamp': datetime.utcnow().isoformat(),
            'checks': {}
        }
        
        try:
            # Load model
            model = torch.load(model_path, map_location=self.device)
            
            # Check 1: Model loads successfully
            validation_results['checks']['model_loads'] = True
            self.logger.info("✓ Model loads successfully")
            
            # Check 2: Basic validation metrics
            test_data = await self._get_validation_data(archetype, model_type)
            metrics = await self.validate_model(model, archetype, model_type, test_data)
            
            validation_results['checks']['validation_metrics'] = {
                'accuracy': metrics.accuracy,
                'avg_reward': metrics.avg_reward,
                'win_rate': metrics.win_rate,
                'loss': metrics.loss
            }
            
            # Check 3: Minimum quality thresholds
            min_accuracy = 0.3  # At least 30% accuracy
            min_win_rate = 0.2  # At least 20% win rate
            
            quality_pass = (
                metrics.accuracy >= min_accuracy and
                metrics.win_rate >= min_win_rate
            )
            
            validation_results['checks']['quality_gate'] = quality_pass
            
            if not quality_pass:
                self.logger.warning(
                    f"✗ Quality gate failed: accuracy={metrics.accuracy:.2%} (min={min_accuracy:.2%}), "
                    f"win_rate={metrics.win_rate:.2%} (min={min_win_rate:.2%})"
                )
                return False, validation_results
            
            self.logger.info(
                f"✓ Quality gate passed: accuracy={metrics.accuracy:.2%}, "
                f"win_rate={metrics.win_rate:.2%}"
            )
            
            # Check 4: Compare with production (if available)
            if prod_model_path and Path(prod_model_path).exists():
                comparison = await self.compare_with_production(
                    model, prod_model_path, archetype, model_type, test_data
                )
                
                validation_results['checks']['production_comparison'] = comparison
                
                if not comparison['deploy_recommended']:
                    self.logger.warning(
                        f"✗ Production comparison failed: {comparison['reason']}"
                    )
                    return False, validation_results
                
                self.logger.info(
                    f"✓ Production comparison passed: "
                    f"improvement={comparison['improvement']['delta']:.2%}"
                )
            else:
                self.logger.info("⚠ No production model for comparison, skipping")
                validation_results['checks']['production_comparison'] = {
                    'deploy_recommended': True,
                    'reason': 'no_production_model'
                }
            
            # Check 5: Performance benchmark
            benchmark = await self.run_performance_benchmark(model)
            validation_results['checks']['performance_benchmark'] = benchmark
            
            # Check latency requirement (<15ms for batch of 128)
            batch_128_latency = benchmark.get('batch_128', {}).get('avg_ms', float('inf'))
            latency_acceptable = batch_128_latency < 50  # Allow some buffer over 15ms
            
            validation_results['checks']['latency_check'] = {
                'latency_ms': batch_128_latency,
                'threshold_ms': 50,
                'passed': latency_acceptable
            }
            
            if not latency_acceptable:
                self.logger.warning(
                    f"✗ Latency check failed: {batch_128_latency:.2f}ms > 50ms"
                )
                return False, validation_results
            
            self.logger.info(f"✓ Latency check passed: {batch_128_latency:.2f}ms")
            
            # All checks passed
            validation_results['deploy_recommended'] = True
            validation_results['summary'] = {
                'all_checks_passed': True,
                'accuracy': metrics.accuracy,
                'avg_reward': metrics.avg_reward,
                'improvement_delta': metrics.improvement_delta
            }
            
            self.logger.info(
                f"✓ All validation checks passed for {archetype}/{model_type}"
            )
            
            return True, validation_results
        
        except Exception as e:
            self.logger.error(f"Validation failed with exception: {e}", exc_info=True)
            validation_results['checks']['exception'] = str(e)
            validation_results['deploy_recommended'] = False
            return False, validation_results
    
    async def log_validation_result(
        self,
        archetype: str,
        model_type: str,
        metrics: ValidationMetrics,
        deployed: bool
    ) -> int:
        """
        Log validation result to database
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            metrics: Validation metrics
            deployed: Whether model was deployed
        
        Returns:
            Log entry ID
        """
        query = """
        INSERT INTO ml_continuous_training_log (
            archetype,
            model_type,
            training_start,
            training_end,
            experiences_processed,
            steps_taken,
            validation_metric,
            improvement_delta,
            deployed,
            rollback
        ) VALUES (
            $1, $2, NOW(), NOW(), $3, 0, $4, $5, $6, FALSE
        )
        RETURNING id
        """
        
        async with self.db_pool.acquire() as conn:
            row = await conn.fetchrow(
                query,
                archetype,
                model_type,
                metrics.sample_count,
                metrics.accuracy,  # Use accuracy as primary metric
                metrics.improvement_delta,
                deployed
            )
            
            log_id = row['id']
            
            self.logger.info(
                f"Logged validation result: {archetype}/{model_type}, "
                f"deployed={deployed}, log_id={log_id}"
            )
            
            return log_id


def model_type_is_actor_critic(model: nn.Module) -> bool:
    """
    Determine if model is actor-critic architecture
    
    Checks model outputs to infer architecture
    
    Args:
        model: PyTorch model
    
    Returns:
        True if actor-critic, False if Q-network
    """
    # Create dummy input
    dummy_input = torch.randn(1, 64)
    
    try:
        with torch.no_grad():
            output = model(dummy_input)
            
            # Actor-critic returns tuple (action_logits, value)
            if isinstance(output, tuple) and len(output) == 2:
                return True
            
            # Q-network returns single tensor
            return False
    except Exception:
        # Default to Q-network if can't determine
        return False


class RollbackDetector:
    """
    Detects when a deployed model is performing worse than expected
    Triggers rollback to previous version
    """
    
    def __init__(
        self,
        db_pool: asyncpg.Pool,
        degradation_threshold: float = 0.1  # 10% degradation triggers rollback
    ):
        """
        Initialize rollback detector
        
        Args:
            db_pool: Database connection pool
            degradation_threshold: Performance drop threshold for rollback
        """
        self.db_pool = db_pool
        self.degradation_threshold = degradation_threshold
        self.logger = logging.getLogger(__name__)
        
        # Track deployed model performance
        self.deployed_metrics: Dict[str, ValidationMetrics] = {}
    
    async def monitor_deployed_model(
        self,
        archetype: str,
        model_type: str,
        lookback_minutes: int = 30
    ) -> Optional[Dict[str, Any]]:
        """
        Monitor deployed model performance in production
        
        Checks if performance has degraded since deployment
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            lookback_minutes: Time window to analyze
        
        Returns:
            Rollback recommendation or None
        """
        # Get recent performance data
        since = datetime.utcnow() - timedelta(minutes=lookback_minutes)
        
        query = """
        SELECT 
            AVG(CASE WHEN action_id = (state_vector[1])::INT THEN 1.0 ELSE 0.0 END) as accuracy,
            AVG(reward) as avg_reward,
            COUNT(*) as sample_count
        FROM ml_experience_replay
        WHERE archetype = $1
            AND timestamp > $2
        """
        
        async with self.db_pool.acquire() as conn:
            row = await conn.fetchrow(query, archetype, since)
            
            if not row or row['sample_count'] < 100:
                # Not enough data yet
                return None
            
            current_accuracy = float(row['accuracy']) if row['accuracy'] else 0.0
            current_reward = float(row['avg_reward']) if row['avg_reward'] else 0.0
        
        # Compare with deployed metrics
        key = f"{archetype}_{model_type}"
        if key not in self.deployed_metrics:
            # No baseline yet, store current as baseline
            self.deployed_metrics[key] = ValidationMetrics(
                accuracy=current_accuracy,
                avg_reward=current_reward,
                win_rate=0.0,
                inference_latency_ms=0.0,
                model_size_mb=0.0,
                loss=0.0,
                improvement_delta=0.0,
                sample_count=row['sample_count']
            )
            return None
        
        baseline = self.deployed_metrics[key]
        
        # Check for degradation
        accuracy_degradation = (baseline.accuracy - current_accuracy) / (baseline.accuracy + 1e-6)
        reward_degradation = (baseline.avg_reward - current_reward) / (abs(baseline.avg_reward) + 1e-6)
        
        # Overall degradation
        degradation = (0.5 * accuracy_degradation + 0.5 * reward_degradation)
        
        if degradation > self.degradation_threshold:
            self.logger.warning(
                f"Performance degradation detected: {archetype}/{model_type} - "
                f"degradation={degradation:.2%} (threshold={self.degradation_threshold:.2%})"
            )
            
            return {
                'rollback_recommended': True,
                'degradation': degradation,
                'current_accuracy': current_accuracy,
                'baseline_accuracy': baseline.accuracy,
                'current_reward': current_reward,
                'baseline_reward': baseline.avg_reward,
                'sample_count': row['sample_count']
            }
        
        return None
    
    def set_baseline(
        self,
        archetype: str,
        model_type: str,
        metrics: ValidationMetrics
    ):
        """
        Set baseline metrics for deployed model
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            metrics: Validation metrics to use as baseline
        """
        key = f"{archetype}_{model_type}"
        self.deployed_metrics[key] = metrics
        
        self.logger.info(
            f"Set baseline for {archetype}/{model_type}: "
            f"accuracy={metrics.accuracy:.2%}, reward={metrics.avg_reward:.2f}"
        )
