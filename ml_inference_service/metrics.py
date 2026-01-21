"""
Prometheus Metrics Collector for ML Inference Service
Exposes metrics on HTTP endpoint for Prometheus scraping
"""

import logging
from typing import Dict, Any, Optional
from prometheus_client import Counter, Histogram, Gauge, Info, start_http_server, REGISTRY
import time


class MetricsCollector:
    """
    Collects and exposes Prometheus metrics for ML inference service
    """
    
    def __init__(self, prometheus_port: int = 9090):
        """
        Initialize metrics collector
        
        Args:
            prometheus_port: Port for Prometheus HTTP server
        """
        self.logger = logging.getLogger(__name__)
        self.prometheus_port = prometheus_port
        self.server_started = False
        
        # Define metrics
        
        # Inference metrics
        self.inference_requests_total = Counter(
            'ml_inference_requests_total',
            'Total inference requests processed',
            ['archetype', 'model_type', 'status']
        )
        
        self.inference_latency = Histogram(
            'ml_inference_latency_seconds',
            'Model inference latency in seconds',
            ['archetype', 'model_type', 'cache_status'],
            buckets=[0.001, 0.005, 0.010, 0.015, 0.020, 0.030, 0.050, 0.100, 0.200, 0.500]
        )
        
        self.batch_size = Histogram(
            'ml_batch_size',
            'Inference batch size distribution',
            buckets=[1, 4, 8, 16, 32, 64, 128, 256]
        )
        
        # Cache metrics
        self.cache_hits_total = Counter(
            'ml_cache_hits_total',
            'Total cache hits',
            ['cache_layer']
        )
        
        self.cache_misses_total = Counter(
            'ml_cache_misses_total',
            'Total cache misses'
        )
        
        self.cache_hit_rate = Gauge(
            'ml_cache_hit_rate',
            'Current cache hit rate',
            ['cache_layer']
        )
        
        # Fallback metrics
        self.fallback_level = Gauge(
            'ml_fallback_level',
            'Current fallback level (1-5)'
        )
        
        self.fallback_transitions = Counter(
            'ml_fallback_transitions_total',
            'Total fallback level transitions',
            ['from_level', 'to_level']
        )
        
        # Resource metrics
        self.vram_usage_bytes = Gauge(
            'ml_vram_usage_bytes',
            'GPU VRAM usage in bytes',
            ['device_id']
        )
        
        self.ram_usage_bytes = Gauge(
            'ml_ram_usage_bytes',
            'System RAM usage in bytes'
        )
        
        self.cpu_usage_percent = Gauge(
            'ml_cpu_usage_percent',
            'CPU usage percentage'
        )
        
        # Model metrics
        self.models_loaded = Gauge(
            'ml_models_loaded_total',
            'Number of models loaded',
            ['archetype', 'model_type']
        )
        
        self.model_inference_errors = Counter(
            'ml_model_inference_errors_total',
            'Model inference errors',
            ['archetype', 'model_type', 'error_type']
        )
        
        # Database metrics
        self.db_queries_total = Counter(
            'ml_db_queries_total',
            'Total database queries',
            ['operation', 'status']
        )
        
        self.db_query_latency = Histogram(
            'ml_db_query_latency_seconds',
            'Database query latency',
            ['operation'],
            buckets=[0.001, 0.002, 0.005, 0.010, 0.020, 0.050, 0.100]
        )
        
        self.db_pool_connections = Gauge(
            'ml_db_pool_connections',
            'Database pool connection count',
            ['pool_name', 'status']
        )
        
        # Service metrics
        self.service_up = Gauge(
            'ml_service_up',
            'Service operational status (1=up, 0=down)'
        )
        
        self.service_info = Info(
            'ml_service',
            'ML inference service information'
        )
        
        self.uptime_seconds = Gauge(
            'ml_uptime_seconds',
            'Service uptime in seconds'
        )
        
        # Set service as up
        self.service_up.set(1)
        self.start_time = time.time()
    
    def start_server(self) -> None:
        """Start Prometheus HTTP server"""
        if self.server_started:
            self.logger.warning("Prometheus server already started")
            return
        
        try:
            start_http_server(self.prometheus_port)
            self.server_started = True
            self.logger.info(f"Prometheus metrics server started on port {self.prometheus_port}")
        except Exception as e:
            self.logger.error(f"Failed to start Prometheus server: {e}")
    
    def record_inference(self, archetype: str, model_type: str, 
                        latency_seconds: float, cache_status: str = 'miss',
                        status: str = 'success') -> None:
        """
        Record inference metrics
        
        Args:
            archetype: Monster archetype
            model_type: Model type used
            latency_seconds: Inference latency in seconds
            cache_status: 'hit', 'miss', or 'l1_hit', 'l2_hit', 'l3_hit'
            status: 'success' or 'error'
        """
        self.inference_requests_total.labels(
            archetype=archetype,
            model_type=model_type,
            status=status
        ).inc()
        
        self.inference_latency.labels(
            archetype=archetype,
            model_type=model_type,
            cache_status=cache_status
        ).observe(latency_seconds)
    
    def record_batch(self, batch_size: int, total_latency_seconds: float) -> None:
        """
        Record batch processing metrics
        
        Args:
            batch_size: Number of requests in batch
            total_latency_seconds: Total batch processing time
        """
        self.batch_size.observe(batch_size)
    
    def update_cache_metrics(self, cache_stats: Dict[str, Any]) -> None:
        """
        Update cache metrics from cache manager stats
        
        Args:
            cache_stats: Cache statistics dictionary
        """
        # Update hit rates
        self.cache_hit_rate.labels(cache_layer='l2').set(
            cache_stats.get('l2_hit_rate', 0.0)
        )
        self.cache_hit_rate.labels(cache_layer='l3').set(
            cache_stats.get('l3_hit_rate', 0.0)
        )
        self.cache_hit_rate.labels(cache_layer='total').set(
            cache_stats.get('total_hit_rate', 0.0)
        )
    
    def update_fallback_level(self, level: int, previous_level: Optional[int] = None) -> None:
        """
        Update fallback level metrics
        
        Args:
            level: Current fallback level (1-5)
            previous_level: Previous level (for transition tracking)
        """
        self.fallback_level.set(level)
        
        if previous_level is not None and previous_level != level:
            self.fallback_transitions.labels(
                from_level=str(previous_level),
                to_level=str(level)
            ).inc()
    
    def update_resource_metrics(self, health_status: Dict[str, Any]) -> None:
        """
        Update resource usage metrics from health check
        
        Args:
            health_status: Health status from HealthMonitor
        """
        # GPU/VRAM
        if 'gpu' in health_status:
            gpu = health_status['gpu']
            if gpu.get('available'):
                self.vram_usage_bytes.labels(
                    device_id=gpu.get('device_id', 0)
                ).set(gpu.get('allocated_bytes', 0))
        
        # RAM
        if 'ram' in health_status:
            ram = health_status['ram']
            if 'used_gb' in ram:
                self.ram_usage_bytes.set(ram['used_gb'] * (1024**3))
        
        # CPU
        if 'cpu' in health_status:
            cpu = health_status['cpu']
            if 'usage_percent' in cpu:
                self.cpu_usage_percent.set(cpu['usage_percent'] * 100)
    
    def update_model_metrics(self, loaded_models: list) -> None:
        """
        Update loaded model count metrics
        
        Args:
            loaded_models: List of (archetype, model_type) tuples
        """
        # Reset all to 0
        for archetype in ['aggressive', 'defensive', 'support', 'mage', 'tank', 'ranged']:
            for model_type in ['combat_dqn', 'movement_ppo', 'skill_dqn', 'threat_assessment',
                              'team_coordination', 'spatial_vit', 'temporal_transformer',
                              'pattern_recognition', 'soft_actor_critic']:
                self.models_loaded.labels(
                    archetype=archetype,
                    model_type=model_type
                ).set(0)
        
        # Set loaded models to 1
        for archetype, model_type in loaded_models:
            self.models_loaded.labels(
                archetype=archetype,
                model_type=model_type
            ).set(1)
    
    def record_model_error(self, archetype: str, model_type: str, error_type: str) -> None:
        """
        Record model inference error
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            error_type: Type of error (e.g., 'oom', 'timeout', 'runtime')
        """
        self.model_inference_errors.labels(
            archetype=archetype,
            model_type=model_type,
            error_type=error_type
        ).inc()
    
    def record_db_query(self, operation: str, latency_seconds: float, status: str = 'success') -> None:
        """
        Record database query metrics
        
        Args:
            operation: Query operation ('poll', 'write', 'cleanup')
            latency_seconds: Query latency in seconds
            status: 'success' or 'error'
        """
        self.db_queries_total.labels(
            operation=operation,
            status=status
        ).inc()
        
        self.db_query_latency.labels(operation=operation).observe(latency_seconds)
    
    def update_db_pool_metrics(self, pool_name: str, active: int, idle: int) -> None:
        """
        Update database pool metrics
        
        Args:
            pool_name: Pool name ('postgresql', 'redis')
            active: Number of active connections
            idle: Number of idle connections
        """
        self.db_pool_connections.labels(pool_name=pool_name, status='active').set(active)
        self.db_pool_connections.labels(pool_name=pool_name, status='idle').set(idle)
    
    def update_uptime(self) -> None:
        """Update service uptime metric"""
        uptime = time.time() - self.start_time
        self.uptime_seconds.set(uptime)
    
    def set_service_info(self, version: str, device: str, precision: str) -> None:
        """
        Set service information
        
        Args:
            version: Service version
            device: Device being used
            precision: Model precision
        """
        self.service_info.info({
            'version': version,
            'device': device,
            'precision': precision
        })
    
    def shutdown(self) -> None:
        """Shutdown metrics collection"""
        self.service_up.set(0)
        self.logger.info("Metrics collector shut down")
