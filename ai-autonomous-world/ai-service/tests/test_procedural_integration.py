"""
Integration tests for Procedural Content Agents
Tests all 3 agents working together with database and cache
"""

import pytest
import asyncio
from datetime import datetime, timedelta, UTC

from agents.procedural.problem_agent import problem_agent
from agents.procedural.dynamic_npc_agent import dynamic_npc_agent
from agents.procedural.world_event_agent import world_event_agent
from database import postgres_db, db
from config import settings


@pytest.fixture
def sample_world_state():
    """Complete world state for integration testing"""
    return {
        "avg_player_level": 55,
        "online_players": 20,
        "map_activity": {
            "prontera": 100,
            "geffen": 50,
            "payon": 20,
            "morocc": 10,
            "cmd_fild01": 5,
            "mjo_dun01": 2
        },
        "monster_kills": {
            "prt_fild01": 150,
            "prt_fild02": 100
        },
        "mvp_kills": {
            "1038": 8,
            "1039": 12,
            "1046": 6
        },
        "economy": {
            "zeny_circulation": 1500000000,
            "inflation_rate": 1.08,
            "scarce_items": ["Old Card Album", "Angelic Ring"]
        }
    }


@pytest.mark.integration
@pytest.mark.asyncio
async def test_full_procedural_workflow(sample_world_state):
    """
    Test complete procedural content workflow:
    1. Generate daily problem
    2. Spawn dynamic NPCs
    3. Check for event triggers
    4. Verify all data persisted
    """
    # This test requires actual database connections
    # Skip if databases not available
    try:
        await postgres_db.health_check()
        await db.health_check()
    except Exception:
        pytest.skip("Database not available for integration test")
    
    # Step 1: Generate daily problem
    print("\n=== Testing Problem Generation ===")
    problem_response = await problem_agent.generate_daily_problem(sample_world_state)
    
    assert problem_response.success, f"Problem generation failed: {problem_response.reasoning}"
    assert "problem_id" in problem_response.data
    
    problem_id = problem_response.data['problem_id']
    print(f"✓ Generated problem #{problem_id}: {problem_response.data['title']}")
    
    # Verify problem stored in database
    problem = await problem_agent.get_current_problem()
    assert problem is not None
    assert problem['problem_id'] == problem_id
    
    # Step 2: Spawn dynamic NPCs
    print("\n=== Testing NPC Spawning ===")
    npc_responses = await dynamic_npc_agent.spawn_daily_npcs(
        map_activity=sample_world_state['map_activity'],
        count=3
    )
    
    assert len(npc_responses) > 0, "No NPCs spawned"
    print(f"✓ Spawned {len(npc_responses)} NPCs")
    
    # Verify NPCs in database
    active_npcs = await dynamic_npc_agent.get_active_npcs()
    assert len(active_npcs) >= len(npc_responses)
    
    # Step 3: Test NPC interaction
    if npc_responses:
        npc_id = npc_responses[0].data['npc_id']
        print(f"\n=== Testing NPC Interaction (NPC #{npc_id}) ===")
        
        interaction_response = await dynamic_npc_agent.handle_npc_interaction(
            npc_id=npc_id,
            player_id=100001,
            interaction_type="talk"
        )
        
        assert interaction_response.success
        assert "npc_response" in interaction_response.data
        print(f"✓ NPC interaction successful")
    
    # Step 4: Check event triggers
    print("\n=== Testing Event Monitoring ===")
    
    # Create high MVP kill state to trigger event
    high_mvp_state = {
        **sample_world_state,
        "mvp_kills": {str(i): 50 for i in range(10)}  # 500 total kills
    }
    
    event_response = await world_event_agent.monitor_and_trigger(high_mvp_state)
    
    if event_response and event_response.success:
        event_id = event_response.data['event_id']
        print(f"✓ Event triggered #{event_id}: {event_response.data['title']}")
        
        # Verify event in database
        active_events = await world_event_agent.get_active_events()
        assert any(e['event_id'] == event_id for e in active_events)
    else:
        print("✓ No event triggered (thresholds not met or cooldown active)")
    
    # Step 5: Test participation recording
    print("\n=== Testing Participation Recording ===")
    
    # Problem participation
    problem_participation = await problem_agent.record_participation(
        problem_id=problem_id,
        player_id=100001,
        action_type="monster_kill",
        contribution_score=50
    )
    assert problem_participation
    print("✓ Problem participation recorded")
    
    # Step 6: Verify cache consistency
    print("\n=== Testing Cache Consistency ===")
    
    # Problem should be cached
    cached_problem = await db.get("problem:active")
    assert cached_problem is not None
    print("✓ Problem cached in DragonflyDB")
    
    # NPCs should be cacheable
    cached_npcs = await db.get("npcs:active")
    # May be None if not cached yet, but shouldn't error
    print(f"✓ NPC cache check complete (cached: {cached_npcs is not None})")
    
    print("\n=== Integration Test Complete ===")


@pytest.mark.integration
@pytest.mark.asyncio
async def test_concurrent_agent_operations(sample_world_state):
    """
    Test concurrent operations across all 3 agents.
    Verifies thread safety and database connection pooling.
    """
    try:
        await postgres_db.health_check()
        await db.health_check()
    except Exception:
        pytest.skip("Database not available for integration test")
    
    print("\n=== Testing Concurrent Agent Operations ===")
    
    # Create concurrent tasks
    tasks = [
        problem_agent.get_current_problem(),
        dynamic_npc_agent.get_active_npcs(),
        world_event_agent.get_active_events(),
        problem_agent.get_current_problem(),  # Duplicate to test caching
        dynamic_npc_agent.get_active_npcs(),  # Duplicate to test caching
    ]
    
    # Execute concurrently
    results = await asyncio.gather(*tasks, return_exceptions=True)
    
    # Verify no exceptions
    exceptions = [r for r in results if isinstance(r, Exception)]
    assert len(exceptions) == 0, f"Concurrent operations failed: {exceptions}"
    
    print(f"✓ {len(tasks)} concurrent operations completed successfully")


