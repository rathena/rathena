"""
AI IPC Service Test Suite

This package contains comprehensive tests for the AI IPC service including:
- Unit tests: Testing individual components in isolation
- Integration tests: Testing component interactions with real database
- End-to-end tests: Testing complete workflows from NPC script to response

Test Categories:
    - tests/unit/: Unit tests with mocked dependencies
    - tests/integration/: Integration tests with real database
    - tests/e2e/: End-to-end workflow tests

Running Tests:
    # Run all tests
    pytest
    
    # Run with coverage
    pytest --cov=. --cov-report=html
    
    # Run specific category
    pytest tests/unit/
    pytest tests/integration/
    pytest tests/e2e/
    
    # Run with verbose output
    pytest -v
"""

__version__ = "1.0.0"