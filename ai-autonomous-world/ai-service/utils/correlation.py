"""
Correlation ID Management

Request correlation ID generation and context propagation for distributed tracing.

Features:
- UUID-based correlation ID generation
- Async context propagation
- Logging injection
- Parent-child trace relationships
- Thread-safe operation
"""

import logging
import uuid
from typing import Optional
from contextvars import ContextVar
from functools import wraps

logger = logging.getLogger(__name__)

# Context variable for storing correlation ID
_correlation_id: ContextVar[Optional[str]] = ContextVar('correlation_id', default=None)
_parent_id: ContextVar[Optional[str]] = ContextVar('parent_id', default=None)


def generate_correlation_id() -> str:
    """
    Generate a new correlation ID.
    
    Returns:
        str: UUID-based correlation ID
    """
    return f"req-{uuid.uuid4().hex[:16]}"


def get_correlation_id() -> Optional[str]:
    """
    Get current correlation ID from context.
    
    Returns:
        str or None: Current correlation ID
    """
    return _correlation_id.get()


def set_correlation_id(correlation_id: str) -> None:
    """
    Set correlation ID in context.
    
    Args:
        correlation_id: Correlation ID to set
    """
    _correlation_id.set(correlation_id)


def get_parent_id() -> Optional[str]:
    """
    Get parent trace ID from context.
    
    Returns:
        str or None: Parent trace ID
    """
    return _parent_id.get()


def set_parent_id(parent_id: str) -> None:
    """
    Set parent trace ID in context.
    
    Args:
        parent_id: Parent trace ID
    """
    _parent_id.set(parent_id)


def clear_correlation_context() -> None:
    """Clear correlation context."""
    _correlation_id.set(None)
    _parent_id.set(None)


def with_correlation_id(func):
    """
    Decorator to inject correlation ID into async function.
    
    Usage:
        @with_correlation_id
        async def my_function():
            correlation_id = get_correlation_id()
            # Use correlation_id
    """
    @wraps(func)
    async def wrapper(*args, **kwargs):
        # Generate new correlation ID if not present
        if not get_correlation_id():
            correlation_id = generate_correlation_id()
            set_correlation_id(correlation_id)
            logger.debug(f"Generated correlation ID: {correlation_id}")
        
        return await func(*args, **kwargs)
    
    return wrapper


def inject_correlation_logging():
    """
    Inject correlation ID into logging records.
    
    Call this once at application startup to add correlation ID to all logs.
    """
    old_factory = logging.getLogRecordFactory()
    
    def record_factory(*args, **kwargs):
        record = old_factory(*args, **kwargs)
        correlation_id = get_correlation_id()
        parent_id = get_parent_id()
        
        record.correlation_id = correlation_id or 'none'
        record.parent_id = parent_id or 'none'
        
        return record
    
    logging.setLogRecordFactory(record_factory)
    
    # Update logging format to include correlation ID
    for handler in logging.root.handlers:
        if handler.formatter:
            format_str = handler.formatter._fmt
            if 'correlation_id' not in format_str:
                new_format = format_str + ' [%(correlation_id)s]'
                handler.setFormatter(logging.Formatter(new_format))


class CorrelationContext:
    """
    Context manager for correlation ID.
    
    Usage:
        async with CorrelationContext() as correlation_id:
            # correlation_id is available
            await do_something()
    """
    
    def __init__(self, correlation_id: Optional[str] = None, parent_id: Optional[str] = None):
        """
        Initialize correlation context.
        
        Args:
            correlation_id: Correlation ID (generated if None)
            parent_id: Parent trace ID
        """
        self.correlation_id = correlation_id or generate_correlation_id()
        self.parent_id = parent_id
        self.previous_correlation_id = None
        self.previous_parent_id = None
    
    async def __aenter__(self):
        """Enter context."""
        self.previous_correlation_id = get_correlation_id()
        self.previous_parent_id = get_parent_id()
        
        set_correlation_id(self.correlation_id)
        if self.parent_id:
            set_parent_id(self.parent_id)
        
        return self.correlation_id
    
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        """Exit context."""
        if self.previous_correlation_id:
            set_correlation_id(self.previous_correlation_id)
        else:
            _correlation_id.set(None)
        
        if self.previous_parent_id:
            set_parent_id(self.previous_parent_id)
        else:
            _parent_id.set(None)