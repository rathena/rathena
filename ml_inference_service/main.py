"""
ML Inference Service - Main Entry Point
Orchestrates polling, caching, inference, and response writing
Includes HTTP server for hot model reloading
"""

import asyncio
import logging
import signal
import sys
from typing import List, Dict, Any, Optional
import numpy as np
import time
from pathlib import Path
from aiohttp import web

from config import load_config, save_default_config
from logger import setup_logging, get_logger
from inference_engine import ONNXInferenceEngine
from request_processor import RequestProcessor
from cache_manager import CacheManager
from fallback_handler import FallbackHandler, FallbackLevel
from health_monitor import HealthMonitor
from metrics import MetricsCollector
from graph_manager import AGEGraphManager, initialize_graph_manager, AGEConnectionError
from signal_coordinator import SignalCoordinator


class MLInferenceService:
    """
    Main ML Inference Service
    Polls PostgreSQL, checks cache, runs inference, writes responses
    """
    
    def __init__(self, config_path: str = "/opt/ml_monster_ai/configs/inference_config.yaml"):
        """
        Initialize ML Inference Service
        
        Args:
            config_path: Path to configuration file
        """
        # Load configuration
        try:
            self.config = load_config(config_path)
        except FileNotFoundError:
            logging.warning(f"Config file not found at {config_path}, creating default")
            save_default_config(config_path)
            self.config = load_config(config_path)
        
        # Setup logging
        setup_logging(self.config['logging'])
        self.logger = get_logger(__name__)
        
        self.logger.info("="*80)
        self.logger.info("ML Inference Service v2.0 Initializing")
        self.logger.info("="*80)
        
        # Initialize components
        self.inference_engine = ONNXInferenceEngine(
            model_dir=self.config['models']['directory'],
            device=self.config['hardware']['device'],
            batch_size=self.config['inference']['batch_size'],
            precision=self.config['models']['precision']
        )
        
        self.request_processor = RequestProcessor(
            db_config=self.config['database']['postgresql'],
            poll_interval_ms=self.config['inference']['poll_interval_ms']
        )
        
        self.cache_manager = CacheManager(
            redis_config=self.config['database']['redis']
        )
        
        self.fallback_handler = FallbackHandler(
            config=self.config['fallback']
        )
        
        self.health_monitor = HealthMonitor(
            device=self.config['hardware']['device']
        )
        
        self.metrics = MetricsCollector(
            prometheus_port=self.config['monitoring']['prometheus_port']
        )
        
        # Graph manager (initialized later in initialize())
        self.graph_manager: Optional[AGEGraphManager] = None
        self.graph_enabled = self.config.get('graph', {}).get('enabled', False)
        
        # Signal coordinator (initialized later in initialize())
        self.signal_coordinator: Optional[SignalCoordinator] = None
        self.pack_coordination_enabled = self.config.get('pack_coordination', {}).get('enabled', True)
        
        # Service state
        self.running = False
        self.requests_processed = 0
        self.batch_count = 0
        
        # Performance tracking
        self.last_cleanup_time = time.time()
        self.last_health_check_time = time.time()
        self.last_metrics_update_time = time.time()
        
        # HTTP server for hot reload (initialized in start_http_server)
        self.http_app: Optional[web.Application] = None
        self.http_runner: Optional[web.AppRunner] = None
        self.http_port = self.config.get('http_server', {}).get('port', 8080)
    
    async def initialize(self) -> None:
        """Initialize all components"""
        self.logger.info("Initializing components...")
        
        # Initialize database connections
        try:
            await self.request_processor.initialize()
        except Exception as e:
            self.logger.critical(f"Failed to initialize database: {e}")
            raise
        
        # Test Redis connection
        if not self.cache_manager.health_check():
            self.logger.warning("Redis connection failed, cache will be disabled")
        else:
            self.logger.info("Redis connection OK")
        
        # Load ONNX models
        self.logger.info("Loading ONNX models...")
        loaded, total = self.inference_engine.load_all_models(
            archetypes=self.config['models']['archetypes'],
            model_types=self.config['models']['types']
        )
        
        if loaded == 0:
            self.logger.critical(
                "No models loaded! Service will operate in FALLBACK mode only (Level 5)."
            )
            self.logger.critical(
                "All requests will be routed to traditional AI (action_id=255)."
            )
            # Force fallback to traditional AI
            self.fallback_handler.force_level(FallbackLevel.TRADITIONAL_AI)
        elif loaded < total:
            self.logger.warning(
                f"Only {loaded}/{total} models loaded. "
                f"Some archetypes may not have full ML support."
            )
        else:
            self.logger.info(f"✓ All {loaded} models loaded successfully")
        
        # Update model metrics
        self.metrics.update_model_metrics(self.inference_engine.get_loaded_models())
        
        # Start metrics server
        self.metrics.start_server()
        self.metrics.set_service_info(
            version='2.0.0',
            device=self.config['hardware']['device'],
            precision=self.config['models']['precision']
        )
        
        # Initialize Apache AGE graph manager
        if self.graph_enabled:
            try:
                self.logger.info("Initializing Apache AGE graph manager...")
                self.graph_manager = await initialize_graph_manager(
                    db_pool=self.request_processor.pool,
                    graph_name=self.config.get('graph', {}).get('graph_name', 'monster_ai')
                )
                
                # Get graph statistics
                graph_stats = await self.graph_manager.get_graph_statistics()
                self.logger.info(
                    f"✓ Graph manager initialized: {graph_stats['total_nodes']} nodes, "
                    f"{graph_stats['total_edges']} edges, "
                    f"{graph_stats['avg_query_time_ms']:.2f}ms avg query time"
                )
            except AGEConnectionError as e:
                self.logger.error(f"Failed to initialize graph manager: {e}")
                self.logger.warning("Graph features will be disabled")
                self.graph_enabled = False
                self.graph_manager = None
            except Exception as e:
                self.logger.error(f"Unexpected error initializing graph manager: {e}", exc_info=True)
                self.graph_enabled = False
                self.graph_manager = None
        else:
            self.logger.info("Graph integration disabled in configuration")
        
        # Initialize signal coordinator (requires graph manager)
        if self.pack_coordination_enabled and self.graph_enabled and self.graph_manager:
            try:
                self.logger.info("Initializing signal coordinator for pack coordination...")
                self.signal_coordinator = SignalCoordinator(
                    graph_manager=self.graph_manager,
                    signal_dim=32,
                    signal_range=self.config.get('pack_coordination', {}).get('signal_range', 15),
                    signal_ttl=self.config.get('pack_coordination', {}).get('signal_ttl', 5)
                )
                self.logger.info("✓ Signal coordinator initialized")
            except Exception as e:
                self.logger.error(f"Failed to initialize signal coordinator: {e}")
                self.pack_coordination_enabled = False
                self.signal_coordinator = None
        else:
            self.logger.info(
                f"Pack coordination disabled "
                f"(pack_enabled={self.pack_coordination_enabled}, "
                f"graph_enabled={self.graph_enabled})"
            )
        
        # Start HTTP server for hot reload
        await self.start_http_server()
        
        self.logger.info("✓ ML Inference Service initialized successfully")
        self.logger.info(f"  Models loaded: {loaded}/{total}")
        self.logger.info(f"  Fallback level: {self.fallback_handler.get_current_level().name}")
        self.logger.info(f"  Cache enabled: {self.config['caching']['enabled']}")
        self.logger.info(f"  Graph enabled: {self.graph_enabled}")
        self.logger.info(f"  Pack coordination enabled: {self.pack_coordination_enabled}")
        self.logger.info(f"  Batch size: {self.config['inference']['batch_size']}")
        self.logger.info(f"  Prometheus port: {self.config['monitoring']['prometheus_port']}")
        self.logger.info(f"  HTTP reload port: {self.http_port}")
    
    async def process_batch(self, requests: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """
        Process a batch of inference requests
        
        Workflow:
        1. Check cache (L2)
        2. Run inference for cache misses
        3. Update cache
        4. Return responses
        
        Args:
            requests: List of request dictionaries
        
        Returns:
            List of response dictionaries
        """
        batch_start = time.perf_counter()
        responses = []
        
        # Query graph for pack coordination data (if enabled)
        pack_coordination_data = {}
        if self.graph_enabled and self.graph_manager and self.config.get('graph', {}).get('use_for_coordination', True):
            try:
                for req in requests:
                    monster_id = req['monster_id']
                    
                    # Get pack team info for coordination
                    team_info = await self.graph_manager.get_monster_team(monster_id)
                    if team_info:
                        pack_coordination_data[monster_id] = team_info
                    
                    # Get threat network if enabled
                    if self.config.get('graph', {}).get('use_for_threat_tracking', True):
                        threats = await self.graph_manager.get_threat_network(monster_id, radius=2)
                        if threats:
                            pack_coordination_data[monster_id] = pack_coordination_data.get(monster_id, {})
                            pack_coordination_data[monster_id]['threats'] = threats
                
                self.logger.debug(f"Queried graph data for {len(pack_coordination_data)} monsters")
            except Exception as e:
                self.logger.warning(f"Graph query failed (non-critical): {e}")
                # Don't fail inference if graph queries fail
        
        # Separate cache hits and misses
        cache_hits = []
        cache_misses = []
        
        for req in requests:
            state_vec = np.array(req['state_vector'], dtype=np.float32)
            
            # Try L2 cache
            if self.config['caching']['enabled']:
                cached_result = self.cache_manager.check_l2_cache(
                    req['monster_id'],
                    state_vec
                )
                
                if cached_result is not None:
                    action_id, confidence = cached_result
                    cache_hits.append((req, action_id, confidence))
                    self.metrics.cache_hits_total.labels(cache_layer='l2').inc()
                else:
                    cache_misses.append(req)
                    self.metrics.cache_misses_total.inc()
            else:
                cache_misses.append(req)
        
        # Process cache hits (instant response)
        for req, action_id, confidence in cache_hits:
            responses.append({
                'request_id': req['request_id'],
                'monster_id': req['monster_id'],
                'action_type': 'combat',
                'action_id': action_id,
                'action_params': {},
                'model_outputs': {'cached': True},
                'confidence': confidence,
                'fusion_weights': {},
                'coordination_action': None,
                'target_monster_id': None,
                'inference_latency_ms': 0.5,  # Approximate cache latency
                'cache_used': True,
                'fallback_level': self.fallback_handler.get_current_level().value
            })
        
        # Process cache misses (run inference)
        if cache_misses:
            inference_start = time.perf_counter()
            
            try:
                # Check if should use fallback action
                if self.fallback_handler.should_use_fallback_action():
                    # Use fallback actions
                    for req in cache_misses:
                        state_vec = np.array(req['state_vector'], dtype=np.float32)
                        fallback_action = self.fallback_handler.get_action_for_level(state_vec)
                        
                        responses.append({
                            'request_id': req['request_id'],
                            'monster_id': req['monster_id'],
                            'action_type': 'fallback',
                            'action_id': fallback_action,
                            'action_params': {},
                            'model_outputs': {'fallback': True},
                            'confidence': 0.5 if fallback_action != 255 else 0.0,
                            'fusion_weights': {},
                            'coordination_action': None,
                            'target_monster_id': None,
                            'inference_latency_ms': 1.0,
                            'cache_used': False,
                            'fallback_level': self.fallback_handler.get_current_level().value
                        })
                else:
                    # Run model inference
                    states = np.array([r['state_vector'] for r in cache_misses], dtype=np.float32)
                    archetypes = [r['archetype'] for r in cache_misses]
                    
                    # Run primary model (combat_dqn)
                    actions = self.inference_engine.infer_batch(
                        states,
                        archetypes,
                        model_type=self.config['inference']['primary_model']
                    )
                    
                    inference_time = (time.perf_counter() - inference_start) * 1000  # ms
                    
                    # Create responses
                    for idx, req in enumerate(cache_misses):
                        action = int(actions[idx])
                        
                        response = {
                            'request_id': req['request_id'],
                            'monster_id': req['monster_id'],
                            'action_type': 'combat',
                            'action_id': action,
                            'action_params': {},
                            'model_outputs': {
                                self.config['inference']['primary_model']: int(action)
                            },
                            'confidence': 0.85,  # From model
                            'fusion_weights': {
                                self.config['inference']['primary_model']: 1.0
                            },
                            'coordination_action': None,
                            'target_monster_id': None,
                            'inference_latency_ms': inference_time / len(cache_misses),
                            'cache_used': False,
                            'fallback_level': self.fallback_handler.get_current_level().value
                        }
                        
                        responses.append(response)
                        
                        # Update cache
                        if self.config['caching']['enabled']:
                            state_vec = states[idx]
                            self.cache_manager.update_l2_cache(
                                req['monster_id'],
                                state_vec,
                                action,
                                0.85
                            )
                    
                    # Check latency and handle violations
                    avg_latency = inference_time / len(cache_misses)
                    if avg_latency > self.config['inference']['max_latency_ms']:
                        self.fallback_handler.handle_latency_violation(avg_latency)
                    else:
                        self.fallback_handler.record_success(avg_latency)
                    
                    # Record metrics
                    self.metrics.record_batch(len(cache_misses), inference_time / 1000)
            
            except Exception as e:
                self.logger.error(f"Inference error: {e}", exc_info=True)
                self.fallback_handler.handle_error(e, context="batch_inference")
                
                # Create fallback responses for failed requests
                for req in cache_misses:
                    responses.append({
                        'request_id': req['request_id'],
                        'monster_id': req['monster_id'],
                        'action_type': 'error_fallback',
                        'action_id': 255,  # Traditional AI
                        'action_params': {},
                        'model_outputs': {'error': str(e)},
                        'confidence': 0.0,
                        'fusion_weights': {},
                        'coordination_action': None,
                        'target_monster_id': None,
                        'inference_latency_ms': 0.0,
                        'cache_used': False,
                        'fallback_level': 5  # Force traditional AI on error
                    })
        
        # Update spatial relationships in graph after inference (if enabled)
        if self.graph_enabled and self.graph_manager and self.config.get('graph', {}).get('use_for_spatial', True):
            try:
                spatial_updates = []
                for req in requests:
                    spatial_updates.append({
                        'monster_id': req['monster_id'],
                        'x': req['position_x'],
                        'y': req['position_y'],
                        'hp_ratio': req['hp_ratio'],
                        'map_id': req['map_id']
                    })
                
                if spatial_updates:
                    updated = await self.graph_manager.bulk_update_positions(spatial_updates)
                    self.logger.debug(f"Updated spatial data for {updated} monsters in graph")
            except Exception as e:
                self.logger.warning(f"Failed to update spatial relationships (non-critical): {e}")
        
        batch_time = (time.perf_counter() - batch_start) * 1000  # ms
        
        self.logger.debug(
            f"Processed batch of {len(requests)} "
            f"(cache hits: {len(cache_hits)}, misses: {len(cache_misses)}) "
            f"in {batch_time:.1f}ms"
        )
        
        return responses
    
    async def process_batch_with_coordination(
        self,
        requests: List[Dict[str, Any]]
    ) -> List[Dict[str, Any]]:
        """
        Process requests with pack coordination
        
        Workflow:
        1. Group requests by pack (using Apache AGE graph)
        2. For each pack:
           a. Detect formation
           b. Process signals
           c. Coordinate actions
        3. For solo monsters:
           a. Standard inference
        4. Return all responses
        
        Args:
            requests: List of request dictionaries
        
        Returns:
            List of response dictionaries with coordination metadata
        """
        batch_start = time.perf_counter()
        
        if not self.pack_coordination_enabled or not self.signal_coordinator:
            # Fall back to standard processing
            return await self.process_batch(requests)
        
        self.logger.debug(f"Processing {len(requests)} requests with pack coordination")
        
        # Step 1: Group requests by pack
        pack_groups = await self._group_requests_by_pack(requests)
        
        # Step 2: Process each pack and solo monsters
        all_responses = []
        
        # Process packs (2+ monsters)
        for pack_id, pack_requests in pack_groups['packs'].items():
            if len(pack_requests) < 2:
                continue  # Skip, will be processed as solo
            
            try:
                # Prepare pack monster data
                pack_monsters = [
                    {
                        'monster_id': req['monster_id'],
                        'state': np.array(req['state_vector'], dtype=np.float32),
                        'archetype': req['archetype'],
                        'position_x': req['position_x'],
                        'position_y': req['position_y']
                    }
                    for req in pack_requests
                ]
                
                # Run pack coordination
                coordinated_actions = await self.inference_engine.infer_pack_coordination(
                    pack_monsters=pack_monsters,
                    signal_coordinator=self.signal_coordinator,
                    graph_manager=self.graph_manager,
                    coordination_threshold=self.config.get('pack_coordination', {}).get('coordination_threshold', 0.7)
                )
                
                # Convert to responses
                for req, action_dict in zip(pack_requests, coordinated_actions):
                    response = {
                        'request_id': req['request_id'],
                        'monster_id': req['monster_id'],
                        'action_type': 'combat',
                        'action_id': action_dict['action'],
                        'action_params': {},
                        'model_outputs': {'pack_coordination': True},
                        'confidence': 0.9 if action_dict.get('coordinated', False) else 0.7,
                        'fusion_weights': {},
                        'coordination_action': action_dict.get('pack_role'),
                        'coordination_bonus': action_dict.get('coordination_bonus', 0.0),
                        'pack_id': pack_id,
                        'pack_size': len(pack_requests),
                        'target_monster_id': None,
                        'inference_latency_ms': 0.0,  # Will be updated
                        'cache_used': False,
                        'fallback_level': self.fallback_handler.get_current_level().value
                    }
                    
                    all_responses.append(response)
            
            except Exception as e:
                self.logger.error(f"Pack coordination failed for pack {pack_id}: {e}", exc_info=True)
                
                # Fall back to individual processing for this pack
                solo_responses = await self.process_batch(pack_requests)
                all_responses.extend(solo_responses)
        
        # Process solo monsters (no pack)
        if pack_groups['solo']:
            solo_responses = await self.process_batch(pack_groups['solo'])
            all_responses.extend(solo_responses)
        
        # Calculate batch metrics
        batch_time = (time.perf_counter() - batch_start) * 1000  # ms
        
        # Update latency for all responses
        avg_latency = batch_time / len(requests) if requests else 0.0
        for resp in all_responses:
            if resp['inference_latency_ms'] == 0.0:
                resp['inference_latency_ms'] = avg_latency
        
        self.logger.debug(
            f"Pack coordination batch complete: "
            f"{len(pack_groups['packs'])} packs, "
            f"{len(pack_groups['solo'])} solo monsters, "
            f"{batch_time:.2f}ms total"
        )
        
        return all_responses
    
    async def _group_requests_by_pack(
        self,
        requests: List[Dict[str, Any]]
    ) -> Dict[str, Any]:
        """
        Group requests by pack membership
        
        Uses Apache AGE graph to identify pack relationships.
        
        Args:
            requests: List of request dicts
        
        Returns:
            Dict with 'packs' (pack_id -> requests) and 'solo' (list of solo requests)
        """
        pack_assignments: Dict[int, int] = {}  # monster_id -> pack_id
        pack_groups: Dict[int, List[Dict[str, Any]]] = {}
        solo_monsters: List[Dict[str, Any]] = []
        
        if not self.graph_manager:
            # No graph, all are solo
            return {'packs': {}, 'solo': requests}
        
        # Query pack membership for each monster
        for req in requests:
            monster_id = req['monster_id']
            
            try:
                # Get monster's team info
                team_data = await self.graph_manager.get_monster_team(monster_id)
                
                if not team_data:
                    solo_monsters.append(req)
                    continue
                
                team_info = team_data[0]
                role = team_info.get('role', 'independent')
                
                if role == 'independent':
                    solo_monsters.append(req)
                    continue
                
                # Determine pack ID (use leader ID as pack ID)
                leader_id = team_info.get('leader_id')
                if leader_id:
                    pack_id = leader_id
                else:
                    # No leader, use own ID as pack ID
                    pack_id = monster_id
                
                # Add to pack group
                if pack_id not in pack_groups:
                    pack_groups[pack_id] = []
                
                pack_groups[pack_id].append(req)
                pack_assignments[monster_id] = pack_id
            
            except Exception as e:
                self.logger.warning(
                    f"Failed to get pack info for monster {monster_id}: {e}"
                )
                solo_monsters.append(req)
        
        self.logger.debug(
            f"Grouped requests: {len(pack_groups)} packs, {len(solo_monsters)} solo"
        )
        
        return {
            'packs': pack_groups,
            'solo': solo_monsters,
            'assignments': pack_assignments
        }
    
    async def run_loop(self) -> None:
        """Main service loop - polls, processes, responds"""
        self.running = True
        self.logger.info("Starting service loop...")
        
        cleanup_interval = 60  # Cleanup every 60 seconds
        health_check_interval = 10  # Health check every 10 seconds
        metrics_update_interval = 5  # Update metrics every 5 seconds
        
        while self.running:
            try:
                # Poll for requests
                requests = await self.request_processor.poll_requests(
                    max_batch_size=self.config['inference']['batch_size']
                )
                
                if requests:
                    # Process batch (with pack coordination if enabled)
                    if self.pack_coordination_enabled and len(requests) > 1:
                        responses = await self.process_batch_with_coordination(requests)
                    else:
                        responses = await self.process_batch(requests)
                    
                    # Write responses
                    if responses:
                        await self.request_processor.write_responses(responses)
                        
                        self.requests_processed += len(requests)
                        self.batch_count += 1
                        
                        # Record metrics
                        for resp in responses:
                            archetype = 'unknown'
                            for req in requests:
                                if req['request_id'] == resp['request_id']:
                                    archetype = req['archetype']
                                    break
                            
                            self.metrics.record_inference(
                                archetype=archetype,
                                model_type=self.config['inference']['primary_model'],
                                latency_seconds=resp['inference_latency_ms'] / 1000,
                                cache_status='hit' if resp['cache_used'] else 'miss',
                                status='success'
                            )
                else:
                    # No requests, short sleep
                    await asyncio.sleep(self.request_processor.poll_interval)
                
                # Periodic tasks
                current_time = time.time()
                
                # Cleanup old requests (every 60 seconds)
                if current_time - self.last_cleanup_time > cleanup_interval:
                    await self.request_processor.cleanup_old_requests(hours=24)
                    await self.request_processor.cleanup_old_responses(hours=24)
                    self.last_cleanup_time = current_time
                
                # Health check (every 10 seconds)
                if current_time - self.last_health_check_time > health_check_interval:
                    health = self.health_monitor.perform_full_health_check()
                    
                    # Update resource metrics
                    self.metrics.update_resource_metrics(health)
                    
                    # Check fallback level
                    if self.fallback_handler.check_and_update_level():
                        self.logger.warning(
                            f"Fallback level changed to: "
                            f"{self.fallback_handler.get_current_level().name}"
                        )
                        self.metrics.update_fallback_level(
                            self.fallback_handler.get_current_level().value
                        )
                    
                    self.last_health_check_time = current_time
                
                # Update metrics (every 5 seconds)
                if current_time - self.last_metrics_update_time > metrics_update_interval:
                    # Update cache stats
                    cache_stats = self.cache_manager.get_cache_stats()
                    self.metrics.update_cache_metrics(cache_stats)
                    
                    # Update uptime
                    self.metrics.update_uptime()
                    
                    # Log stats periodically
                    if self.batch_count % 100 == 0 and self.batch_count > 0:
                        self.logger.info(
                            f"Processed {self.requests_processed} requests in {self.batch_count} batches, "
                            f"cache hit rate: {cache_stats.get('total_hit_rate', 0.0):.1%}, "
                            f"avg latency: {self.inference_engine.get_avg_latency_ms():.2f}ms"
                        )
                    
                    self.last_metrics_update_time = current_time
            
            except asyncio.CancelledError:
                self.logger.info("Service loop cancelled")
                break
            
            except Exception as e:
                self.logger.error(f"Service loop error: {e}", exc_info=True)
                await asyncio.sleep(1.0)  # Back off on error
    
    async def shutdown(self) -> None:
        """Graceful shutdown"""
        self.logger.info("Shutting down ML Inference Service...")
        self.running = False
        
        # Print final statistics
        self.logger.info("="*80)
        self.logger.info("Final Statistics:")
        self.logger.info(f"  Total Requests Processed: {self.requests_processed}")
        self.logger.info(f"  Total Batches: {self.batch_count}")
        self.logger.info(f"  Avg Latency: {self.inference_engine.get_avg_latency_ms():.2f}ms")
        
        cache_stats = self.cache_manager.get_cache_stats()
        self.logger.info(f"  Cache Hit Rate: {cache_stats.get('total_hit_rate', 0.0):.1%}")
        self.logger.info(f"    L2 Hits: {cache_stats.get('l2_hits', 0)}")
        self.logger.info(f"    L3 Hits: {cache_stats.get('l3_hits', 0)}")
        
        fallback_stats = self.fallback_handler.get_statistics()
        self.logger.info(f"  Fallback Level: {fallback_stats['current_level_name']}")
        
        # Graph statistics
        if self.graph_enabled and self.graph_manager:
            try:
                graph_stats = await self.graph_manager.get_graph_statistics()
                graph_perf = self.graph_manager.get_performance_metrics()
                self.logger.info(f"  Graph Queries: {graph_perf['query_count']}")
                self.logger.info(f"  Graph Avg Query Time: {graph_perf['avg_query_time_ms']:.2f}ms")
                self.logger.info(f"  Graph Errors: {graph_perf['error_count']}")
            except Exception as e:
                self.logger.warning(f"Could not retrieve graph statistics: {e}")
        
        self.logger.info("="*80)
        
        # Close connections
        await self.request_processor.close()
        self.cache_manager.close()
        self.inference_engine.unload_all_models()
        
        # Shut down HTTP server
        await self.stop_http_server()
        
        # Shut down metrics
        self.metrics.shutdown()
        
        self.logger.info("✓ Shutdown complete")
    
    async def start_http_server(self):
        """Start HTTP server for hot reload and metrics"""
        self.http_app = web.Application()
        
        # Add routes
        self.http_app.router.add_post('/reload/{archetype}/{model_type}', self.handle_reload_request)
        self.http_app.router.add_get('/health', self.handle_health_check)
        self.http_app.router.add_get('/models', self.handle_list_models)
        self.http_app.router.add_get('/stats', self.handle_stats)
        
        # Setup and start
        self.http_runner = web.AppRunner(self.http_app)
        await self.http_runner.setup()
        
        site = web.TCPSite(self.http_runner, 'localhost', self.http_port)
        await site.start()
        
        self.logger.info(f"✓ HTTP server started on port {self.http_port}")
    
    async def stop_http_server(self):
        """Stop HTTP server"""
        if self.http_runner:
            await self.http_runner.cleanup()
            self.logger.info("✓ HTTP server stopped")
    
    async def handle_reload_request(self, request: web.Request) -> web.Response:
        """
        Handle model reload HTTP request
        
        POST /reload/{archetype}/{model_type}
        
        Args:
            request: aiohttp request
        
        Returns:
            JSON response
        """
        archetype = request.match_info['archetype']
        model_type = request.match_info['model_type']
        
        self.logger.info(f"Reload request received: {archetype}/{model_type}")
        
        # Optional: get custom model path from request body
        model_path = None
        if request.content_type == 'application/json':
            try:
                data = await request.json()
                model_path = data.get('model_path')
            except Exception:
                pass
        
        # Reload model
        try:
            success = await self.inference_engine.reload_single_model(
                archetype, model_type, model_path
            )
            
            if success:
                return web.json_response({
                    'success': True,
                    'message': f'Model {archetype}/{model_type} reloaded successfully',
                    'archetype': archetype,
                    'model_type': model_type,
                    'timestamp': time.time()
                })
            else:
                return web.json_response({
                    'success': False,
                    'message': f'Failed to reload model {archetype}/{model_type}',
                    'archetype': archetype,
                    'model_type': model_type
                }, status=500)
        
        except Exception as e:
            self.logger.error(f"Reload request failed: {e}", exc_info=True)
            return web.json_response({
                'success': False,
                'message': f'Exception during reload: {str(e)}',
                'archetype': archetype,
                'model_type': model_type
            }, status=500)
    
    async def handle_health_check(self, request: web.Request) -> web.Response:
        """
        Handle health check request
        
        GET /health
        
        Returns:
            JSON health status
        """
        health = self.inference_engine.health_check()
        vram = self.inference_engine.get_vram_usage()
        
        response_data = {
            'status': health['status'],
            'message': health['message'],
            'models_loaded': health['models_loaded'],
            'device_available': health['device_available'],
            'avg_latency_ms': health['avg_latency_ms'],
            'inference_count': health['inference_count'],
            'vram': vram,
            'requests_processed': self.requests_processed,
            'batch_count': self.batch_count,
            'fallback_level': self.fallback_handler.get_current_level().value,
            'uptime_seconds': time.time() - (self.last_cleanup_time - 60)  # Approximate
        }
        
        status_code = 200 if health['status'] == 'healthy' else 503
        
        return web.json_response(response_data, status=status_code)
    
    async def handle_list_models(self, request: web.Request) -> web.Response:
        """
        Handle list models request
        
        GET /models
        
        Returns:
            JSON list of loaded models with metadata
        """
        models = []
        metadata_all = self.inference_engine.get_all_model_metadata()
        
        for (archetype, model_type), metadata in metadata_all.items():
            models.append({
                'archetype': archetype,
                'model_type': model_type,
                'model_path': metadata.get('model_path', ''),
                'loaded_at': metadata.get('loaded_at').isoformat() if metadata.get('loaded_at') else None,
                'reload_count': metadata.get('reload_count', 0),
                'input_shape': list(metadata.get('input_shape', [])),
                'output_shape': list(metadata.get('output_shape', []))
            })
        
        return web.json_response({
            'models': models,
            'total_count': len(models)
        })
    
    async def handle_stats(self, request: web.Request) -> web.Response:
        """
        Handle statistics request
        
        GET /stats
        
        Returns:
            JSON statistics
        """
        stats = self.inference_engine.get_statistics()
        cache_stats = self.cache_manager.get_cache_stats()
        
        return web.json_response({
            'inference': stats,
            'cache': cache_stats,
            'requests_processed': self.requests_processed,
            'batch_count': self.batch_count
        })


async def main():
    """Entry point"""
    service = None
    
    try:
        # Create service instance
        service = MLInferenceService()
        
        # Setup signal handlers
        loop = asyncio.get_event_loop()
        
        def signal_handler(sig):
            logging.info(f"Received signal {sig}, initiating shutdown...")
            if service:
                loop.create_task(service.shutdown())
                loop.stop()
        
        for sig in (signal.SIGINT, signal.SIGTERM):
            loop.add_signal_handler(sig, lambda s=sig: signal_handler(s))
        
        # Initialize
        await service.initialize()
        
        # Run service
        await service.run_loop()
    
    except KeyboardInterrupt:
        logging.info("KeyboardInterrupt received")
    
    except Exception as e:
        logging.critical(f"Fatal error: {e}", exc_info=True)
        sys.exit(1)
    
    finally:
        if service:
            await service.shutdown()


if __name__ == '__main__':
    asyncio.run(main())
