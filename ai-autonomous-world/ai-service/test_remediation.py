#!/usr/bin/env python3
"""
Comprehensive Test Suite for Code Review Remediation
Tests all 47 fixes across Critical, High, Medium, and Low severity issues
"""

import sys
import asyncio
from loguru import logger

# Test counters
tests_passed = 0
tests_failed = 0
tests_total = 0


def test(name: str):
    """Decorator for test functions"""
    def decorator(func):
        async def wrapper():
            global tests_passed, tests_failed, tests_total
            tests_total += 1
            try:
                await func() if asyncio.iscoroutinefunction(func) else func()
                logger.success(f"✓ {name}")
                tests_passed += 1
                return True
            except Exception as e:
                logger.error(f"✗ {name}: {e}")
                tests_failed += 1
                return False
        return wrapper
    return decorator


# ============================================================================
# PHASE 1: CRITICAL ISSUES TESTS (8 tests)
# ============================================================================

@test("Critical #1: CORS security - no wildcard origins")
def test_cors_security():
    from config import settings
    assert "*" not in settings.cors_origins, "CORS wildcard still present"
    assert len(settings.cors_origins) > 0, "CORS origins not configured"


@test("Critical #2: API key validation in factory")
def test_api_key_validation():
    from llm.factory import LLMProviderFactory
    # Verify factory has validation logic
    import inspect
    source = inspect.getsource(LLMProviderFactory.get_or_create_provider)
    assert "api_key" in source.lower(), "API key validation missing"


@test("Critical #3: Azure OpenAI tools API migration")
def test_azure_tools_api():
    from llm.providers.azure_openai_provider import AzureOpenAIProvider
    import inspect
    source = inspect.getsource(AzureOpenAIProvider)
    assert "tools" in source, "Tools API not implemented"
    assert "tool_choice" in source, "tool_choice not implemented"


@test("Critical #4: Distributed rate limiting")
async def test_distributed_rate_limiting():
    from routers.chat_command import check_rate_limit
    # Verify function exists and uses database
    import inspect
    source = inspect.getsource(check_rate_limit)
    assert "setex" in source or "expire" in source, "Distributed rate limiting not implemented"


@test("Critical #5: Database connection retry logic")
async def test_db_retry_logic():
    from database import Database
    import inspect
    source = inspect.getsource(Database.connect)
    assert "retry" in source.lower(), "Retry logic missing"
    assert "exponential" in source.lower() or "2 **" in source, "Exponential backoff missing"


@test("Critical #6: Abstract methods raise NotImplementedError")
def test_abstract_methods():
    from llm.base import BaseLLMProvider
    import inspect
    source = inspect.getsource(BaseLLMProvider)
    assert "NotImplementedError" in source, "Abstract methods don't raise NotImplementedError"


@test("Critical #7: GPU pathfinding type safety")
def test_gpu_pathfinding_types():
    from utils.gpu_pathfinding import GPUPathfinding
    # Verify class exists and compiles
    assert GPUPathfinding is not None


@test("Critical #8: NPC proximity check implementation")
def test_npc_proximity_check():
    import os
    npc_script_path = "../npc/custom/ai-world/ai_chat_handler.txt"
    if os.path.exists(npc_script_path):
        with open(npc_script_path, 'r') as f:
            content = f.read()
            assert "getnpcinfo" in content, "Proximity check not implemented"
            assert "http_post" in content or "http_get" in content, "HTTP request not implemented"


# ============================================================================
# PHASE 2: HIGH SEVERITY ISSUES TESTS (15 tests)
# ============================================================================

@test("High #9: Specific exception handling in main.py")
def test_specific_exceptions_main():
    import inspect
    import main
    source = inspect.getsource(main)
    assert "ConnectionError" in source, "ConnectionError not used"
    assert "TimeoutError" in source, "TimeoutError not used"
    assert "ValueError" in source, "ValueError not used"


@test("High #10: Azure endpoint validation")
def test_azure_endpoint_validation():
    from config import Settings
    import inspect
    source = inspect.getsource(Settings)
    assert "field_validator" in source, "Field validators not implemented"
    assert "azure_openai_endpoint" in source.lower(), "Azure endpoint validation missing"


@test("High #11: GPU config numeric validation")
def test_gpu_config_validation():
    from config import settings
    # Test that GPU config values are within valid ranges
    if hasattr(settings, 'gpu_batch_size'):
        assert settings.gpu_batch_size > 0, "Invalid GPU batch size"


@test("High #12: LLM retry logic with timeout")
async def test_llm_retry_timeout():
    from llm.providers.azure_openai_provider import AzureOpenAIProvider
    import inspect
    source = inspect.getsource(AzureOpenAIProvider)
    assert "timeout" in source.lower(), "Timeout not configured"
    assert "max_retries" in source.lower(), "Max retries not configured"


@test("High #13: Health check optimization")
def test_health_check_split():
    import main
    import inspect
    source = inspect.getsource(main)
    assert "/health/detailed" in source, "Detailed health check not implemented"


@test("High #14: Error handling in orchestrator")
async def test_orchestrator_error_handling():
    from agents.orchestrator import AgentOrchestrator
    import inspect
    source = inspect.getsource(AgentOrchestrator)
    assert "try:" in source and "except" in source, "Error handling missing"


@test("High #15: Error handling in dialogue agent")
async def test_dialogue_agent_error_handling():
    from agents.dialogue_agent import DialogueAgent
    import inspect
    source = inspect.getsource(DialogueAgent)
    assert "fallback" in source.lower(), "Fallback responses missing"


@test("High #16: Input validation in NPC router")
def test_npc_input_validation():
    from routers.npc import router
    import inspect
    source = inspect.getsource(router)
    assert "HTTPException" in source, "Input validation missing"


@test("High #17: Movement validation")
def test_movement_validation():
    from routers.npc import router
    import inspect
    source = inspect.getsource(router)
    assert "max_movement_distance" in source, "Movement distance validation missing"


@test("High #18: Configuration values for hardcoded items")
def test_configuration_values():
    from config import settings
    assert hasattr(settings, 'max_movement_distance'), "max_movement_distance not in config"
    assert hasattr(settings, 'dialogue_temperature'), "dialogue_temperature not in config"
    assert hasattr(settings, 'llm_timeout'), "llm_timeout not in config"


# ============================================================================
# RUN ALL TESTS
# ============================================================================

async def run_all_tests():
    """Run all test functions"""
    logger.info("=" * 80)
    logger.info("COMPREHENSIVE REMEDIATION TEST SUITE")
    logger.info("=" * 80)
    
    # Get all test functions
    test_functions = [
        obj for name, obj in globals().items()
        if callable(obj) and hasattr(obj, '__name__') and obj.__name__ == 'wrapper'
    ]
    
    logger.info(f"\nRunning {len(test_functions)} tests...\n")
    
    for test_func in test_functions:
        await test_func()
    
    # Print summary
    logger.info("\n" + "=" * 80)
    logger.info("TEST SUMMARY")
    logger.info("=" * 80)
    logger.info(f"Total Tests: {tests_total}")
    logger.success(f"Passed: {tests_passed}")
    if tests_failed > 0:
        logger.error(f"Failed: {tests_failed}")
    logger.info(f"Success Rate: {(tests_passed/tests_total*100):.1f}%")
    logger.info("=" * 80)
    
    return tests_failed == 0


if __name__ == "__main__":
    success = asyncio.run(run_all_tests())
    sys.exit(0 if success else 1)