@pytest.mark.integration
@pytest.mark.asyncio
async def test_scheduler_integration():
    """
    Test procedural scheduler initialization and job registration.
    """
    from tasks.procedural_scheduler import procedural_scheduler
    
    print("\n=== Testing Scheduler Integration ===")
    
    # Check if scheduler can be started
    if not procedural_scheduler.is_running():
        procedural_scheduler.start()
        print("✓ Scheduler started")
    
    # Verify jobs registered
    jobs = procedural_scheduler.get_jobs()
    assert len(jobs) > 0, "No jobs registered"
    
    job_names = [job.name for job in jobs]
    print(f"✓ Registered jobs: {', '.join(job_names)}")
    
    # Expected jobs
    expected_jobs = [
        'Generate Daily Problem',
        'Spawn Daily NPCs',
        'Despawn Expired NPCs',
        'Monitor World Events',
        'Cleanup Old Data'
    ]
    
    for expected in expected_jobs:
        assert expected in job_names, f"Missing job: {expected}"
    
    print("✓ All expected jobs registered")


@pytest.mark.integration  
@pytest.mark.asyncio
async def test_api_router_registration():
    """
    Test that procedural router is registered in FastAPI app.
    """
    from main import app
    
    print("\n=== Testing API Router Registration ===")
    
    # Get all routes
    routes = [route.path for route in app.routes]
    
    # Check procedural endpoints
    procedural_endpoints = [
        "/api/v1/procedural/problem/generate",
        "/api/v1/procedural/problem/current",
        "/api/v1/procedural/npc/spawn",
        "/api/v1/procedural/npc/active",
        "/api/v1/procedural/events/check",
        "/api/v1/procedural/events/active",
        "/api/v1/procedural/status"
    ]
    
    for endpoint in procedural_endpoints:
        assert endpoint in routes, f"Missing endpoint: {endpoint}"
        print(f"✓ Registered: {endpoint}")
    
    print(f"✓ All {len(procedural_endpoints)} procedural endpoints registered")


@pytest.mark.integration
@pytest.mark.asyncio
async def test_database_migration_applied():
    """
    Test that database migration was applied successfully.
    """
    try:
        await postgres_db.health_check()
    except Exception:
        pytest.skip("PostgreSQL not available")
    
    print("\n=== Testing Database Migration ===")
    
    # Check tables exist
    tables = [
        "world_problems",
        "problem_history",
        "dynamic_npcs",
        "npc_interactions",
        "world_events",
        "event_participation"
    ]
    
    for table in tables:
        result = await postgres_db.fetch_one(
            "SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_name = $1)",
            table
        )
        assert result['exists'], f"Table {table} not found"
        print(f"✓ Table exists: {table}")
    
    # Check functions exist
    functions = [
        "expire_old_problems",
        "despawn_expired_npcs",
        "expire_old_events",
        "cleanup_old_problem_history",
        "cleanup_old_npc_interactions",
        "cleanup_old_event_participation"
    ]
    
    for func in functions:
        result = await postgres_db.fetch_one(
            "SELECT EXISTS (SELECT FROM pg_proc WHERE proname = $1)",
            func
        )
        assert result['exists'], f"Function {func} not found"
        print(f"✓ Function exists: {func}")
    
    print("✓ Database migration successfully applied")


@pytest.mark.integration
@pytest.mark.asyncio
async def test_end_to_end_problem_lifecycle(sample_world_state):
    """
    Test complete problem lifecycle:
    Generate -> Participate -> Complete -> Cleanup
    """
    try:
        await postgres_db.health_check()
        await db.health_check()
    except Exception:
        pytest.skip("Database not available")
    
    print("\n=== Testing Problem Lifecycle ===")
    
    # 1. Generate problem
    response = await problem_agent.generate_daily_problem(sample_world_state)
    assert response.success
    problem_id = response.data['problem_id']
    print(f"1. ✓ Generated problem #{problem_id}")
    
    # 2. Record participation
    participated = await problem_agent.record_participation(
        problem_id=problem_id,
        player_id=100001,
        action_type="monster_kill",
        contribution_score=100
    )
    assert participated
    print(f"2. ✓ Recorded participation")
    
    # 3. Get current problem (should be cached)
    current = await problem_agent.get_current_problem()
    assert current is not None
    assert current['problem_id'] == problem_id
    print(f"3. ✓ Retrieved from cache")
    
    # 4. Complete problem
    completed = await problem_agent.complete_problem(problem_id)
    assert completed
    print(f"4. ✓ Completed problem")
    
    # 5. Verify status updated
    updated = await postgres_db.fetch_one(
        "SELECT status FROM world_problems WHERE problem_id = $1",
        problem_id
    )
    assert updated['status'] == 'completed'
    print(f"5. ✓ Status updated in database")
    
    print("✓ Complete problem lifecycle tested successfully")


if __name__ == "__main__":
    """Run integration tests manually"""
    print("=" * 80)
    print("PROCEDURAL AGENTS INTEGRATION TESTS")
    print("=" * 80)
    
    pytest.main([__file__, "-v", "-m", "integration"])