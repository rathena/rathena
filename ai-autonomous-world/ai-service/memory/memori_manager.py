"""
Memori SDK Manager - Handles initialization and configuration of Memori SDK
for persistent memory storage in PostgreSQL
"""

from typing import Optional
from loguru import logger

try:
    from memori import Memori
    from memori.database.connectors import PostgreSQLConnector
    MEMORI_AVAILABLE = True
except ImportError:
    logger.warning("Memori SDK not available. Install with: pip install git+https://github.com/GibsonAI/memori.git")
    MEMORI_AVAILABLE = False
    Memori = None
    PostgreSQLConnector = None


class MemoriManager:
    """
    Manager for Memori SDK integration with PostgreSQL
    
    Handles:
    - Memori SDK initialization with PostgreSQL backend
    - Connection management
    - Memory operations delegation
    """
    
    def __init__(self, connection_string: str, namespace: str = "ai_world"):
        """
        Initialize Memori Manager
        
        Args:
            connection_string: PostgreSQL connection string (postgresql+psycopg2://...)
            namespace: Namespace for memory isolation (default: "ai_world")
        """
        self.connection_string = connection_string
        self.namespace = namespace
        self.memori_client: Optional[Memori] = None
        self._initialized = False
        
        if not MEMORI_AVAILABLE:
            logger.error("Memori SDK is not available. Memory features will be disabled.")
    
    def initialize(self) -> bool:
        """
        Initialize Memori SDK with PostgreSQL backend
        
        Returns:
            bool: True if initialization successful, False otherwise
        """
        if not MEMORI_AVAILABLE:
            logger.error("Cannot initialize Memori: SDK not available")
            return False
        
        try:
            logger.info(f"Initializing Memori SDK with PostgreSQL backend (namespace: {self.namespace})")
            
            # Initialize Memori with PostgreSQL connector
            self.memori_client = Memori(
                database_connect=self.connection_string,
                namespace=self.namespace,
                auto_ingest=True,  # Automatically store conversation history
                conscious_ingest=True,  # Store important decisions and preferences
                verbose=False,  # Disable verbose logging (we use loguru)
            )
            
            self._initialized = True
            logger.info("✓ Memori SDK initialized successfully with PostgreSQL backend")
            
            # Log database info
            logger.info(f"Memori namespace: {self.namespace}")
            logger.info(f"Memori auto-ingest: enabled")
            logger.info(f"Memori conscious-ingest: enabled")
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize Memori SDK: {e}", exc_info=True)
            self._initialized = False
            return False
    
    def get_client(self) -> Optional[Memori]:
        """
        Get Memori client instance
        
        Returns:
            Memori client or None if not initialized
        """
        if not self._initialized:
            logger.warning("Memori client requested but not initialized")
            return None
        return self.memori_client
    
    def is_available(self) -> bool:
        """Check if Memori SDK is available and initialized"""
        return MEMORI_AVAILABLE and self._initialized
    
    def shutdown(self):
        """Shutdown Memori client"""
        if self.memori_client:
            try:
                # Memori SDK handles cleanup automatically
                logger.info("Memori SDK shutdown complete")
                self._initialized = False
            except Exception as e:
                logger.error(f"Error during Memori shutdown: {e}")


# Global Memori manager instance (initialized in main.py)
memori_manager: Optional[MemoriManager] = None


def get_memori_manager() -> Optional[MemoriManager]:
    """Get global Memori manager instance"""
    return memori_manager


def initialize_memori(connection_string: str, namespace: str = "ai_world") -> MemoriManager:
    """
    Initialize global Memori manager
    
    Args:
        connection_string: PostgreSQL connection string
        namespace: Namespace for memory isolation
        
    Returns:
        MemoriManager instance
    """
    global memori_manager
    
    logger.info("Initializing global Memori manager")
    memori_manager = MemoriManager(connection_string, namespace)
    
    if memori_manager.initialize():
        logger.info("✓ Global Memori manager initialized successfully")
    else:
        logger.warning("⚠ Memori manager initialization failed - memory features will be limited")
    
    return memori_manager

