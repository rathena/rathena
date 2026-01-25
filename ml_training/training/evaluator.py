"""
ML Monster AI - Model Evaluation and Metrics
Enhanced ML Monster AI System v2.0

Implements comprehensive model evaluation:
- Win rate calculation
- Average reward per episode
- Action distribution analysis
- Latency profiling
- Model size measurement
- VRAM usage monitoring
"""

import torch
import torch.nn as nn
import numpy as np
from typing import Dict, Any, List, Optional
import time
import logging
from collections import defaultdict

logger = logging.getLogger(__name__)


class ModelEvaluator:
    """
    Comprehensive model evaluation
    
    Metrics:
    - Performance: win rate, avg reward, success rate
    - Efficiency: latency, throughput
    - Quality: action diversity, value accuracy
    - Resources: model size, VRAM usage
    """
    
    def __init__(self, device: str = 'cuda'):
        """
        Initialize evaluator
        
        Args:
            device: Device for evaluation
        """
        self.device = device
        self.results = defaultdict(list)
        
        logger.info(f"ModelEvaluator initialized on {device}")
    
    @torch.no_grad()
    def evaluate_model(
        self,
        model: nn.Module,
        test_data: List[Dict[str, Any]],
        num_episodes: int = 100
    ) -> Dict[str, float]:
        """
        Comprehensive model evaluation
        
        Args:
            model: Model to evaluate
            test_data: Test episodes
            num_episodes: Number of episodes to evaluate
        
        Returns:
            Dictionary with evaluation metrics
        """
        model.eval()
        
        metrics = {
            'win_rate': 0.0,
            'avg_reward': 0.0,
            'avg_episode_length': 0.0,
            'avg_latency_ms': 0.0,
            'action_entropy': 0.0
        }
        
        wins = 0
        total_reward = 0.0
        total_length = 0
        latencies = []
        action_counts = defaultdict(int)
        
        for i, episode in enumerate(test_data[:num_episodes]):
            # Run episode
            episode_reward = 0.0
            episode_length = 0
            
            states = episode.get('states', [])
            
            for state in states:
                state_tensor = torch.FloatTensor(state).unsqueeze(0).to(self.device)
                
                # Measure latency
                start_time = time.time()
                
                if hasattr(model, 'get_action'):
                    action, _, _ = model.get_action(state_tensor, deterministic=True)
                    action = action.item()
                else:
                    output = model(state_tensor)
                    action = output.argmax(1).item()
                
                latency = (time.time() - start_time) * 1000  # ms
                latencies.append(latency)
                
                # Track action
                action_counts[action] += 1
                episode_length += 1
            
            # Get episode outcome
            episode_reward = episode.get('total_reward', 0.0)
            won = episode.get('won', False)
            
            if won:
                wins += 1
            
            total_reward += episode_reward
            total_length += episode_length
        
        # Compute metrics
        num_evaluated = min(num_episodes, len(test_data))
        
        metrics['win_rate'] = wins / num_evaluated
        metrics['avg_reward'] = total_reward / num_evaluated
        metrics['avg_episode_length'] = total_length / num_evaluated
        metrics['avg_latency_ms'] = np.mean(latencies) if latencies else 0.0
        
        # Action entropy (measure of exploration)
        total_actions = sum(action_counts.values())
        action_probs = np.array([action_counts[a] / total_actions for a in sorted(action_counts.keys())])
        metrics['action_entropy'] = -np.sum(action_probs * np.log(action_probs + 1e-10))
        
        logger.info(f"Evaluation complete: {num_evaluated} episodes")
        logger.info(f"  Win Rate: {metrics['win_rate']:.2%}")
        logger.info(f"  Avg Reward: {metrics['avg_reward']:.2f}")
        logger.info(f"  Avg Latency: {metrics['avg_latency_ms']:.2f} ms")
        logger.info(f"  Action Entropy: {metrics['action_entropy']:.3f}")
        
        model.train()
        return metrics
    
    @torch.no_grad()
    def benchmark_inference(
        self,
        model: nn.Module,
        state_dim: int = 64,
        batch_sizes: List[int] = [1, 16, 32, 64, 128],
        num_iterations: int = 100
    ) -> Dict[int, Dict[str, float]]:
        """
        Benchmark inference latency
        
        Args:
            model: Model to benchmark
            state_dim: State dimension
            batch_sizes: List of batch sizes to test
            num_iterations: Number of iterations per batch size
        
        Returns:
            Dictionary mapping batch_size -> metrics
        """
        model.eval()
        
        results = {}
        
        logger.info(f"Benchmarking inference latency...")
        
        for batch_size in batch_sizes:
            latencies = []
            
            # Warmup
            for _ in range(10):
                dummy_input = torch.randn(batch_size, state_dim, device=self.device)
                _ = model(dummy_input)
            
            # Benchmark
            for _ in range(num_iterations):
                dummy_input = torch.randn(batch_size, state_dim, device=self.device)
                
                torch.cuda.synchronize() if self.device.startswith('cuda') else None
                start_time = time.time()
                
                _ = model(dummy_input)
                
                torch.cuda.synchronize() if self.device.startswith('cuda') else None
                end_time = time.time()
                
                latency_ms = (end_time - start_time) * 1000
                latencies.append(latency_ms)
            
            results[batch_size] = {
                'mean_latency_ms': np.mean(latencies),
                'std_latency_ms': np.std(latencies),
                'p50_latency_ms': np.percentile(latencies, 50),
                'p95_latency_ms': np.percentile(latencies, 95),
                'p99_latency_ms': np.percentile(latencies, 99),
                'throughput': batch_size / (np.mean(latencies) / 1000)  # samples/sec
            }
            
            logger.info(f"  Batch {batch_size}: {results[batch_size]['mean_latency_ms']:.2f} ms "
                       f"(p95: {results[batch_size]['p95_latency_ms']:.2f} ms)")
        
        model.train()
        return results
    
    def measure_model_size(self, model: nn.Module) -> Dict[str, float]:
        """
        Measure model size
        
        Args:
            model: Model to measure
        
        Returns:
            Dictionary with size metrics
        """
        # Count parameters
        total_params = sum(p.numel() for p in model.parameters())
        trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
        
        # Estimate sizes
        fp32_size_mb = (total_params * 4) / (1024 ** 2)
        fp16_size_mb = (total_params * 2) / (1024 ** 2)
        int8_size_mb = total_params / (1024 ** 2)
        
        metrics = {
            'total_parameters': total_params,
            'trainable_parameters': trainable_params,
            'fp32_size_mb': fp32_size_mb,
            'fp16_size_mb': fp16_size_mb,
            'int8_size_mb': int8_size_mb
        }
        
        logger.info(f"Model size: {total_params:,} parameters")
        logger.info(f"  FP32: {fp32_size_mb:.2f} MB")
        logger.info(f"  FP16: {fp16_size_mb:.2f} MB")
        logger.info(f"  INT8: {int8_size_mb:.2f} MB")
        
        return metrics
    
    def measure_vram_usage(self) -> Dict[str, float]:
        """
        Measure current VRAM usage
        
        Returns:
            Dictionary with VRAM metrics
        """
        if not torch.cuda.is_available():
            return {'vram_allocated_mb': 0.0, 'vram_reserved_mb': 0.0}
        
        torch.cuda.synchronize()
        
        allocated = torch.cuda.memory_allocated(self.device) / (1024 ** 2)
        reserved = torch.cuda.memory_reserved(self.device) / (1024 ** 2)
        max_allocated = torch.cuda.max_memory_allocated(self.device) / (1024 ** 2)
        
        metrics = {
            'vram_allocated_mb': allocated,
            'vram_reserved_mb': reserved,
            'vram_max_allocated_mb': max_allocated
        }
        
        logger.info(f"VRAM Usage: Allocated={allocated:.2f} MB, Reserved={reserved:.2f} MB")
        
        return metrics
    
    def analyze_action_distribution(
        self,
        model: nn.Module,
        states: np.ndarray,
        action_dim: int
    ) -> Dict[str, Any]:
        """
        Analyze action distribution for given states
        
        Args:
            model: Model to analyze
            states: State array
            action_dim: Number of actions
        
        Returns:
            Dictionary with distribution metrics
        """
        model.eval()
        
        action_counts = np.zeros(action_dim)
        
        states_t = torch.FloatTensor(states).to(self.device)
        
        with torch.no_grad():
            if hasattr(model, 'get_action'):
                # PPO/SAC style
                for state in states_t:
                    action, _, _ = model.get_action(state.unsqueeze(0), deterministic=True)
                    action_counts[action.item()] += 1
            else:
                # DQN style
                outputs = model(states_t)
                actions = outputs.argmax(1).cpu().numpy()
                for action in actions:
                    action_counts[action] += 1
        
        # Compute statistics
        action_probs = action_counts / action_counts.sum()
        entropy = -np.sum(action_probs * np.log(action_probs + 1e-10))
        
        metrics = {
            'action_counts': action_counts.tolist(),
            'action_probabilities': action_probs.tolist(),
            'entropy': float(entropy),
            'most_common_action': int(np.argmax(action_counts)),
            'least_common_action': int(np.argmin(action_counts))
        }
        
        logger.info(f"Action distribution: Entropy={entropy:.3f}")
        logger.info(f"  Most common: Action {metrics['most_common_action']} ({action_probs[metrics['most_common_action']]:.1%})")
        
        model.train()
        return metrics
    
    def compute_value_accuracy(
        self,
        model: nn.Module,
        states: np.ndarray,
        true_returns: np.ndarray
    ) -> Dict[str, float]:
        """
        Compute value function accuracy
        
        Args:
            model: Model with value function
            states: State array
            true_returns: True discounted returns
        
        Returns:
            Dictionary with accuracy metrics
        """
        model.eval()
        
        states_t = torch.FloatTensor(states).to(self.device)
        true_returns_t = torch.FloatTensor(true_returns).to(self.device)
        
        with torch.no_grad():
            if hasattr(model, 'forward') and len(list(model.forward.__code__.co_varnames)) > 1:
                # Actor-critic model (PPO)
                _, values = model(states_t)
                values = values.squeeze()
            else:
                # DQN (use max Q-value as proxy for value)
                q_values = model(states_t)
                values = q_values.max(1)[0]
        
        # Compute errors
        errors = (values - true_returns_t).cpu().numpy()
        
        metrics = {
            'mae': float(np.mean(np.abs(errors))),
            'mse': float(np.mean(errors ** 2)),
            'rmse': float(np.sqrt(np.mean(errors ** 2))),
            'explained_variance': float(1 - np.var(errors) / np.var(true_returns))
        }
        
        logger.info(f"Value accuracy: MAE={metrics['mae']:.2f}, ExplainedVar={metrics['explained_variance']:.2%}")
        
        model.train()
        return metrics
    
    def profile_training_step(
        self,
        trainer: Any,
        num_steps: int = 100
    ) -> Dict[str, float]:
        """
        Profile training step performance
        
        Args:
            trainer: Trainer object
            num_steps: Number of steps to profile
        
        Returns:
            Profiling metrics
        """
        logger.info(f"Profiling {num_steps} training steps...")
        
        step_times = []
        memory_usage = []
        
        for i in range(num_steps):
            torch.cuda.synchronize() if self.device.startswith('cuda') else None
            start_time = time.time()
            
            # Training step
            _ = trainer.train_step()
            
            torch.cuda.synchronize() if self.device.startswith('cuda') else None
            end_time = time.time()
            
            step_time = (end_time - start_time) * 1000  # ms
            step_times.append(step_time)
            
            # Measure memory
            if torch.cuda.is_available():
                memory = torch.cuda.memory_allocated(self.device) / (1024 ** 2)
                memory_usage.append(memory)
        
        metrics = {
            'mean_step_time_ms': np.mean(step_times),
            'std_step_time_ms': np.std(step_times),
            'p95_step_time_ms': np.percentile(step_times, 95),
            'throughput_steps_per_sec': 1000 / np.mean(step_times)
        }
        
        if memory_usage:
            metrics['mean_memory_mb'] = np.mean(memory_usage)
            metrics['peak_memory_mb'] = np.max(memory_usage)
        
        logger.info(f"Training step profile:")
        logger.info(f"  Mean: {metrics['mean_step_time_ms']:.2f} ms")
        logger.info(f"  P95: {metrics['p95_step_time_ms']:.2f} ms")
        logger.info(f"  Throughput: {metrics['throughput_steps_per_sec']:.1f} steps/sec")
        
        return metrics
    
    def compare_models(
        self,
        models: Dict[str, nn.Module],
        test_data: List[Dict[str, Any]]
    ) -> Dict[str, Dict[str, float]]:
        """
        Compare multiple models
        
        Args:
            models: Dictionary of model_name -> model
            test_data: Test episodes
        
        Returns:
            Dictionary mapping model_name -> metrics
        """
        logger.info(f"Comparing {len(models)} models...")
        
        all_metrics = {}
        
        for name, model in models.items():
            logger.info(f"Evaluating {name}...")
            metrics = self.evaluate_model(model, test_data)
            all_metrics[name] = metrics
        
        # Find best model per metric
        logger.info("\nBest models per metric:")
        
        metric_names = list(next(iter(all_metrics.values())).keys())
        for metric in metric_names:
            best_model = max(all_metrics.items(), key=lambda x: x[1][metric])
            logger.info(f"  {metric}: {best_model[0]} ({best_model[1][metric]:.4f})")
        
        return all_metrics
    
    def generate_report(
        self,
        archetype: str,
        model_type: str,
        metrics: Dict[str, float],
        training_history: Dict[str, List[float]]
    ) -> str:
        """
        Generate evaluation report
        
        Args:
            archetype: Model archetype
            model_type: Model type
            metrics: Evaluation metrics
            training_history: Training history
        
        Returns:
            Report string
        """
        report = f"""
{'='*60}
MODEL EVALUATION REPORT
{'='*60}

Archetype: {archetype}
Model Type: {model_type}

PERFORMANCE METRICS:
  Win Rate: {metrics.get('win_rate', 0.0):.2%}
  Average Reward: {metrics.get('avg_reward', 0.0):.2f}
  Average Episode Length: {metrics.get('avg_episode_length', 0):.1f} steps
  
EFFICIENCY METRICS:
  Average Latency: {metrics.get('avg_latency_ms', 0.0):.2f} ms
  P95 Latency: {metrics.get('p95_latency_ms', 0.0):.2f} ms
  
QUALITY METRICS:
  Action Entropy: {metrics.get('action_entropy', 0.0):.3f}
  Value MAE: {metrics.get('mae', 0.0):.2f}
  Explained Variance: {metrics.get('explained_variance', 0.0):.2%}

RESOURCE METRICS:
  Model Size (FP16): {metrics.get('fp16_size_mb', 0.0):.2f} MB
  VRAM Usage: {metrics.get('vram_allocated_mb', 0.0):.2f} MB
  Peak VRAM: {metrics.get('vram_max_allocated_mb', 0.0):.2f} MB

TRAINING HISTORY:
  Total Steps: {len(training_history.get('loss', []))}
  Final Loss: {training_history.get('loss', [0.0])[-1]:.4f}
  Best Loss: {min(training_history.get('loss', [float('inf')])):.4f}

{'='*60}
"""
        
        return report


