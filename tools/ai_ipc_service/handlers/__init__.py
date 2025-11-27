"""
AI IPC Service Handlers Package

This package contains request handlers for different types of AI requests.
Each handler implements the BaseHandler interface and processes specific
request types from the ai_requests database table.

Available Handlers:
    - HealthCheckHandler: Simple health check responses
    - HttpProxyHandler: HTTP GET/POST proxy to external services
    - AIDialogueHandler: AI-powered NPC dialogue generation
    - AIDecisionHandler: AI-powered NPC decision making
    - AIEmotionHandler: AI-powered NPC emotion processing
    - AIAsyncRequestHandler: Async AI request submission
    - AICheckResultHandler: Async AI result checking

Handler Registration:
    Handlers are registered by request_type in the processor module.
    The request_type field in ai_requests determines which handler processes
    each request.

Request Types -> Handlers:
    - 'health_check', 'health' -> HealthCheckHandler
    - 'http_get', 'http_post', 'httpget', 'httppost' -> HttpProxyHandler
    - 'dialogue', 'ai_dialogue', 'npc_dialogue', 'ai_npc_dialogue' -> AIDialogueHandler
    - 'decision', 'ai_decision', 'npc_decision', 'ai_npc_decide_action' -> AIDecisionHandler
    - 'emotion', 'ai_emotion', 'npc_emotion', 'ai_npc_get_emotion' -> AIEmotionHandler
    - 'ai_request_async', 'async_request' -> AIAsyncRequestHandler
    - 'ai_check_result', 'check_result' -> AICheckResultHandler
"""

from .base import BaseHandler
from .health_check import HealthCheckHandler
from .http_proxy import HttpProxyHandler
from .ai_dialogue import AIDialogueHandler
from .ai_decision import AIDecisionHandler
from .ai_emotion import AIEmotionHandler
from .ai_async import AIAsyncRequestHandler, AICheckResultHandler

__all__ = [
    "BaseHandler",
    "HealthCheckHandler",
    "HttpProxyHandler",
    "AIDialogueHandler",
    "AIDecisionHandler",
    "AIEmotionHandler",
    "AIAsyncRequestHandler",
    "AICheckResultHandler",
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
    
    # AI dialogue handlers (matches C++ ai_npc_dialogue command)
    "dialogue": AIDialogueHandler,
    "ai_dialogue": AIDialogueHandler,
    "npc_dialogue": AIDialogueHandler,
    "ai_npc_dialogue": AIDialogueHandler,
    
    # AI decision handlers (matches C++ ai_npc_decide_action command)
    "decision": AIDecisionHandler,
    "ai_decision": AIDecisionHandler,
    "npc_decision": AIDecisionHandler,
    "ai_npc_decide_action": AIDecisionHandler,
    
    # AI emotion handlers (matches C++ ai_npc_get_emotion command)
    "emotion": AIEmotionHandler,
    "ai_emotion": AIEmotionHandler,
    "npc_emotion": AIEmotionHandler,
    "ai_npc_get_emotion": AIEmotionHandler,
    
    # Async request handlers (matches C++ ai_request_async command)
    "ai_request_async": AIAsyncRequestHandler,
    "async_request": AIAsyncRequestHandler,
    
    # Result checking handlers (matches C++ ai_check_result command)
    "ai_check_result": AICheckResultHandler,
    "check_result": AICheckResultHandler,
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