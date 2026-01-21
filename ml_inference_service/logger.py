"""
Structured Logging Configuration for ML Inference Service
Provides JSON and text logging with rotation
"""

import logging
import logging.handlers
import json
import sys
from pathlib import Path
from datetime import datetime
from typing import Dict, Any, Optional


class JSONFormatter(logging.Formatter):
    """Format log records as JSON for structured logging"""
    
    def format(self, record: logging.LogRecord) -> str:
        """
        Format log record as JSON
        
        Args:
            record: Log record to format
        
        Returns:
            JSON-formatted log string
        """
        log_obj = {
            'timestamp': datetime.utcfromtimestamp(record.created).isoformat() + 'Z',
            'level': record.levelname,
            'logger': record.name,
            'message': record.getMessage(),
            'module': record.module,
            'function': record.funcName,
            'line': record.lineno
        }
        
        # Add exception info if present
        if record.exc_info:
            log_obj['exception'] = self.formatException(record.exc_info)
        
        # Add extra fields if present
        if hasattr(record, 'extra_fields'):
            log_obj.update(record.extra_fields)
        
        return json.dumps(log_obj)


class TextFormatter(logging.Formatter):
    """Format log records as readable text"""
    
    def __init__(self):
        super().__init__(
            fmt='%(asctime)s [%(levelname)8s] %(name)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )


def setup_logging(config: Dict[str, Any]) -> None:
    """
    Setup logging configuration
    
    Args:
        config: Logging configuration dict with keys:
            - level: Log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
            - format: 'json' or 'text'
            - file: Path to log file (optional)
            - max_bytes: Max log file size before rotation (default: 100MB)
            - backup_count: Number of backup files to keep (default: 10)
    """
    # Get log level
    log_level_str = config.get('level', 'INFO')
    log_level = getattr(logging, log_level_str.upper(), logging.INFO)
    
    # Get log format
    log_format = config.get('format', 'text')
    
    # Create formatter
    if log_format == 'json':
        formatter = JSONFormatter()
    else:
        formatter = TextFormatter()
    
    # Get root logger
    root_logger = logging.getLogger()
    root_logger.setLevel(log_level)
    
    # Remove existing handlers
    for handler in root_logger.handlers[:]:
        root_logger.removeHandler(handler)
    
    # Console handler (always add)
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(log_level)
    console_handler.setFormatter(formatter)
    root_logger.addHandler(console_handler)
    
    # File handler (if configured)
    log_file = config.get('file')
    if log_file:
        log_path = Path(log_file)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        
        max_bytes = config.get('max_bytes', 100 * 1024 * 1024)  # 100MB default
        backup_count = config.get('backup_count', 10)
        
        file_handler = logging.handlers.RotatingFileHandler(
            log_file,
            maxBytes=max_bytes,
            backupCount=backup_count,
            encoding='utf-8'
        )
        file_handler.setLevel(log_level)
        file_handler.setFormatter(formatter)
        root_logger.addHandler(file_handler)
        
        logging.info(f"File logging enabled: {log_file}")
    
    # Set library log levels (reduce noise)
    logging.getLogger('asyncpg').setLevel(logging.WARNING)
    logging.getLogger('redis').setLevel(logging.WARNING)
    logging.getLogger('onnxruntime').setLevel(logging.WARNING)
    
    logging.info(f"Logging initialized: level={log_level_str}, format={log_format}")


class ServiceLogger:
    """
    Enhanced logger with structured fields
    Provides methods for common logging patterns
    """
    
    def __init__(self, name: str):
        """
        Initialize service logger
        
        Args:
            name: Logger name (usually module name)
        """
        self.logger = logging.getLogger(name)
    
    def log_with_fields(self, level: int, message: str, **fields) -> None:
        """
        Log message with additional structured fields
        
        Args:
            level: Log level (logging.INFO, etc.)
            message: Log message
            **fields: Additional fields to include in log
        """
        record = self.logger.makeRecord(
            self.logger.name,
            level,
            "(unknown file)",
            0,
            message,
            (),
            None
        )
        record.extra_fields = fields
        self.logger.handle(record)
    
    def debug(self, message: str, **fields) -> None:
        """Debug level log"""
        self.log_with_fields(logging.DEBUG, message, **fields)
    
    def info(self, message: str, **fields) -> None:
        """Info level log"""
        self.log_with_fields(logging.INFO, message, **fields)
    
    def warning(self, message: str, **fields) -> None:
        """Warning level log"""
        self.log_with_fields(logging.WARNING, message, **fields)
    
    def error(self, message: str, **fields) -> None:
        """Error level log"""
        self.log_with_fields(logging.ERROR, message, **fields)
    
    def critical(self, message: str, **fields) -> None:
        """Critical level log"""
        self.log_with_fields(logging.CRITICAL, message, **fields)
    
    def log_inference(self, monster_id: int, archetype: str, action: int, 
                     latency_ms: float, from_cache: bool) -> None:
        """
        Log inference event with structured data
        
        Args:
            monster_id: Monster instance ID
            archetype: Monster archetype
            action: Action ID returned
            latency_ms: Inference latency in milliseconds
            from_cache: Whether result came from cache
        """
        self.info(
            f"Inference complete: monster={monster_id}, action={action}, latency={latency_ms:.2f}ms",
            monster_id=monster_id,
            archetype=archetype,
            action=action,
            latency_ms=latency_ms,
            from_cache=from_cache,
            event_type='inference'
        )
    
    def log_batch_processing(self, batch_size: int, cache_hits: int, 
                            cache_misses: int, total_latency_ms: float) -> None:
        """
        Log batch processing event
        
        Args:
            batch_size: Number of requests in batch
            cache_hits: Number of cache hits
            cache_misses: Number of cache misses
            total_latency_ms: Total batch processing time
        """
        self.info(
            f"Batch processed: size={batch_size}, cache_hits={cache_hits}, "
            f"latency={total_latency_ms:.2f}ms",
            batch_size=batch_size,
            cache_hits=cache_hits,
            cache_misses=cache_misses,
            total_latency_ms=total_latency_ms,
            avg_latency_ms=total_latency_ms / max(batch_size, 1),
            cache_hit_rate=cache_hits / max(batch_size, 1),
            event_type='batch_processing'
        )
    
    def log_fallback(self, from_level: int, to_level: int, reason: str) -> None:
        """
        Log fallback level change
        
        Args:
            from_level: Previous fallback level
            to_level: New fallback level
            reason: Reason for fallback
        """
        self.warning(
            f"Fallback level changed: {from_level} → {to_level}, reason={reason}",
            from_level=from_level,
            to_level=to_level,
            reason=reason,
            event_type='fallback'
        )
    
    def log_health_check(self, status: str, details: Dict[str, Any]) -> None:
        """
        Log health check result
        
        Args:
            status: Health status ('healthy', 'degraded', 'critical')
            details: Health check details
        """
        log_method = self.info if status == 'healthy' else self.warning
        log_method(
            f"Health check: {status}",
            status=status,
            event_type='health_check',
            **details
        )
    
    def log_model_load(self, archetype: str, model_type: str, 
                      success: bool, size_mb: Optional[float] = None,
                      error: Optional[str] = None) -> None:
        """
        Log model loading event
        
        Args:
            archetype: Model archetype
            model_type: Model type
            success: Whether load succeeded
            size_mb: Model size in MB (if successful)
            error: Error message (if failed)
        """
        if success:
            self.info(
                f"Model loaded: {archetype}/{model_type} ({size_mb:.1f} MB)",
                archetype=archetype,
                model_type=model_type,
                size_mb=size_mb,
                success=True,
                event_type='model_load'
            )
        else:
            self.error(
                f"Model load failed: {archetype}/{model_type} - {error}",
                archetype=archetype,
                model_type=model_type,
                success=False,
                error=error,
                event_type='model_load'
            )


# Global logger instance
_global_logger: Optional[ServiceLogger] = None


def get_logger(name: str) -> ServiceLogger:
    """
    Get service logger instance
    
    Args:
        name: Logger name
    
    Returns:
        ServiceLogger instance
    """
    return ServiceLogger(name)


def init_global_logger(name: str = 'ml_inference') -> ServiceLogger:
    """
    Initialize global logger instance
    
    Args:
        name: Logger name
    
    Returns:
        Global ServiceLogger instance
    """
    global _global_logger
    _global_logger = ServiceLogger(name)
    return _global_logger


def get_global_logger() -> ServiceLogger:
    """
    Get global logger instance
    
    Returns:
        Global ServiceLogger instance
    
    Raises:
        RuntimeError: If global logger not initialized
    """
    if _global_logger is None:
        raise RuntimeError("Global logger not initialized. Call init_global_logger() first.")
    return _global_logger
