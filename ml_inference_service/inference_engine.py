"""
ONNX Inference Engine
Loads and runs ONNX models with batching and GPU/CPU support
Handles all 54 models (9 types × 6 archetypes)
Supports hot model reloading without service restart
"""

import onnxruntime as ort
import numpy as np
from typing import Dict, List, Tuple, Optional, Any
import logging
from pathlib import Path
import time
from collections import defaultdict
import asyncio
import shutil
from datetime import datetime


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
        
        # Model metadata: {(archetype, model_type): metadata}
        self.model_metadata: Dict[Tuple[str, str], Dict[str, Any]] = {}
        
        # Thread safety for hot reload
        self.reload_lock = asyncio.Lock()
        
        # Performance stats
        self.inference_count = 0
        self.total_latency_ms = 0.0
        self.model_latencies: Dict[str, List[float]] = defaultdict(list)
        self.reload_count = 0
        self.last_reload_time: Dict[Tuple[str, str], datetime] = {}
    
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
            
            # Store metadata
            self.model_metadata[(archetype, model_type)] = {
                'model_path': str(model_path),
                'input_name': input_name,
                'input_shape': input_shape,
                'output_name': output_name,
                'output_shape': output_shape,
                'loaded_at': datetime.now(),
                'version': model_path.stat().st_mtime  # Use file modification time as version
            }
            
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
            'inference_count': self.inference_count,
            'reload_count': self.reload_count
        }
    
    async def reload_single_model(
        self,
        archetype: str,
        model_type: str,
        model_path: Optional[str] = None
    ) -> bool:
        """
        Hot reload a specific model without disrupting others
        
        Thread-safe atomic model swapping
        
        Args:
            archetype: Monster archetype
            model_type: Model type to reload
            model_path: Path to new ONNX model (default: production path)
        
        Returns:
            True if reload succeeded
        """
        session_key = (archetype, model_type)
        
        # Use lock to ensure thread safety
        async with self.reload_lock:
            self.logger.info(f"Hot reload requested: {archetype}/{model_type}")
            
            try:
                # Step 1: Determine model path
                if model_path is None:
                    model_path = str(self.model_dir / archetype / f"{model_type}.onnx")
                
                model_path_obj = Path(model_path)
                
                if not model_path_obj.exists():
                    self.logger.error(f"Model file not found: {model_path}")
                    return False
                
                # Step 2: Load new model to temporary session
                self.logger.info(f"Loading new model from: {model_path}")
                
                try:
                    new_session = ort.InferenceSession(
                        model_path,
                        self.session_options,
                        providers=self.providers
                    )
                except Exception as e:
                    self.logger.error(f"Failed to load new model: {e}")
                    return False
                
                # Step 3: Verify model dimensions match expected
                input_shape = new_session.get_inputs()[0].shape
                output_shape = new_session.get_outputs()[0].shape
                
                # Check if model already loaded
                if session_key in self.model_metadata:
                    old_input_shape = self.model_metadata[session_key]['input_shape']
                    old_output_shape = self.model_metadata[session_key]['output_shape']
                    
                    # Verify shapes match (except batch dimension)
                    if (input_shape[1:] != old_input_shape[1:] or
                        output_shape[1:] != old_output_shape[1:]):
                        self.logger.error(
                            f"Model shape mismatch: "
                            f"input {old_input_shape} -> {input_shape}, "
                            f"output {old_output_shape} -> {output_shape}"
                        )
                        return False
                
                # Step 4: Run test inference to verify model works
                test_input = np.random.randn(1, 64).astype(np.float32)
                input_name = new_session.get_inputs()[0].name
                output_name = new_session.get_outputs()[0].name
                
                try:
                    test_output = new_session.run([output_name], {input_name: test_input})
                    self.logger.info(f"✓ Test inference successful: output shape {test_output[0].shape}")
                except Exception as e:
                    self.logger.error(f"Test inference failed: {e}")
                    return False
                
                # Step 5: Atomically swap models
                old_session = self.sessions.get(session_key)
                self.sessions[session_key] = new_session
                
                # Step 6: Update metadata
                self.model_metadata[session_key] = {
                    'model_path': model_path,
                    'input_name': input_name,
                    'input_shape': input_shape,
                    'output_name': output_name,
                    'output_shape': output_shape,
                    'loaded_at': datetime.now(),
                    'version': model_path_obj.stat().st_mtime,
                    'reload_count': self.model_metadata.get(session_key, {}).get('reload_count', 0) + 1
                }
                
                # Step 7: Clean up old model (Python garbage collection will handle ONNX session)
                if old_session:
                    del old_session
                    self.logger.info("✓ Old model cleaned up")
                
                # Step 8: Update metrics
                self.reload_count += 1
                self.last_reload_time[session_key] = datetime.now()
                
                self.logger.info(
                    f"✓ Hot reload complete: {archetype}/{model_type} "
                    f"(reload #{self.model_metadata[session_key].get('reload_count', 1)})"
                )
                
                return True
            
            except Exception as e:
                self.logger.error(f"Hot reload failed with exception: {e}", exc_info=True)
                return False
    
    async def reload_all_models(self) -> Dict[str, bool]:
        """
        Reload all currently loaded models
        
        Useful after mass model update
        
        Returns:
            Dictionary mapping (archetype, model_type) to reload success
        """
        results = {}
        
        for archetype, model_type in list(self.sessions.keys()):
            success = await self.reload_single_model(archetype, model_type)
            results[(archetype, model_type)] = success
        
        successes = sum(1 for s in results.values() if s)
        total = len(results)
        
        self.logger.info(
            f"Bulk reload complete: {successes}/{total} models reloaded successfully"
        )
        
        return results
    
    def get_model_metadata(
        self,
        archetype: str,
        model_type: str
    ) -> Optional[Dict[str, Any]]:
        """
        Get metadata for loaded model
        
        Args:
            archetype: Monster archetype
            model_type: Model type
        
        Returns:
            Metadata dictionary or None
        """
        session_key = (archetype, model_type)
        return self.model_metadata.get(session_key)
    
    def get_all_model_metadata(self) -> Dict[Tuple[str, str], Dict[str, Any]]:
        """
        Get metadata for all loaded models
        
        Returns:
            Dictionary mapping (archetype, model_type) to metadata
        """
        return self.model_metadata.copy()
    
    # ========================================================================
    # PACK COORDINATION (CTDE Integration)
    # ========================================================================
    
    async def infer_pack_coordination(
        self,
        pack_monsters: List[Dict[str, Any]],
        signal_coordinator,
        graph_manager,
        coordination_threshold: float = 0.7
    ) -> List[Dict[str, Any]]:
        """
        Coordinate actions for a pack of monsters using decentralized actors
        
        Workflow:
        1. Identify pack structure from Apache AGE graph
        2. Load decentralized actor models for each monster
        3. Process signals for pack coordination
        4. Run parallel inference for all pack members
        5. Apply coordination bonus if synergy detected
        6. Return coordinated actions
        
        Args:
            pack_monsters: List of monster dicts with state, archetype, monster_id
            signal_coordinator: SignalCoordinator instance for signal processing
            graph_manager: AGEGraphManager for pack structure queries
            coordination_threshold: Minimum coordination score for bonus
        
        Returns:
            List of action dicts with coordination metadata
        """
        start_time = time.perf_counter()
        
        if not pack_monsters or len(pack_monsters) < 2:
            # Not a pack, process individually
            return await self._process_individual_monsters(pack_monsters)
        
        pack_size = len(pack_monsters)
        monster_ids = [m['monster_id'] for m in pack_monsters]
        
        self.logger.debug(
            f"Processing pack coordination: {pack_size} monsters {monster_ids}"
        )
        
        # Step 1: Detect pack formation from graph
        pack_info = await self.detect_pack_formation(monster_ids, graph_manager)
        
        if pack_info is None:
            # No pack structure found, process individually
            self.logger.debug(f"No pack structure found for {monster_ids}, processing individually")
            return await self._process_individual_monsters(pack_monsters)
        
        # Step 2: Process signals for coordination
        signals_dict = await signal_coordinator.process_coordination_signals(monster_ids)
        
        # Step 3: Run inference for each pack member with signals
        coordinated_actions = []
        
        for monster in pack_monsters:
            monster_id = monster['monster_id']
            state = monster['state']
            archetype = monster['archetype']
            
            # Get signals for this monster
            signal_vector = signals_dict.get(monster_id, np.zeros(32, dtype=np.float32))
            
            # Run decentralized actor inference
            # For now, use existing model (will be replaced with CTDE actor in production)
            action = await self._infer_with_signals(state, archetype, signal_vector)
            
            coordinated_actions.append({
                'monster_id': monster_id,
                'action': action,
                'archetype': archetype,
                'has_signals': signal_vector.sum() > 0.0,
                'pack_role': pack_info.get('roles', {}).get(monster_id, 'member')
            })
        
        # Step 4: Apply coordination bonus if pack is well-coordinated
        coordination_score = self._calculate_coordination_score(
            coordinated_actions,
            pack_info,
            signals_dict
        )
        
        if coordination_score >= coordination_threshold:
            for action_dict in coordinated_actions:
                action_dict['coordination_bonus'] = coordination_score
                action_dict['coordinated'] = True
        else:
            for action_dict in coordinated_actions:
                action_dict['coordination_bonus'] = 0.0
                action_dict['coordinated'] = False
        
        # Record metrics
        latency_ms = (time.perf_counter() - start_time) * 1000
        
        self.logger.debug(
            f"Pack coordination complete: {pack_size} monsters, "
            f"coordination_score={coordination_score:.2f}, latency={latency_ms:.2f}ms"
        )
        
        return coordinated_actions
    
    async def detect_pack_formation(
        self,
        monster_ids: List[int],
        graph_manager
    ) -> Optional[Dict[str, Any]]:
        """
        Detect pack formation using Apache AGE graph
        
        Queries graph for:
        - LEADS edges (leader-follower relationships)
        - TEAMMATE_OF edges (team membership)
        - Pack structure and roles
        
        Args:
            monster_ids: List of monster IDs to check
            graph_manager: AGEGraphManager instance
        
        Returns:
            Pack formation info dict or None if no pack structure
        """
        if not monster_ids:
            return None
        
        try:
            # Query pack structure for first monster (representative)
            representative_id = monster_ids[0]
            
            team_data = await graph_manager.get_monster_team(representative_id)
            
            if not team_data:
                return None
            
            team_info = team_data[0]
            
            # Extract pack information
            leader_id = team_info.get('leader_id')
            follower_ids = team_info.get('follower_ids', [])
            teammate_ids = team_info.get('teammate_ids', [])
            role = team_info.get('role', 'independent')
            
            # Build pack member set
            pack_members = set([leader_id] if leader_id else [])
            if follower_ids and follower_ids != [None]:
                pack_members.update(follower_ids)
            if teammate_ids and teammate_ids != [None]:
                pack_members.update(teammate_ids)
            pack_members.discard(None)
            
            # Check if requested monsters are in same pack
            requested_set = set(monster_ids)
            if not requested_set.issubset(pack_members):
                self.logger.debug(
                    f"Requested monsters {monster_ids} not in same pack {list(pack_members)}"
                )
                return None
            
            # Determine roles
            roles = {}
            if leader_id:
                roles[leader_id] = 'leader'
            for fid in (follower_ids or []):
                if fid and fid != None:
                    roles[fid] = 'follower'
            for tid in (teammate_ids or []):
                if tid and tid != None:
                    roles[tid] = 'member'
            
            pack_info = {
                'leader_id': leader_id,
                'pack_size': len(pack_members),
                'pack_members': list(pack_members),
                'roles': roles,
                'structure': 'hierarchical' if leader_id else 'flat'
            }
            
            self.logger.debug(f"Detected pack formation: {pack_info}")
            
            return pack_info
        
        except Exception as e:
            self.logger.error(f"Failed to detect pack formation: {e}", exc_info=True)
            return None
    
    async def _process_individual_monsters(
        self,
        monsters: List[Dict[str, Any]]
    ) -> List[Dict[str, Any]]:
        """
        Process monsters individually (no coordination)
        
        Args:
            monsters: List of monster dicts
        
        Returns:
            List of action dicts
        """
        actions = []
        
        for monster in monsters:
            state = monster['state']
            archetype = monster['archetype']
            monster_id = monster['monster_id']
            
            # Standard inference without coordination
            action = self.infer_single(state, archetype, model_type='combat_dqn')
            
            actions.append({
                'monster_id': monster_id,
                'action': action,
                'archetype': archetype,
                'coordinated': False,
                'coordination_bonus': 0.0
            })
        
        return actions
    
    async def _infer_with_signals(
        self,
        state: np.ndarray,
        archetype: str,
        signal_vector: np.ndarray
    ) -> int:
        """
        Run inference with signal vector (CTDE actor)
        
        For now, uses existing models (ignores signals).
        When CTDE actor models are deployed, this will use them.
        
        Args:
            state: State vector (64,)
            archetype: Monster archetype
            signal_vector: Signal vector (32,)
        
        Returns:
            action: Predicted action ID
        """
        # Check if CTDE actor model exists
        ctde_actor_key = (archetype, 'pack_coordination')
        
        if ctde_actor_key in self.sessions:
            # Use CTDE actor model (takes state + signals)
            session = self.sessions[ctde_actor_key]
            
            try:
                # Prepare inputs
                state_input = state.reshape(1, -1).astype(np.float32)
                signal_input = signal_vector.reshape(1, -1).astype(np.float32)
                
                # Get input/output names
                input_names = [inp.name for inp in session.get_inputs()]
                output_name = session.get_outputs()[0].name
                
                # Run inference
                feed_dict = {
                    input_names[0]: state_input,
                    input_names[1]: signal_input
                }
                
                outputs = session.run([output_name], feed_dict)
                action_probs = outputs[0]
                
                # Sample action (greedy for inference)
                action = int(np.argmax(action_probs[0]))
                
                return action
            
            except Exception as e:
                self.logger.error(f"CTDE actor inference failed: {e}, falling back")
        
        # Fallback: Use standard model (ignore signals)
        return self.infer_single(state, archetype, model_type='team_coordination')
    
    def _calculate_coordination_score(
        self,
        actions: List[Dict[str, Any]],
        pack_info: Dict[str, Any],
        signals_dict: Dict[int, np.ndarray]
    ) -> float:
        """
        Calculate coordination quality score
        
        Factors:
        - Signal utilization (how many monsters have signals)
        - Pack cohesion (all monsters in pack participated)
        - Leader presence (bonus if leader coordinates)
        - Action diversity (different roles taking different actions)
        
        Args:
            actions: List of action dicts
            pack_info: Pack formation info
            signals_dict: Signals received by each monster
        
        Returns:
            Coordination score (0.0-1.0)
        """
        if not actions:
            return 0.0
        
        # Factor 1: Signal utilization
        monsters_with_signals = sum(
            1 for a in actions if a.get('has_signals', False)
        )
        signal_utilization = monsters_with_signals / len(actions)
        
        # Factor 2: Pack coverage
        pack_members = set(pack_info.get('pack_members', []))
        action_monsters = set(a['monster_id'] for a in actions)
        coverage = len(action_monsters.intersection(pack_members)) / len(pack_members) if pack_members else 0.0
        
        # Factor 3: Leader coordination bonus
        leader_id = pack_info.get('leader_id')
        leader_present = any(a['monster_id'] == leader_id for a in actions)
        leader_bonus = 0.2 if leader_present else 0.0
        
        # Factor 4: Action diversity (not all doing same thing)
        action_values = [a['action'] for a in actions]
        unique_actions = len(set(action_values))
        diversity = min(unique_actions / len(actions), 1.0) if actions else 0.0
        
        # Combined score
        coordination_score = (
            0.4 * signal_utilization +
            0.3 * coverage +
            0.2 * leader_bonus +
            0.1 * diversity
        )
        
        return coordination_score
