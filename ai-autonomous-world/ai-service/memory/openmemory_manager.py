"""
OpenMemory SDK Manager for persistent memory storage

This module provides a wrapper around the OpenMemory SDK for managing long-term
persistent memory storage in PostgreSQL with multi-sector memory classification.

Features:
- Long-term memory storage in PostgreSQL
- Multi-sector memory (episodic, semantic, procedural, emotional, reflective)
- Vector similarity search for memory retrieval
- Automatic memory decay and reinforcement
- User isolation for multi-user support
"""

from typing import Optional, Dict, List, Any
from loguru import logger

try:
    from openmemory import OpenMemory
    OPENMEMORY_AVAILABLE = True
    # Brain sector constants
    SECTORS = {
        'EPISODIC': 'episodic',      # Event memories
        'SEMANTIC': 'semantic',      # Facts & preferences
        'PROCEDURAL': 'procedural',  # Habits, triggers
        'EMOTIONAL': 'emotional',    # Sentiment states
        'REFLECTIVE': 'reflective'   # Meta memory & logs
    }
except ImportError:
    logger.warning("OpenMemory SDK not available - install with: pip install openmemory-py")
    OPENMEMORY_AVAILABLE = False
    OpenMemory = None
    SECTORS = None


class OpenMemoryManager:
    """
    Manager for OpenMemory SDK integration with PostgreSQL
    
    Handles:
    - OpenMemory SDK initialization with backend service
    - Connection management
    - Memory operations delegation
    """
    
    def __init__(self, base_url: str = "http://localhost:8080", api_key: str = ""):
        """
        Initialize OpenMemory Manager
        
        Args:
            base_url: OpenMemory backend service URL (default: http://localhost:8080)
            api_key: Optional API key for authentication
        
        Raises:
            ImportError: If OpenMemory SDK is not available
        """
        if not OPENMEMORY_AVAILABLE:
            raise ImportError(
                "OpenMemory SDK is not available. "
                "Check memory/openmemory_client.py"
            )
        
        self.base_url = base_url
        self.api_key = api_key
        self.client: Optional[OpenMemory] = None
        self._initialized = False
    
    def initialize(self):
        """
        Initialize OpenMemory SDK client
        
        Raises:
            RuntimeError: If initialization fails
        """
        try:
            logger.info(f"Initializing OpenMemory SDK client (backend: {self.base_url})")
            
            # Create OpenMemory client
            self.client = OpenMemory(
                base_url=self.base_url,
                api_key=self.api_key
            )
            
            # Test connection by checking health
            health = self.client.get_health()
            logger.info(f"✓ OpenMemory backend health check: {health}")
            
            self._initialized = True
            logger.info("✓ OpenMemory SDK initialized successfully")
            logger.info(f"✓ OpenMemory backend URL: {self.base_url}")
            logger.info(f"✓ OpenMemory is_available: {self.is_available()}")
            
        except Exception as e:
            error_msg = f"Failed to initialize OpenMemory SDK: {e}"
            logger.error(error_msg, exc_info=True)
            logger.warning("⚠ OpenMemory SDK not available - memory persistence disabled")
            # Don't raise - allow graceful degradation
            self._initialized = False
    
    def get_client(self) -> OpenMemory:
        """
        Get OpenMemory client instance
        
        Returns:
            OpenMemory client instance
        
        Raises:
            RuntimeError: If OpenMemory is not initialized
        """
        if not self._initialized or self.client is None:
            raise RuntimeError(
                "OpenMemory client is not initialized. Call initialize() first."
            )
        return self.client
    
    def is_available(self) -> bool:
        """Check if OpenMemory SDK is initialized and ready"""
        return self._initialized and self.client is not None
    
    def shutdown(self):
        """Shutdown OpenMemory client"""
        if self.client:
            try:
                logger.info("OpenMemory SDK shutdown complete")
                self._initialized = False
            except Exception as e:
                logger.error(f"Error during OpenMemory shutdown: {e}")


# Global OpenMemory manager instance (initialized in main.py)
openmemory_manager: Optional[OpenMemoryManager] = None


def get_openmemory_manager() -> Optional[OpenMemoryManager]:
    """Get global OpenMemory manager instance"""
    return openmemory_manager


def initialize_openmemory(base_url: str = "http://localhost:8080", api_key: str = "") -> OpenMemoryManager:
    """
    Initialize global OpenMemory manager
    
    Args:
        base_url: OpenMemory backend service URL
        api_key: Optional API key for authentication
    
    Returns:
        OpenMemoryManager instance
    
    Raises:
        ImportError: If OpenMemory SDK is not available
    """
    global openmemory_manager
    
    logger.info("Initializing global OpenMemory manager")
    openmemory_manager = OpenMemoryManager(base_url, api_key)
    openmemory_manager.initialize()  # Will log warning if fails but won't raise
    
    if openmemory_manager.is_available():
        logger.info("✓ Global OpenMemory manager initialized successfully")
    else:
        logger.warning("⚠ Global OpenMemory manager initialization failed - memory persistence disabled")
    
    return openmemory_manager

