#!/usr/bin/env python3
"""
AI IPC Service - Main Entry Point

This is the main entry point for the AI IPC polling service.
It initializes configuration, database connection, and starts
the polling loop with graceful shutdown handling.

Usage:
    # Run directly
    python -m ai_ipc_service.main
    
    # Or as a script
    python main.py
    
    # With custom config
    AI_IPC_CONFIG=/path/to/config.yaml python main.py

Environment Variables:
    AI_IPC_CONFIG: Path to YAML configuration file
    DB_HOST, DB_PORT, DB_USER, DB_PASSWORD, DB_NAME: Database settings
    LOG_LEVEL: Logging level (DEBUG, INFO, WARNING, ERROR)
    
Signals:
    SIGINT (Ctrl+C): Graceful shutdown
    SIGTERM: Graceful shutdown
"""

from __future__ import annotations

import asyncio
import logging
import os
import signal
import sys
from datetime import datetime
from pathlib import Path
from typing import Any

import structlog

# Add parent directory to path for imports when running directly
if __name__ == "__main__":
    sys.path.insert(0, str(Path(__file__).parent.parent))

from ai_ipc_service.config import Config
from ai_ipc_service.database import Database, ConnectionError
from ai_ipc_service.processor import RequestProcessor, ProcessorError


# Structured logging configuration
def configure_logging(config: Config) -> None:
    """
    Configure logging based on configuration.
    
    Sets up either JSON (structured) or text logging based on
    the logging.format configuration option.
    
    Args:
        config: Service configuration
    """
    log_level = getattr(logging, config.logging.level.upper(), logging.INFO)
    
    if config.logging.format.lower() == "json":
        # Configure structlog for JSON output
        structlog.configure(
            processors=[
                structlog.stdlib.filter_by_level,
                structlog.stdlib.add_logger_name,
                structlog.stdlib.add_log_level,
                structlog.stdlib.PositionalArgumentsFormatter(),
                structlog.processors.TimeStamper(fmt="iso"),
                structlog.processors.StackInfoRenderer(),
                structlog.processors.format_exc_info,
                structlog.processors.UnicodeDecoder(),
                structlog.processors.JSONRenderer()
            ],
            context_class=dict,
            logger_factory=structlog.stdlib.LoggerFactory(),
            wrapper_class=structlog.stdlib.BoundLogger,
            cache_logger_on_first_use=True,
        )
        
        # Configure stdlib logging for JSON
        logging.basicConfig(
            format="%(message)s",
            level=log_level,
            stream=sys.stdout,
        )
    else:
        # Configure standard text logging
        log_format = (
            "%(asctime)s [%(levelname)s] %(name)s: %(message)s"
        )
        logging.basicConfig(
            format=log_format,
            level=log_level,
            stream=sys.stdout,
        )
    
    # Reduce noise from third-party libraries
    logging.getLogger("aiomysql").setLevel(logging.WARNING)
    logging.getLogger("aiohttp").setLevel(logging.WARNING)


