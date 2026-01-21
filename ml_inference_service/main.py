"""
ML Inference Service - Main Entry Point
Orchestrates polling, caching, inference, and response writing
"""

import asyncio
import logging
import signal
import sys
from typing import List, Dict, Any
import numpy as np
import time
from pathlib import Path

from config import load_config, save_default_config
from logger import setup_logging, get_logger
from inference_engine import ONNXInferenceEngine
from request_processor import RequestProcessor
from cache_manager import CacheManager
from fallback_handler import FallbackHandler, FallbackLevel
from health_monitor import HealthMonitor
from metrics import MetricsCollector


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
        
        # Service state
        self.running = False
        self.requests_processed = 0
        self.batch_count = 0
        
        # Performance tracking
        self.last_cleanup_time = time.time()
        self.last_health_check_time = time.time()
        self.last_metrics_update_time = time.time()
    
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
        
        self.logger.info("✓ ML Inference Service initialized successfully")
        self.logger.info(f"  Models loaded: {loaded}/{total}")
        self.logger.info(f"  Fallback level: {self.fallback_handler.get_current_level().name}")
        self.logger.info(f"  Cache enabled: {self.config['caching']['enabled']}")
        self.logger.info(f"  Batch size: {self.config['inference']['batch_size']}")
        self.logger.info(f"  Prometheus port: {self.config['monitoring']['prometheus_port']}")
    
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
        
        batch_time = (time.perf_counter() - batch_start) * 1000  # ms
        
        self.logger.debug(
            f"Processed batch of {len(requests)} "
            f"(cache hits: {len(cache_hits)}, misses: {len(cache_misses)}) "
            f"in {batch_time:.1f}ms"
        )
        
        return responses
    
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
                    # Process batch
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
        self.logger.info("="*80)
        
        # Close connections
        await self.request_processor.close()
        self.cache_manager.close()
        self.inference_engine.unload_all_models()
        
        # Shut down metrics
        self.metrics.shutdown()
        
        self.logger.info("✓ Shutdown complete")


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
