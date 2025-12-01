"""
AI IPC Service - Database Polling Service for rAthena NPC Scripts

This service replaces HTTP-based NPC script commands (httpget, httppost) with
database-based IPC (Inter-Process Communication). It polls the MySQL database
for pending requests, processes them through appropriate handlers, and writes
responses back to the database.

Architecture:
    NPC Script -> ai_requests table -> This Service -> ai_responses table -> NPC Script

Components:
    - config: Configuration management (YAML and environment variables)
    - database: Async MySQL connection pool and queries
    - processor: Request routing and batch processing
    - handlers: Request type handlers (health_check, http_proxy, ai_dialogue)

Usage:
    python -m ai_ipc_service.main
    
    Or:
    from ai_ipc_service.main import main
    asyncio.run(main())

Environment Variables:
    AI_IPC_CONFIG: Path to YAML config file (default: config.yaml)
    DB_HOST: Database host (default: localhost)
    DB_PORT: Database port (default: 3306)
    DB_USER: Database user
    DB_PASSWORD: Database password
    DB_NAME: Database name (default: ragnarok)

Version: 1.0.0
Author: AI Architecture Team
"""

__version__ = "1.0.0"
__author__ = "AI Architecture Team"
__all__ = [
    "config",
    "database",
    "processor",
    "handlers",
]