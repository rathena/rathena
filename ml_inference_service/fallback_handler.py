"""
5-Level Fallback Handler for ML Inference Service
Manages graceful degradation from GPU FP16 → Traditional AI
"""

import logging
import time
import numpy as np
from enum import Enum
from typing import Optional, Dict, Any
from collections import deque


class FallbackLevel(Enum):
    """5-level fallback hierarchy"""
    GPU_FP16 = 1          # Primary: GPU FP16 inference (10-15ms)
    GPU_INT8 = 2          # VRAM pressure: GPU INT8 quantized (20-25ms)
    CPU_INT8 = 3          # GPU failure: CPU INT8 multi-threaded (40-60ms)
    RULE_BASED_ML = 4     # CPU overload: Simple rule-based ML (80-100ms)
    TRADITIONAL_AI = 5    # Complete failure: Return special code for C++ AI


class FallbackHandler:
    """
    Manages 5-level fallback system with automatic detection and switching
    
    Automatically degrades on errors or performance issues
    Attempts recovery when conditions improve
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """
        Initialize fallback handler
        
        Args:
            config: Fallback configuration dict
        """
        self.logger = logging.getLogger(__name__)
        
        # Configuration
        if config is None:
            config = {}
        
        self.enabled = config.get('enabled', True)
        self.max_consecutive_errors = config.get('max_consecutive_errors', 3)
        self.recovery_check_interval = config.get('recovery_check_interval_seconds', 60)
        self.auto_recovery = config.get('auto_recovery', True)
        
        # Current state
        self.current_level = FallbackLevel.GPU_FP16
        self.consecutive_errors = 0
        self.last_recovery_check = time.time()
        
        # Performance tracking
        self.recent_latencies = deque(maxlen=100)
        self.recent_errors = deque(maxlen=100)
        
        # Performance thresholds per level (milliseconds)
        self.max_latency_ms = {
            FallbackLevel.GPU_FP16: 20.0,
            FallbackLevel.GPU_INT8: 30.0,
            FallbackLevel.CPU_INT8: 80.0,
            FallbackLevel.RULE_BASED_ML: 120.0,
            FallbackLevel.TRADITIONAL_AI: float('inf')
        }
        
        # Level descriptions
        self.level_names = {
            FallbackLevel.GPU_FP16: "GPU FP16 (Nominal)",
            FallbackLevel.GPU_INT8: "GPU INT8 (Quantized)",
            FallbackLevel.CPU_INT8: "CPU INT8 (Multi-threaded)",
            FallbackLevel.RULE_BASED_ML: "Rule-Based ML (Simplified)",
            FallbackLevel.TRADITIONAL_AI: "Traditional AI (Fallback)"
        }
        
        self.logger.info(f"Fallback handler initialized at level {self.current_level.name}")
    
    def handle_error(self, error: Exception, context: str = "") -> None:
        """
        Handle inference error and potentially escalate fallback
        
        Args:
            error: Exception that occurred
            context: Error context for logging
        """
        self.consecutive_errors += 1
        
        self.logger.warning(
            f"Inference error (#{self.consecutive_errors}): {error}",
            extra={
                'consecutive_errors': self.consecutive_errors,
                'current_level': self.current_level.value,
                'context': context,
                'error_type': type(error).__name__
            }
        )
        
        # Record error
        self.recent_errors.append(1.0)
        
        # Escalate if too many consecutive errors
        if self.consecutive_errors >= self.max_consecutive_errors:
            self._escalate_fallback(f"Too many consecutive errors: {self.consecutive_errors}")
            self.consecutive_errors = 0
    
    def handle_latency_violation(self, latency_ms: float) -> None:
        """
        Handle latency exceeding threshold for current level
        
        Args:
            latency_ms: Measured latency in milliseconds
        """
        threshold = self.max_latency_ms[self.current_level]
        
        # Record latency
        self.recent_latencies.append(latency_ms)
        
        # Check if significantly over threshold (50% margin)
        if latency_ms > threshold * 1.5:
            self.logger.warning(
                f"Latency violation: {latency_ms:.1f}ms > {threshold:.1f}ms threshold",
                extra={
                    'latency_ms': latency_ms,
                    'threshold_ms': threshold,
                    'current_level': self.current_level.value,
                    'overage_percent': (latency_ms / threshold - 1.0) * 100
                }
            )
            
            self._escalate_fallback(f"Latency {latency_ms:.1f}ms exceeds threshold {threshold:.1f}ms")
    
    def record_success(self, latency_ms: float) -> None:
        """
        Record successful inference
        
        Args:
            latency_ms: Inference latency
        """
        self.consecutive_errors = 0
        self.recent_latencies.append(latency_ms)
        self.recent_errors.append(0.0)
    
    def _escalate_fallback(self, reason: str) -> None:
        """
        Move to next fallback level
        
        Args:
            reason: Reason for escalation
        """
        if not self.enabled:
            self.logger.warning("Fallback escalation requested but fallback is disabled")
            return
        
        if self.current_level == FallbackLevel.TRADITIONAL_AI:
            self.logger.error(
                f"Already at lowest fallback level (TRADITIONAL_AI), cannot escalate further. Reason: {reason}"
            )
            return
        
        old_level = self.current_level
        next_level_value = self.current_level.value + 1
        self.current_level = FallbackLevel(next_level_value)
        
        self.logger.critical(
            f"FALLBACK ESCALATION: {self.level_names[old_level]} → {self.level_names[self.current_level]}",
            extra={
                'old_level': old_level.value,
                'new_level': self.current_level.value,
                'reason': reason,
                'event_type': 'fallback_escalation'
            }
        )
    
    def try_recover(self) -> bool:
        """
        Attempt to recover to higher fallback level
        
        Returns:
            True if recovery attempted, False otherwise
        """
        if not self.enabled or not self.auto_recovery:
            return False
        
        # Check if enough time passed since last check
        current_time = time.time()
        if current_time - self.last_recovery_check < self.recovery_check_interval:
            return False
        
        self.last_recovery_check = current_time
        
        # Already at best level
        if self.current_level == FallbackLevel.GPU_FP16:
            return False
        
        # Check if conditions allow recovery
        target_level_value = self.current_level.value - 1
        target_level = FallbackLevel(target_level_value)
        
        if self._can_recover_to_level(target_level):
            old_level = self.current_level
            self.current_level = target_level
            
            self.logger.info(
                f"FALLBACK RECOVERY: {self.level_names[old_level]} → {self.level_names[self.current_level]}",
                extra={
                    'old_level': old_level.value,
                    'new_level': self.current_level.value,
                    'event_type': 'fallback_recovery'
                }
            )
            
            return True
        
        return False
    
    def _can_recover_to_level(self, target_level: FallbackLevel) -> bool:
        """
        Check if conditions allow recovery to target level
        
        Args:
            target_level: Target fallback level
        
        Returns:
            True if recovery is safe
        """
        # Need enough data points
        if len(self.recent_latencies) < 20 or len(self.recent_errors) < 20:
            return False
        
        # Calculate recent metrics
        avg_latency = np.mean(list(self.recent_latencies))
        p95_latency = np.percentile(list(self.recent_latencies), 95)
        error_rate = np.mean(list(self.recent_errors))
        
        # Level-specific recovery criteria
        if target_level == FallbackLevel.GPU_FP16:
            # Recovery to GPU FP16 requires:
            # - GPU available
            # - Latency < 15ms avg, < 18ms p95
            # - Error rate < 3%
            return (
                self._check_gpu_available() and
                avg_latency < 15.0 and
                p95_latency < 18.0 and
                error_rate < 0.03
            )
        
        elif target_level == FallbackLevel.GPU_INT8:
            # Recovery to GPU INT8 requires:
            # - GPU available
            # - Latency < 25ms avg
            # - Error rate < 5%
            return (
                self._check_gpu_available() and
                avg_latency < 25.0 and
                error_rate < 0.05
            )
        
        elif target_level == FallbackLevel.CPU_INT8:
            # Recovery to CPU INT8 requires:
            # - Latency < 60ms avg
            # - Error rate < 10%
            return (
                avg_latency < 60.0 and
                error_rate < 0.10
            )
        
        elif target_level == FallbackLevel.RULE_BASED_ML:
            # Recovery to rule-based requires:
            # - Latency < 100ms avg
            # - Error rate < 20%
            return (
                avg_latency < 100.0 and
                error_rate < 0.20
            )
        
        return False
    
    def _check_gpu_available(self) -> bool:
        """
        Check if GPU is available and functional
        
        Returns:
            True if GPU can be used
        """
        try:
            import torch
            
            if not torch.cuda.is_available():
                return False
            
            # Try simple GPU operation
            test_tensor = torch.randn(100, 100, device='cuda:0')
            result = test_tensor @ test_tensor
            torch.cuda.synchronize()
            
            del test_tensor
            del result
            torch.cuda.empty_cache()
            
            return True
        
        except Exception as e:
            self.logger.warning(f"GPU health check failed: {e}")
            return False
    
    def get_action_for_level(self, state: np.ndarray) -> Optional[int]:
        """
        Get action based on current fallback level
        
        For levels 1-3, returns None (normal inference should be attempted)
        For level 4, returns rule-based action
        For level 5, returns 255 (special code for traditional AI)
        
        Args:
            state: State vector (64 dims)
        
        Returns:
            Action ID or None if normal inference should proceed
        """
        if self.current_level == FallbackLevel.TRADITIONAL_AI:
            # Return special action code (255) that tells C++ to use traditional AI
            return 255
        
        elif self.current_level == FallbackLevel.RULE_BASED_ML:
            # Use simple rule-based decision
            return self._rule_based_action(state)
        
        # For GPU/CPU levels, return None to indicate normal inference
        return None
    
    def _rule_based_action(self, state: np.ndarray) -> int:
        """
        Simple rule-based action selection
        Uses state vector features to make basic decisions
        
        Args:
            state: State vector (64 dimensions)
                   [0]: HP ratio
                   [1]: SP ratio
                   [2-4]: Position (x, y, z)
                   [30-35]: Enemy distance/count features
                   [32]: Threat level
        
        Returns:
            Action ID (0-9)
        """
        # Extract key features with safety checks
        hp_ratio = float(state[0]) if len(state) > 0 else 1.0
        sp_ratio = float(state[1]) if len(state) > 1 else 1.0
        nearest_enemy_dist = float(state[30]) if len(state) > 30 else 10.0
        threat_level = float(state[32]) if len(state) > 32 else 0.0
        
        # Action IDs (matching C++ MLAction enum)
        # 0=IDLE, 1=ATTACK, 2=MOVE_CLOSER, 3=MOVE_AWAY, 4=MOVE_RANDOM,
        # 5=SKILL_1, 6=SKILL_2, 7=SKILL_3, 8=CHANGE_TARGET, 9=FLEE
        
        # Rule-based logic
        if hp_ratio < 0.2:
            # Critical HP - flee
            return 9  # FLEE
        
        elif hp_ratio < 0.3 and threat_level > 0.7:
            # Low HP and high threat - move away
            return 3  # MOVE_AWAY
        
        elif threat_level > 0.8:
            # Very high threat - defensive
            return 3  # MOVE_AWAY
        
        elif nearest_enemy_dist < 0.1:
            # Enemy very close - attack
            return 1  # ATTACK
        
        elif nearest_enemy_dist < 0.3:
            # Enemy close - attack or use skill
            if sp_ratio > 0.3:
                return 5  # SKILL_1
            else:
                return 1  # ATTACK
        
        elif nearest_enemy_dist < 0.6:
            # Enemy in range - move closer
            return 2  # MOVE_CLOSER
        
        else:
            # Enemy far - random movement or idle
            return 4  # MOVE_RANDOM
    
    def get_current_level(self) -> FallbackLevel:
        """Get current fallback level"""
        return self.current_level
    
    def force_level(self, level: FallbackLevel) -> None:
        """
        Force fallback to specific level (for testing/manual intervention)
        
        Args:
            level: Target fallback level
        """
        old_level = self.current_level
        self.current_level = level
        
        self.logger.warning(
            f"Fallback level forced: {self.level_names[old_level]} → {self.level_names[level]}",
            extra={
                'old_level': old_level.value,
                'new_level': level.value,
                'forced': True,
                'event_type': 'fallback_forced'
            }
        )
    
    def get_statistics(self) -> Dict[str, Any]:
        """
        Get fallback handler statistics
        
        Returns:
            Statistics dictionary
        """
        if len(self.recent_latencies) == 0:
            avg_latency = 0.0
            p95_latency = 0.0
        else:
            avg_latency = float(np.mean(list(self.recent_latencies)))
            p95_latency = float(np.percentile(list(self.recent_latencies), 95))
        
        error_rate = float(np.mean(list(self.recent_errors))) if len(self.recent_errors) > 0 else 0.0
        
        return {
            'current_level': self.current_level.value,
            'current_level_name': self.level_names[self.current_level],
            'consecutive_errors': self.consecutive_errors,
            'avg_latency_ms': avg_latency,
            'p95_latency_ms': p95_latency,
            'error_rate': error_rate,
            'samples_tracked': len(self.recent_latencies),
            'enabled': self.enabled,
            'auto_recovery': self.auto_recovery
        }
    
    def should_use_fallback_action(self) -> bool:
        """
        Check if should use fallback action instead of model inference
        
        Returns:
            True if should skip model inference and use fallback
        """
        return self.current_level in [FallbackLevel.RULE_BASED_ML, FallbackLevel.TRADITIONAL_AI]
    
    def get_level_for_inference_engine(self) -> str:
        """
        Get precision/device configuration for inference engine
        
        Returns:
            String indicating inference configuration:
            - 'gpu_fp16': Use GPU with FP16 models
            - 'gpu_int8': Use GPU with INT8 models
            - 'cpu_int8': Use CPU with INT8 models
            - 'fallback': Skip model inference
        """
        if self.current_level == FallbackLevel.GPU_FP16:
            return 'gpu_fp16'
        elif self.current_level == FallbackLevel.GPU_INT8:
            return 'gpu_int8'
        elif self.current_level == FallbackLevel.CPU_INT8:
            return 'cpu_int8'
        else:
            return 'fallback'
    
    def check_and_update_level(self) -> bool:
        """
        Check conditions and update fallback level if needed
        
        Called periodically by main service loop
        
        Returns:
            True if level changed, False otherwise
        """
        old_level = self.current_level
        
        # Try recovery if auto-recovery enabled
        if self.auto_recovery:
            self.try_recover()
        
        return old_level != self.current_level
    
    def reset_error_count(self) -> None:
        """Reset consecutive error count (after successful recovery)"""
        self.consecutive_errors = 0