class AIIPCService:
    """
    Main service class for AI IPC polling service.
    
    Manages the lifecycle of the service including:
    - Configuration loading
    - Database connection
    - Polling loop
    - Graceful shutdown
    
    Attributes:
        config: Service configuration
        db: Database interface
        processor: Request processor
        running: Flag indicating service is active
    """
    
    def __init__(self, config_path: str | Path | None = None) -> None:
        """
        Initialize the AI IPC service.
        
        Args:
            config_path: Optional path to configuration file
        """
        self.config: Config | None = None
        self.db: Database | None = None
        self.processor: RequestProcessor | None = None
        self._running = False
        self._shutdown_event = asyncio.Event()
        self._config_path = config_path
        self.logger = logging.getLogger(self.__class__.__name__)
    
    async def start(self) -> None:
        """
        Start the AI IPC service.
        
        Loads configuration, connects to database, and starts
        the polling loop.
        
        Raises:
            Various exceptions on startup failure
        """
        self.logger.info("=" * 60)
        self.logger.info("AI IPC Service Starting")
        self.logger.info("=" * 60)
        
        # Load configuration
        self.logger.info("Loading configuration...")
        self.config = Config.load(self._config_path)
        
        # Configure logging based on config
        configure_logging(self.config)
        
        # Log configuration (with masked password)
        self.logger.info(f"Configuration: {self.config.to_dict()}")
        
        # Validate configuration
        warnings = self.config.validate()
        for warning in warnings:
            self.logger.warning(f"Config warning: {warning}")
        
        # Initialize database
        self.logger.info("Connecting to database...")
        self.db = Database(self.config.database)
        
        try:
            await self.db.connect()
        except ConnectionError as e:
            self.logger.error(f"Failed to connect to database: {e}")
            raise
        
        # Verify database connectivity
        health = await self.db.health_check()
        self.logger.info(f"Database health: {health}")
        
        if health.get("status") != "healthy":
            raise ConnectionError("Database health check failed")
        
        # Initialize processor
        self.logger.info("Initializing request processor...")
        self.processor = RequestProcessor(self.db, self.config)
        
        # Mark as running
        self._running = True
        self.processor.start()
        
        self.logger.info("=" * 60)
        self.logger.info("AI IPC Service Started Successfully")
        self.logger.info(f"  Service: {self.config.service_name} v{self.config.version}")
        self.logger.info(f"  Database: {self.config.database.database}@{self.config.database.host}")
        self.logger.info(f"  Polling: {self.config.polling.interval_ms}ms interval")
        self.logger.info(f"  Workers: {self.config.polling.worker_count}")
        self.logger.info(f"  Batch size: {self.config.polling.batch_size}")
        self.logger.info("=" * 60)
    
    async def run(self) -> None:
        """
        Run the main polling loop.
        
        Continuously polls for requests until shutdown is signaled.
        Includes periodic cleanup of expired requests.
        """
        if not self._running or self.processor is None or self.config is None:
            raise RuntimeError("Service not started. Call start() first.")
        
        poll_interval_sec = self.config.polling.interval_ms / 1000.0
        cleanup_interval_sec = 60.0  # Run cleanup every minute
        last_cleanup = asyncio.get_event_loop().time()
        
        self.logger.info("Starting polling loop...")
        
        poll_count = 0
        while self._running:
            try:
                # Process a batch of requests
                processed = await self.processor.process_batch()
                poll_count += 1
                
                # Log periodic status
                if poll_count % 100 == 0:
                    stats = self.processor.get_stats()
                    self.logger.info(
                        f"Status: processed={stats['requests_processed']}, "
                        f"failed={stats['requests_failed']}, "
                        f"rate={stats['requests_per_second']:.2f}/s"
                    )
                
                # Periodic cleanup
                current_time = asyncio.get_event_loop().time()
                if current_time - last_cleanup >= cleanup_interval_sec:
                    await self.processor.cleanup_expired()
                    last_cleanup = current_time
                
                # Check for shutdown signal
                if self._shutdown_event.is_set():
                    self.logger.info("Shutdown signal received")
                    break
                
                # Wait for next poll interval (or shutdown)
                try:
                    await asyncio.wait_for(
                        self._shutdown_event.wait(),
                        timeout=poll_interval_sec
                    )
                    # If we get here, shutdown was signaled
                    break
                except asyncio.TimeoutError:
                    # Normal timeout, continue polling
                    pass
                
            except ProcessorError as e:
                self.logger.error(f"Processor error: {e}")
                # Back off on processor errors
                await asyncio.sleep(1.0)
                
            except Exception as e:
                self.logger.error(f"Unexpected error in polling loop: {e}", exc_info=True)
                # Back off on unexpected errors
                await asyncio.sleep(5.0)
        
        self.logger.info("Polling loop ended")
    
    async def stop(self) -> None:
        """
        Stop the AI IPC service gracefully.
        
        Signals shutdown, waits for current processing to complete,
        and closes all connections.
        """
        self.logger.info("Stopping AI IPC Service...")
        
        # Signal shutdown
        self._running = False
        self._shutdown_event.set()
        
        # Stop processor
        if self.processor:
            self.processor.stop()
            
            # Log final statistics
            stats = self.processor.get_stats()
            self.logger.info("=" * 60)
            self.logger.info("Final Statistics:")
            self.logger.info(f"  Requests processed: {stats['requests_processed']}")
            self.logger.info(f"  Requests failed: {stats['requests_failed']}")
            self.logger.info(f"  Requests expired: {stats['requests_expired']}")
            self.logger.info(f"  Batches processed: {stats['batches_processed']}")
            self.logger.info(f"  Uptime: {stats['uptime_seconds']} seconds")
            self.logger.info(f"  Avg rate: {stats['requests_per_second']:.2f}/s")
            self.logger.info("=" * 60)
        
        # Close database connection
        if self.db:
            self.logger.info("Closing database connection...")
            await self.db.disconnect()
        
        self.logger.info("AI IPC Service stopped")
    
    def request_shutdown(self) -> None:
        """
        Request graceful shutdown.
        
        Can be called from signal handlers or other contexts.
        """
        self.logger.info("Shutdown requested")
        self._shutdown_event.set()


def setup_signal_handlers(service: AIIPCService, loop: asyncio.AbstractEventLoop) -> None:
    """
    Setup signal handlers for graceful shutdown.
    
    Handles SIGINT (Ctrl+C) and SIGTERM signals.
    
    Args:
        service: Service instance to shutdown
        loop: Event loop for signal handling
    """
    def signal_handler(sig: signal.Signals) -> None:
        logging.info(f"Received signal {sig.name}")
        service.request_shutdown()
    
    # Register signal handlers
    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, signal_handler, sig)
        except NotImplementedError:
            # Windows doesn't support add_signal_handler
            signal.signal(sig, lambda s, f: signal_handler(signal.Signals(s)))


async def main(config_path: str | Path | None = None) -> int:
    """
    Main entry point for the AI IPC service.
    
    Args:
        config_path: Optional path to configuration file
        
    Returns:
        Exit code (0 for success, non-zero for failure)
    """
    service = AIIPCService(config_path)
    
    # Setup signal handlers
    loop = asyncio.get_running_loop()
    setup_signal_handlers(service, loop)
    
    try:
        # Start service
        await service.start()
        
        # Run polling loop
        await service.run()
        
        # Graceful shutdown
        await service.stop()
        
        return 0
        
    except KeyboardInterrupt:
        logging.info("Keyboard interrupt received")
        await service.stop()
        return 0
        
    except ConnectionError as e:
        logging.error(f"Database connection error: {e}")
        return 1
        
    except Exception as e:
        logging.error(f"Fatal error: {e}", exc_info=True)
        try:
            await service.stop()
        except Exception:
            pass
        return 1


def run() -> None:
    """
    Entry point for console script.
    
    Parses command line arguments and runs the service.
    """
    import argparse
    
    parser = argparse.ArgumentParser(
        description="AI IPC Service - Database polling service for rAthena NPC scripts"
    )
    parser.add_argument(
        "-c", "--config",
        help="Path to configuration file (default: config.yaml)",
        default=None
    )
    parser.add_argument(
        "-v", "--version",
        action="version",
        version="ai_ipc_service 1.0.0"
    )
    
    args = parser.parse_args()
    
    # Run service
    exit_code = asyncio.run(main(args.config))
    sys.exit(exit_code)


if __name__ == "__main__":
    run()