def calculate_win_rate(
    episode_outcomes: List[Dict[str, Any]]
) -> float:
    """
    Calculate win rate from episode outcomes
    
    Args:
        episode_outcomes: List of episode outcome dictionaries
    
    Returns:
        Win rate (0.0 to 1.0)
    """
    if not episode_outcomes:
        return 0.0
    
    wins = sum(1 for ep in episode_outcomes if ep.get('won', False))
    win_rate = wins / len(episode_outcomes)
    
    return win_rate


def calculate_average_reward(
    episode_outcomes: List[Dict[str, Any]]
) -> float:
    """
    Calculate average reward per episode
    
    Args:
        episode_outcomes: List of episode outcome dictionaries
    
    Returns:
        Average reward
    """
    if not episode_outcomes:
        return 0.0
    
    total_reward = sum(ep.get('total_reward', 0.0) for ep in episode_outcomes)
    avg_reward = total_reward / len(episode_outcomes)
    
    return avg_reward


def analyze_training_stability(
    losses: List[float],
    window_size: int = 100
) -> Dict[str, float]:
    """
    Analyze training stability
    
    Args:
        losses: List of training losses
        window_size: Window for computing statistics
    
    Returns:
        Stability metrics
    """
    if len(losses) < window_size:
        window_size = len(losses)
    
    recent_losses = losses[-window_size:]
    
    # Compute trend (linear regression slope)
    x = np.arange(len(recent_losses))
    y = np.array(recent_losses)
    
    if len(x) > 1:
        slope = np.polyfit(x, y, 1)[0]
    else:
        slope = 0.0
    
    metrics = {
        'mean_loss': np.mean(recent_losses),
        'std_loss': np.std(recent_losses),
        'min_loss': np.min(recent_losses),
        'max_loss': np.max(recent_losses),
        'trend': slope,  # Negative = improving, positive = degrading
        'variance': np.var(recent_losses),
        'coefficient_of_variation': np.std(recent_losses) / (np.mean(recent_losses) + 1e-10)
    }
    
    # Assess stability
    if abs(slope) < 0.001 and metrics['coefficient_of_variation'] < 0.3:
        stability = 'stable'
    elif slope < -0.01:
        stability = 'improving'
    elif slope > 0.01:
        stability = 'degrading'
    else:
        stability = 'fluctuating'
    
    metrics['stability_assessment'] = stability
    
    logger.info(f"Training stability: {stability} (trend={slope:.4f}, cv={metrics['coefficient_of_variation']:.2f})")
    
    return metrics


