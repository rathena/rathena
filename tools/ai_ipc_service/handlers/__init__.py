"""
AI IPC Service Handlers Package

This package contains request handlers for different types of AI requests.
Each handler implements the BaseHandler interface and processes specific
request types from the ai_requests database table.

Available Handlers:
    - HealthCheckHandler: Simple health check responses
    - HttpProxyHandler: HTTP GET/POST proxy to external services
    - AIDialogueHandler: AI-powered NPC dialogue generation

Handler Registration:
    Handlers are registered by request_type in the processor module.
    The request_type field in ai_requests determines which handler processes
    each request.

Request Types -> Handlers:
    - 'health_check', 'health' -> HealthCheckHandler
    - 'http_get', 'http_post', 'httpget', 'httppost' -> HttpProxyHandler
    - 'dialogue', 'ai_dialogue', 'npc_dialogue' -> AIDialogueHandler
"""

from .base import BaseHandler
from .health_check import HealthCheckHandler
from .http_proxy import HttpProxyHandler
from .ai_dialogue import AIDialogueHandler

__all__ = [
    "BaseHandler",
    "HealthCheckHandler",
    "HttpProxyHandler",
    "AIDialogueHandler",
]

# Handler registry mapping request_type to handler class
HANDLER_REGISTRY: dict[str, type[BaseHandler]] = {
    # Health check handlers
    "health_check": HealthCheckHandler,
    "health": HealthCheckHandler,
    
    # HTTP proxy handlers (backward compatibility with httpget/httppost)
    "http_get": HttpProxyHandler,
    "http_post": HttpProxyHandler,
    "httpget": HttpProxyHandler,
    "httppost": HttpProxyHandler,
    "get": HttpProxyHandler,
    "post": HttpProxyHandler,
    
    # AI dialogue handlers
    "dialogue": AIDialogueHandler,
    "ai_dialogue": AIDialogueHandler,
    "npc_dialogue": AIDialogueHandler,
}


def get_handler(request_type: str, config) -> BaseHandler | None:
    """
    Get a handler instance for the given request type.
    
    Args:
        request_type: The type of request to handle
        config: Configuration object to pass to the handler
        
    Returns:
        Handler instance or None if no handler registered for type
    """
    handler_class = HANDLER_REGISTRY.get(request_type.lower())
    if handler_class:
        return handler_class(config)
    return None