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
except ImportError as e:
    error_msg = (
        "Memori SDK is REQUIRED but not installed. "
        "Install with: pip install git+https://github.com/GibsonAI/memori.git"
    )
    logger.error(error_msg)
    raise ImportError(error_msg) from e


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

        Raises:
            ImportError: If Memori SDK is not available (required dependency)
        """
        if not MEMORI_AVAILABLE:
            raise ImportError(
                "Memori SDK is REQUIRED but not available. "
                "Install with: pip install git+https://github.com/GibsonAI/memori.git"
            )

        self.connection_string = connection_string
        self.namespace = namespace
        self.memori_client: Optional[Memori] = None
        self._initialized = False
    
    def initialize(self):
        """
        Initialize Memori SDK with PostgreSQL backend

        Raises:
            RuntimeError: If initialization fails (required for operation)
        """
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

        except Exception as e:
            error_msg = f"CRITICAL: Failed to initialize Memori SDK (required dependency): {e}"
            logger.error(error_msg, exc_info=True)
            raise RuntimeError(error_msg) from e
    
    def get_client(self) -> Memori:
        """
        Get Memori client instance

        Returns:
            Memori client instance

        Raises:
            RuntimeError: If Memori is not initialized
        """
        if not self._initialized or self.memori_client is None:
            raise RuntimeError(
                "Memori client is not initialized. Call initialize() first."
            )
        return self.memori_client

    def is_available(self) -> bool:
        """Check if Memori SDK is initialized and ready"""
        return self._initialized and self.memori_client is not None
    
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

    Raises:
        ImportError: If Memori SDK is not available
        RuntimeError: If initialization fails
    """
    global memori_manager

    logger.info("Initializing global Memori manager (REQUIRED)")
    memori_manager = MemoriManager(connection_string, namespace)
    memori_manager.initialize()  # Will raise RuntimeError if fails
    logger.info("✓ Global Memori manager initialized successfully")

    return memori_manager