def estimate_convergence(
    losses: List[float],
    threshold: float = 0.001,
    window: int = 100
) -> Dict[str, Any]:
    """
    Estimate if training has converged
    
    Args:
        losses: Training loss history
        threshold: Convergence threshold (relative change)
        window: Window for checking convergence
    
    Returns:
        Convergence metrics
    """
    if len(losses) < window * 2:
        return {
            'converged': False,
            'confidence': 0.0,
            'reason': 'Insufficient data'
        }
    
    # Compare recent window to previous window
    recent = losses[-window:]
    previous = losses[-2*window:-window]
    
    recent_mean = np.mean(recent)
    previous_mean = np.mean(previous)
    
    # Relative change
    if previous_mean > 0:
        relative_change = abs(recent_mean - previous_mean) / previous_mean
    else:
        relative_change = abs(recent_mean - previous_mean)
    
    # Check variance
    recent_std = np.std(recent)
    variance_stable = recent_std < 0.1 * recent_mean
    
    # Convergence criteria
    converged = (relative_change < threshold) and variance_stable
    
    # Confidence (higher if more data and stable)
    confidence = min(1.0, len(losses) / (window * 10))
    if variance_stable:
        confidence *= 1.2
    confidence = min(1.0, confidence)
    
    metrics = {
        'converged': converged,
        'confidence': confidence,
        'relative_change': relative_change,
        'recent_mean': recent_mean,
        'recent_std': recent_std,
        'reason': 'Converged' if converged else 'Still training'
    }
    
    if converged:
        logger.info(f"Model appears to have converged (change={relative_change:.4f} < {threshold})")
    
    return metrics
