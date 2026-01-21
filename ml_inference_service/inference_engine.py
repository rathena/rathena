"""
ONNX Inference Engine
Loads and runs ONNX models with batching and GPU/CPU support
Handles all 54 models (9 types × 6 archetypes)
"""

import onnxruntime as ort
import numpy as np
from typing import Dict, List, Tuple, Optional, Any
import logging
from pathlib import Path
import time
from collections import defaultdict


class ONNXInferenceEngine:
    """
    ONNX Runtime inference engine with batching and caching
    Loads all 54 models (9 types × 6 archetypes) in FP16
    """
    
    def __init__(self, model_dir: str = "/opt/ml_monster_ai/models",
                 device: str = "cuda:0", batch_size: int = 128,
                 precision: str = "fp16"):
        """
        Initialize inference engine
        
        Args:
            model_dir: Directory containing ONNX models
            device: Device to use ('cuda:0' or 'cpu')
            batch_size: Maximum batch size
            precision: Model precision ('fp16' or 'int8')
        """
        self.model_dir = Path(model_dir)
        self.device = device
        self.batch_size = batch_size
        self.precision = precision
        self.logger = logging.getLogger(__name__)
        
        # Parse device
        self.use_gpu = device.startswith('cuda')
        self.device_id = 0
        if self.use_gpu and ':' in device:
            self.device_id = int(device.split(':')[1])
        
        # ONNX Runtime session options
        self.session_options = ort.SessionOptions()
        self.session_options.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL
        self.session_options.execution_mode = ort.ExecutionMode.ORT_SEQUENTIAL
        self.session_options.intra_op_num_threads = 4
        self.session_options.inter_op_num_threads = 2
        
        # Provider list (GPU with TensorRT, fallback to CPU)
        if self.use_gpu:
            self.providers = [
                ('CUDAExecutionProvider', {
                    'device_id': self.device_id,
                    'arena_extend_strategy': 'kNextPowerOfTwo',
                    'gpu_mem_limit': 6 * 1024 * 1024 * 1024,  # 6GB
                    'cudnn_conv_algo_search': 'EXHAUSTIVE',
                }),
                'CPUExecutionProvider'
            ]
        else:
            self.providers = ['CPUExecutionProvider']
        
        # Model registry: {(archetype, model_type): session}
        self.sessions: Dict[Tuple[str, str], ort.InferenceSession] = {}
        
        # Performance stats
        self.inference_count = 0
        self.total_latency_ms = 0.0
        self.model_latencies: Dict[str, List[float]] = defaultdict(list)
    
    def load_all_models(self, archetypes: Optional[List[str]] = None,
                       model_types: Optional[List[str]] = None) -> Tuple[int, int]:
        """
        Load all available ONNX models into memory
        
        Args:
            archetypes: List of archetypes to load (default: all)
            model_types: List of model types to load (default: all)
        
        Returns:
            Tuple of (loaded_count, total_expected)
        """
        if archetypes is None:
            archetypes = ['aggressive', 'defensive', 'support', 'mage', 'tank', 'ranged']
        
        if model_types is None:
            model_types = [
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
        
        total_expected = len(archetypes) * len(model_types)
        loaded_count = 0
        
        self.logger.info(f"Loading {total_expected} ONNX models from {self.model_dir}")
        self.logger.info(f"Device: {self.device}, Precision: {self.precision}")
        
        for archetype in archetypes:
            for model_type in model_types:
                # Try to load model
                success = self._load_single_model(archetype, model_type)
                if success:
                    loaded_count += 1
        
        self.logger.info(f"Successfully loaded {loaded_count}/{total_expected} models")
        
        if loaded_count == 0:
            self.logger.error("No models loaded! Service will operate in fallback mode only.")
        elif loaded_count < total_expected:
            self.logger.warning(
                f"Only {loaded_count}/{total_expected} models loaded. "
                f"Some archetypes/types may not have ML support."
            )
        
        return (loaded_count, total_expected)
    
    def _load_single_model(self, archetype: str, model_type: str) -> bool:
        """
        Load single ONNX model
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            True if loaded successfully, False otherwise
        """
        # Construct model path
        model_filename = f"{model_type}.onnx"
        model_path = self.model_dir / archetype / model_filename
        
        if not model_path.exists():
            self.logger.debug(f"Model not found: {model_path}, skipping")
            return False
        
        try:
            # Create ONNX Runtime session
            session = ort.InferenceSession(
                str(model_path),
                self.session_options,
                providers=self.providers
            )
            
            # Store session
            self.sessions[(archetype, model_type)] = session
            
            # Get model info
            input_name = session.get_inputs()[0].name
            input_shape = session.get_inputs()[0].shape
            output_name = session.get_outputs()[0].name
            output_shape = session.get_outputs()[0].shape
            
            self.logger.info(
                f"✓ Loaded {archetype}/{model_type}: "
                f"input={input_shape}, output={output_shape}",
                extra={
                    'archetype': archetype,
                    'model_type': model_type,
                    'input_name': input_name,
                    'output_name': output_name,
                    'event_type': 'model_loaded'
                }
            )
            
            return True
        
        except Exception as e:
            self.logger.error(
                f"✗ Failed to load {archetype}/{model_type}: {e}",
                extra={
                    'archetype': archetype,
                    'model_type': model_type,
                    'error': str(e),
                    'event_type': 'model_load_failed'
                }
            )
            return False
    
    def infer_batch(self, states: np.ndarray, archetypes: List[str],
                   model_type: str = 'combat_dqn') -> np.ndarray:
        """
        Run batched inference
        
        Args:
            states: State vectors (batch_size, 64)
            archetypes: List of archetype names per state
            model_type: Model type to use for inference
        
        Returns:
            actions: Predicted actions (batch_size,)
        """
        start_time = time.perf_counter()
        
        # Group by archetype for batched inference
        archetype_groups = defaultdict(list)
        for idx, archetype in enumerate(archetypes):
            archetype_groups[archetype].append(idx)
        
        # Initialize results array
        batch_size = states.shape[0]
        actions = np.zeros(batch_size, dtype=np.int32)
        
        # Run inference per archetype group
        for archetype, indices in archetype_groups.items():
            session_key = (archetype, model_type)
            
            if session_key not in self.sessions:
                self.logger.warning(
                    f"No model for {session_key}, using fallback (IDLE action)",
                    extra={'archetype': archetype, 'model_type': model_type}
                )
                actions[indices] = 0  # IDLE action as fallback
                continue
            
            try:
                session = self.sessions[session_key]
                
                # Get states for this group
                group_states = states[indices].astype(np.float32)
                
                # Run inference
                input_name = session.get_inputs()[0].name
                output_name = session.get_outputs()[0].name
                
                outputs = session.run([output_name], {input_name: group_states})
                logits = outputs[0]
                
                # Get actions (argmax over last dimension)
                if len(logits.shape) > 1:
                    group_actions = np.argmax(logits, axis=-1).astype(np.int32)
                else:
                    group_actions = logits.astype(np.int32)
                
                # Assign back to results
                for i, idx in enumerate(indices):
                    if i < len(group_actions):
                        actions[idx] = group_actions[i]
                
            except Exception as e:
                self.logger.error(
                    f"Inference error for {session_key}: {e}",
                    extra={'archetype': archetype, 'model_type': model_type, 'error': str(e)}
                )
                # Fallback to IDLE for this group
                actions[indices] = 0
        
        # Update stats
        latency_ms = (time.perf_counter() - start_time) * 1000
        self.inference_count += batch_size
        self.total_latency_ms += latency_ms
        self.model_latencies[model_type].append(latency_ms)
        
        return actions
    
    def infer_single(self, state: np.ndarray, archetype: str,
                    model_type: str = 'combat_dqn') -> int:
        """
        Run inference for single state
        
        Args:
            state: State vector (64,)
            archetype: Monster archetype
            model_type: Model type to use
        
        Returns:
            action: Predicted action ID
        """
        # Batch inference with single sample
        states = state.reshape(1, -1)
        actions = self.infer_batch(states, [archetype], model_type)
        return int(actions[0])
    
    def get_model_output(self, state: np.ndarray, archetype: str,
                        model_type: str) -> Optional[np.ndarray]:
        """
        Get raw model output (logits) instead of argmax action
        
        Useful for model fusion and L3 caching
        
        Args:
            state: State vector
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            Model output logits or None if model not available
        """
        session_key = (archetype, model_type)
        
        if session_key not in self.sessions:
            return None
        
        try:
            session = self.sessions[session_key]
            
            # Prepare input (add batch dimension if needed)
            if len(state.shape) == 1:
                input_state = state.reshape(1, -1).astype(np.float32)
            else:
                input_state = state.astype(np.float32)
            
            # Run inference
            input_name = session.get_inputs()[0].name
            output_name = session.get_outputs()[0].name
            
            outputs = session.run([output_name], {input_name: input_state})
            logits = outputs[0]
            
            return logits
        
        except Exception as e:
            self.logger.error(f"Failed to get model output for {session_key}: {e}")
            return None
    
    def get_avg_latency_ms(self) -> float:
        """
        Get average latency per inference
        
        Returns:
            Average latency in milliseconds
        """
        if self.inference_count == 0:
            return 0.0
        return self.total_latency_ms / self.inference_count
    
    def get_model_latencies(self) -> Dict[str, Dict[str, float]]:
        """
        Get per-model latency statistics
        
        Returns:
            Dictionary of model latencies with stats
        """
        latency_stats = {}
        
        for model_type, latencies in self.model_latencies.items():
            if latencies:
                latency_stats[model_type] = {
                    'avg_ms': float(np.mean(latencies)),
                    'p50_ms': float(np.percentile(latencies, 50)),
                    'p95_ms': float(np.percentile(latencies, 95)),
                    'p99_ms': float(np.percentile(latencies, 99)),
                    'min_ms': float(np.min(latencies)),
                    'max_ms': float(np.max(latencies)),
                    'count': len(latencies)
                }
        
        return latency_stats
    
    def get_loaded_models(self) -> List[Tuple[str, str]]:
        """
        Get list of loaded models
        
        Returns:
            List of (archetype, model_type) tuples
        """
        return list(self.sessions.keys())
    
    def is_model_loaded(self, archetype: str, model_type: str) -> bool:
        """
        Check if specific model is loaded
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            True if model is loaded
        """
        return (archetype, model_type) in self.sessions
    
    def unload_all_models(self) -> None:
        """Unload all models to free VRAM/RAM"""
        model_count = len(self.sessions)
        self.sessions.clear()
        
        self.logger.info(f"Unloaded {model_count} models")
        
        # Clear CUDA cache if using GPU
        if self.use_gpu:
            try:
                import torch
                torch.cuda.empty_cache()
                self.logger.info("CUDA cache cleared")
            except Exception as e:
                self.logger.warning(f"Failed to clear CUDA cache: {e}")
    
    def unload_model(self, archetype: str, model_type: str) -> bool:
        """
        Unload specific model
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            True if model was loaded and unloaded
        """
        session_key = (archetype, model_type)
        
        if session_key in self.sessions:
            del self.sessions[session_key]
            self.logger.info(f"Unloaded model: {archetype}/{model_type}")
            return True
        
        return False
    
    def get_statistics(self) -> Dict[str, Any]:
        """
        Get inference engine statistics
        
        Returns:
            Statistics dictionary
        """
        return {
            'models_loaded': len(self.sessions),
            'total_inferences': self.inference_count,
            'avg_latency_ms': self.get_avg_latency_ms(),
            'device': self.device,
            'precision': self.precision,
            'batch_size': self.batch_size,
            'model_latencies': self.get_model_latencies()
        }
    
    def get_vram_usage(self) -> Optional[Dict[str, Any]]:
        """
        Get VRAM usage statistics (GPU only)
        
        Returns:
            VRAM usage dictionary or None if not using GPU
        """
        if not self.use_gpu:
            return None
        
        try:
            import torch
            
            torch.cuda.synchronize(self.device_id)
            
            allocated = torch.cuda.memory_allocated(self.device_id)
            reserved = torch.cuda.memory_reserved(self.device_id)
            total = torch.cuda.get_device_properties(self.device_id).total_memory
            
            return {
                'allocated_bytes': allocated,
                'allocated_mb': allocated / (1024**2),
                'allocated_gb': allocated / (1024**3),
                'reserved_bytes': reserved,
                'reserved_mb': reserved / (1024**2),
                'total_bytes': total,
                'total_mb': total / (1024**2),
                'total_gb': total / (1024**3),
                'utilization_percent': (allocated / total) * 100
            }
        
        except Exception as e:
            self.logger.warning(f"Failed to get VRAM usage: {e}")
            return None
    
    def health_check(self) -> Dict[str, Any]:
        """
        Perform health check
        
        Returns:
            Health status dictionary
        """
        # Check if models are loaded
        models_loaded = len(self.sessions) > 0
        
        # Check device availability
        device_available = True
        if self.use_gpu:
            try:
                import torch
                device_available = torch.cuda.is_available()
                if device_available:
                    # Try simple operation
                    test = torch.randn(10, 10, device=f'cuda:{self.device_id}')
                    _ = test @ test
                    torch.cuda.synchronize(self.device_id)
                    del test
            except Exception as e:
                self.logger.warning(f"GPU health check failed: {e}")
                device_available = False
        
        # Calculate health metrics
        avg_latency = self.get_avg_latency_ms()
        
        # Determine status
        if not models_loaded:
            status = 'critical'
            message = 'No models loaded'
        elif not device_available and self.use_gpu:
            status = 'degraded'
            message = 'GPU not available'
        elif avg_latency > 30:
            status = 'degraded'
            message = f'High latency ({avg_latency:.1f}ms)'
        else:
            status = 'healthy'
            message = 'All systems operational'
        
        return {
            'status': status,
            'message': message,
            'models_loaded': len(self.sessions),
            'device_available': device_available,
            'avg_latency_ms': avg_latency,
            'inference_count': self.inference_count
        }
