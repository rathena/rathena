"""
Custom Exception Classes for P2P Coordinator Service

Provides specific exception types for better error handling and debugging.
"""

from typing import Optional


class P2PCoordinatorException(Exception):
    """Base exception for P2P Coordinator Service"""
    def __init__(self, message: str, details: Optional[dict] = None):
        self.message = message
        self.details = details or {}
        super().__init__(self.message)


class DatabaseException(P2PCoordinatorException):
    """Database operation failed"""
    pass


class HostNotFoundException(P2PCoordinatorException):
    """Host not found in database"""
    pass


class HostRegistrationException(P2PCoordinatorException):
    """Failed to register host"""
    pass


class ZoneNotFoundException(P2PCoordinatorException):
    """Zone not found in database"""
    pass


class ZoneConfigurationException(P2PCoordinatorException):
    """Zone configuration error"""
    pass


class SessionNotFoundException(P2PCoordinatorException):
    """Session not found in database"""
    pass


class SessionCreationException(P2PCoordinatorException):
    """Failed to create session"""
    pass


class SessionStateException(P2PCoordinatorException):
    """Invalid session state transition"""
    pass


class SignalingException(P2PCoordinatorException):
    """WebRTC signaling error"""
    pass


class AuthenticationException(P2PCoordinatorException):
    """Authentication failed"""
    pass


class ValidationException(P2PCoordinatorException):
    """Input validation failed"""
    pass


class RateLimitException(P2PCoordinatorException):
    """Rate limit exceeded"""
    pass


class AIServiceException(P2PCoordinatorException):
    """AI service integration error"""
    pass


class RedisException(P2PCoordinatorException):
    """Redis/DragonflyDB operation failed"""
    pass

