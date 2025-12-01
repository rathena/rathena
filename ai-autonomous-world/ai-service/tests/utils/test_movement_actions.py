"""
Tests for Movement Actions Utility

Comprehensive test suite covering:
- Position calculations and transformations
- Boundary validation and clamping
- Movement action generation
- Pathfinding and waypoint calculation
- Edge cases and performance
"""

import math
import pytest
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

from utils.movement_actions import (
    Position,
    Boundary,
    MovementType,
    validate_position,
    calculate_movement_speed,
    generate_movement_action_data,
    calculate_patrol_waypoints,
    find_nearest_safe_position,
    calculate_intercept_position,
    is_path_clear
)


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def origin():
    """Origin position (0, 0)"""
    return Position(0.0, 0.0)


@pytest.fixture
def standard_position():
    """Standard test position"""
    return Position(100.0, 150.0)


@pytest.fixture
def standard_boundary():
    """Standard map boundary"""
    return Boundary(0.0, 1000.0, 0.0, 1000.0)


@pytest.fixture
def small_boundary():
    """Small test boundary"""
    return Boundary(0.0, 100.0, 0.0, 100.0)


@pytest.fixture
def danger_positions():
    """List of dangerous positions"""
    return [
        Position(50.0, 50.0),
        Position(150.0, 150.0),
        Position(200.0, 100.0)
    ]


# =============================================================================
# POSITION CLASS TESTS
# =============================================================================

class TestPosition:
    """Test Position class methods"""
    
    def test_position_creation(self):
        """Test position initialization"""
        pos = Position(10.5, 20.5)
        assert pos.x == 10.5
        assert pos.y == 20.5
    
    def test_distance_to(self, origin, standard_position):
        """Test Euclidean distance calculation"""
        distance = origin.distance_to(standard_position)
        expected = math.sqrt(100**2 + 150**2)
        assert abs(distance - expected) < 0.01
    
    def test_distance_to_same_position(self, standard_position):
        """Test distance to same position is zero"""
        distance = standard_position.distance_to(standard_position)
        assert distance == 0.0
    
    def test_manhattan_distance(self, origin, standard_position):
        """Test Manhattan distance calculation"""
        distance = origin.manhattan_distance_to(standard_position)
        assert distance == 250.0
    
    def test_direction_to(self, origin):
        """Test direction angle calculation"""
        target = Position(100.0, 0.0)
        angle = origin.direction_to(target)
        assert abs(angle - 0.0) < 0.01  # Points east (0 radians)
        
        target = Position(0.0, 100.0)
        angle = origin.direction_to(target)
        assert abs(angle - math.pi/2) < 0.01  # Points north
    
    def test_move_towards_partial(self, origin):
        """Test moving partway towards target"""
        target = Position(100.0, 0.0)
        moved = origin.move_towards(target, 50.0)
        
        assert abs(moved.x - 50.0) < 0.01
        assert abs(moved.y - 0.0) < 0.01
    
    def test_move_towards_exact(self, origin):
        """Test moving exactly to target"""
        target = Position(100.0, 0.0)
        moved = origin.move_towards(target, 100.0)
        
        assert abs(moved.x - target.x) < 0.01
        assert abs(moved.y - target.y) < 0.01
    
    def test_move_towards_beyond(self, origin):
        """Test moving beyond target snaps to target"""
        target = Position(100.0, 0.0)
        moved = origin.move_towards(target, 200.0)
        
        assert abs(moved.x - target.x) < 0.01
        assert abs(moved.y - target.y) < 0.01
    
    def test_to_dict(self, standard_position):
        """Test dictionary conversion"""
        result = standard_position.to_dict()
        assert result == {'x': 100.0, 'y': 150.0}
    
    @pytest.mark.performance
    def test_position_performance(self, origin):
        """Test position operations performance"""
        target = Position(1000.0, 1000.0)
        
        start = time.perf_counter()
        for _ in range(10000):
            _ = origin.distance_to(target)
            _ = origin.direction_to(target)
            _ = origin.move_towards(target, 10.0)
        duration = time.perf_counter() - start
        
        assert duration < 0.1  # Should complete in < 100ms


# =============================================================================
# BOUNDARY CLASS TESTS
# =============================================================================

class TestBoundary:
    """Test Boundary class methods"""
    
    def test_boundary_creation(self, standard_boundary):
        """Test boundary initialization"""
        assert standard_boundary.min_x == 0.0
        assert standard_boundary.max_x == 1000.0
        assert standard_boundary.min_y == 0.0
        assert standard_boundary.max_y == 1000.0
    
    def test_contains_inside(self, standard_boundary):
        """Test position inside boundary"""
        pos = Position(500.0, 500.0)
        assert standard_boundary.contains(pos)
    
    def test_contains_on_edge(self, standard_boundary):
        """Test position on boundary edge"""
        pos = Position(0.0, 500.0)
        assert standard_boundary.contains(pos)
        
        pos = Position(1000.0, 500.0)
        assert standard_boundary.contains(pos)
    
    def test_contains_outside(self, standard_boundary):
        """Test position outside boundary"""
        pos = Position(-10.0, 500.0)
        assert not standard_boundary.contains(pos)
        
        pos = Position(1100.0, 500.0)
        assert not standard_boundary.contains(pos)
    
    def test_clamp_inside(self, standard_boundary):
        """Test clamping position already inside"""
        pos = Position(500.0, 500.0)
        clamped = standard_boundary.clamp(pos)
        assert clamped.x == 500.0
        assert clamped.y == 500.0
    
    def test_clamp_outside_x(self, standard_boundary):
        """Test clamping X coordinate"""
        pos = Position(-50.0, 500.0)
        clamped = standard_boundary.clamp(pos)
        assert clamped.x == 0.0
        assert clamped.y == 500.0
        
        pos = Position(1100.0, 500.0)
        clamped = standard_boundary.clamp(pos)
        assert clamped.x == 1000.0
    
    def test_clamp_outside_y(self, standard_boundary):
        """Test clamping Y coordinate"""
        pos = Position(500.0, -50.0)
        clamped = standard_boundary.clamp(pos)
        assert clamped.x == 500.0
        assert clamped.y == 0.0
    
    def test_clamp_outside_both(self, standard_boundary):
        """Test clamping both coordinates"""
        pos = Position(-50.0, 1100.0)
        clamped = standard_boundary.clamp(pos)
        assert clamped.x == 0.0
        assert clamped.y == 1000.0


# =============================================================================
# VALIDATION TESTS
# =============================================================================

class TestValidation:
    """Test position validation functions"""
    
    def test_validate_position_valid(self, standard_position, standard_boundary):
        """Test validating valid position"""
        is_valid, error = validate_position(standard_position, standard_boundary, "test_location")
        assert is_valid
        assert error is None
    
    def test_validate_position_invalid(self, standard_boundary):
        """Test validating invalid position"""
        pos = Position(-50.0, 500.0)
        is_valid, error = validate_position(pos, standard_boundary, "test_location")
        assert not is_valid
        assert "out of bounds" in error
    
    def test_validate_position_default_boundary(self):
        """Test validation with default boundary"""
        pos = Position(500.0, 500.0)
        is_valid, error = validate_position(pos, None)
        assert is_valid
        
        pos = Position(2000.0, 500.0)
        is_valid, error = validate_position(pos, None)
        assert not is_valid


# =============================================================================
# MOVEMENT SPEED TESTS
# =============================================================================

class TestMovementSpeed:
    """Test movement speed calculations"""
    
    def test_walk_speed(self):
        """Test walk speed"""
        speed = calculate_movement_speed(MovementType.WALK, 5.0)
        assert speed == 5.0
    
    def test_run_speed(self):
        """Test run speed (2x walk)"""
        speed = calculate_movement_speed(MovementType.RUN, 5.0)
        assert speed == 10.0
    
    def test_flee_speed(self):
        """Test flee speed (fastest)"""
        speed = calculate_movement_speed(MovementType.FLEE, 5.0)
        assert speed == 12.5
    
    def test_wander_speed(self):
        """Test wander speed (slowest)"""
        speed = calculate_movement_speed(MovementType.WANDER, 5.0)
        assert speed == 3.0
    
    def test_teleport_speed(self):
        """Test teleport (instant)"""
        speed = calculate_movement_speed(MovementType.TELEPORT, 5.0)
        assert speed == 0.0
    
    def test_terrain_modifier(self):
        """Test terrain speed modifier"""
        speed = calculate_movement_speed(MovementType.WALK, 5.0, terrain_modifier=0.5)
        assert speed == 2.5
        
        speed = calculate_movement_speed(MovementType.WALK, 5.0, terrain_modifier=1.5)
        assert speed == 7.5


# =============================================================================
# MOVEMENT ACTION GENERATION TESTS
# =============================================================================

class TestMovementActionGeneration:
    """Test movement action data generation"""
    
    def test_generate_basic_action(self, origin, standard_boundary):
        """Test basic movement action generation"""
        target = Position(100.0, 0.0)
        action = generate_movement_action_data(
            "npc-001",
            origin,
            target,
            MovementType.WALK,
            "prontera",
            standard_boundary
        )
        
        assert action['action'] == 'move'
        assert action['npc_id'] == 'npc-001'
        assert action['movement_type'] == 'walk'
        assert action['location'] == 'prontera'
        assert action['distance'] == 100.0
        assert 'duration_seconds' in action
    
    def test_generate_action_with_metadata(self, origin, standard_boundary):
        """Test action generation with metadata"""
        target = Position(100.0, 0.0)
        metadata = {'reason': 'patrol', 'waypoint': 1}
        
        action = generate_movement_action_data(
            "npc-001",
            origin,
            target,
            MovementType.PATROL,
            "prontera",
            standard_boundary,
            metadata
        )
        
        assert action['metadata'] == metadata
    
    def test_generate_action_invalid_position(self, origin, small_boundary):
        """Test action generation with invalid target"""
        target = Position(500.0, 500.0)  # Outside small boundary
        
        action = generate_movement_action_data(
            "npc-001",
            origin,
            target,
            MovementType.WALK,
            "test",
            small_boundary
        )
        
        # Should clamp to boundary
        assert action['to']['x'] <= 100.0
        assert action['to']['y'] <= 100.0
    
    def test_duration_calculation(self, origin):
        """Test movement duration calculation"""
        target = Position(100.0, 0.0)
        
        action = generate_movement_action_data(
            "npc-001", origin, target, MovementType.WALK
        )
        
        # Distance 100, speed 5 = 20 seconds
        assert abs(action['duration_seconds'] - 20.0) < 0.1
    
    def test_direction_calculation(self, origin):
        """Test direction calculation in action"""
        target = Position(100.0, 0.0)
        action = generate_movement_action_data(
            "npc-001", origin, target, MovementType.WALK
        )
        
        # Should point east (0 degrees)
        assert abs(action['direction_degrees'] - 0.0) < 0.1


# =============================================================================
# PATROL WAYPOINT TESTS
# =============================================================================

class TestPatrolWaypoints:
    """Test patrol waypoint calculation"""
    
    def test_basic_patrol(self):
        """Test basic patrol waypoint generation"""
        center = Position(500.0, 500.0)
        waypoints = calculate_patrol_waypoints(center, 100.0, num_points=4)
        
        assert len(waypoints) == 4
        
        # All waypoints should be ~100 units from center
        for wp in waypoints:
            distance = center.distance_to(wp)
            assert abs(distance - 100.0) < 1.0
    
    def test_patrol_with_boundary(self, small_boundary):
        """Test patrol with boundary clamping"""
        center = Position(90.0, 90.0)
        waypoints = calculate_patrol_waypoints(
            center, 50.0, num_points=4, boundary=small_boundary
        )
        
        # All waypoints should be within boundary
        for wp in waypoints:
            assert small_boundary.contains(wp)
    
    def test_patrol_different_counts(self):
        """Test patrol with different waypoint counts"""
        center = Position(500.0, 500.0)
        
        for count in [3, 6, 8, 12]:
            waypoints = calculate_patrol_waypoints(center, 100.0, num_points=count)
            assert len(waypoints) == count
    
    @pytest.mark.performance
    def test_patrol_performance(self):
        """Test patrol waypoint performance"""
        center = Position(500.0, 500.0)
        
        start = time.perf_counter()
        for _ in range(1000):
            calculate_patrol_waypoints(center, 100.0, num_points=8)
        duration = time.perf_counter() - start
        
        assert duration < 0.1  # Should complete in < 100ms


# =============================================================================
# SAFE POSITION TESTS
# =============================================================================

class TestSafePosition:
    """Test safe position finding"""
    
    def test_find_safe_position(self, danger_positions):
        """Test finding safe position away from danger"""
        current = Position(100.0, 100.0)
        safe_pos = find_nearest_safe_position(
            current, danger_positions, safe_distance=50.0
        )
        
        # Should be at least 50 units from all dangers
        for danger in danger_positions:
            distance = safe_pos.distance_to(danger)
            assert distance >= 45.0  # Allow small tolerance
    
    def test_already_safe(self, danger_positions):
        """Test when already in safe position"""
        current = Position(500.0, 500.0)  # Far from dangers
        safe_pos = find_nearest_safe_position(
            current, danger_positions, safe_distance=50.0
        )
        
        # Should return same position
        assert safe_pos.x == current.x
        assert safe_pos.y == current.y
    
    def test_no_dangers(self):
        """Test with no danger positions"""
        current = Position(100.0, 100.0)
        safe_pos = find_nearest_safe_position(
            current, [], safe_distance=50.0
        )
        
        # Should return current position
        assert safe_pos.x == current.x
        assert safe_pos.y == current.y
    
    def test_safe_position_with_boundary(self, danger_positions, small_boundary):
        """Test safe position with boundary"""
        current = Position(50.0, 50.0)
        safe_pos = find_nearest_safe_position(
            current, danger_positions, safe_distance=30.0,
            boundary=small_boundary
        )
        
        # Should be within boundary
        assert small_boundary.contains(safe_pos)


# =============================================================================
# INTERCEPT CALCULATION TESTS
# =============================================================================

class TestInterceptCalculation:
    """Test intercept position calculation"""
    
    def test_intercept_stationary_target(self):
        """Test intercept with stationary target"""
        chaser = Position(0.0, 0.0)
        target = Position(100.0, 0.0)
        velocity = (0.0, 0.0)  # Stationary
        
        intercept = calculate_intercept_position(
            chaser, chaser_speed=10.0, target=target, target_velocity=velocity
        )
        
        # Should be same as target position
        assert abs(intercept.x - target.x) < 0.01
        assert abs(intercept.y - target.y) < 0.01
    
    def test_intercept_moving_target(self):
        """Test intercept with moving target"""
        chaser = Position(0.0, 0.0)
        target = Position(100.0, 0.0)
        velocity = (5.0, 0.0)  # Moving east at 5 units/s
        
        intercept = calculate_intercept_position(
            chaser, chaser_speed=10.0, target=target, target_velocity=velocity
        )
        
        # Intercept should be ahead of current target position
        assert intercept.x > target.x


# =============================================================================
# PATH CLEAR TESTS
# =============================================================================

class TestPathClear:
    """Test path clearance checking"""
    
    def test_path_clear_no_obstacles(self):
        """Test clear path with no obstacles"""
        start = Position(0.0, 0.0)
        end = Position(100.0, 0.0)
        
        assert is_path_clear(start, end, [])
    
    def test_path_blocked(self):
        """Test blocked path"""
        start = Position(0.0, 0.0)
        end = Position(100.0, 0.0)
        obstacles = [Position(50.0, 0.0)]  # Directly in path
        
        assert not is_path_clear(start, end, obstacles, obstacle_radius=5.0)
    
    def test_path_clear_offset_obstacles(self):
        """Test clear path with offset obstacles"""
        start = Position(0.0, 0.0)
        end = Position(100.0, 0.0)
        obstacles = [Position(50.0, 20.0)]  # Off to the side
        
        assert is_path_clear(start, end, obstacles, obstacle_radius=5.0)
    
    def test_path_same_position(self):
        """Test path where start == end"""
        pos = Position(50.0, 50.0)
        obstacles = [Position(50.0, 50.0)]
        
        # Zero-length path should be clear
        assert is_path_clear(pos, pos, obstacles)


# =============================================================================
# THREAD SAFETY TESTS
# =============================================================================

class TestThreadSafety:
    """Test thread safety of movement operations"""
    
    @pytest.mark.concurrent
    def test_concurrent_position_operations(self):
        """Test concurrent position calculations"""
        positions = [Position(float(i), float(i)) for i in range(100)]
        target = Position(500.0, 500.0)
        
        def calculate_distances(pos):
            return pos.distance_to(target)
        
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(calculate_distances, pos) for pos in positions]
            results = [f.result() for f in as_completed(futures)]
        
        assert len(results) == 100
    
    @pytest.mark.concurrent
    def test_concurrent_action_generation(self):
        """Test concurrent action generation"""
        def generate_action(i):
            origin = Position(float(i), float(i))
            target = Position(float(i + 100), float(i + 100))
            return generate_movement_action_data(
                f"npc-{i}", origin, target, MovementType.WALK
            )
        
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(generate_action, i) for i in range(100)]
            results = [f.result() for f in as_completed(futures)]
        
        assert len(results) == 100
        assert all(r['action'] == 'move' for r in results)


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    def test_negative_coordinates(self):
        """Test handling of negative coordinates"""
        pos = Position(-50.0, -50.0)
        target = Position(50.0, 50.0)
        
        distance = pos.distance_to(target)
        assert distance > 0
    
    def test_very_large_coordinates(self):
        """Test very large coordinate values"""
        pos = Position(1e10, 1e10)
        target = Position(1e10 + 100, 1e10 + 100)
        
        distance = pos.distance_to(target)
        expected = math.sqrt(2 * 100**2)
        assert abs(distance - expected) < 1.0
    
    def test_zero_speed(self):
        """Test movement with zero speed"""
        action = generate_movement_action_data(
            "npc-001",
            Position(0.0, 0.0),
            Position(100.0, 0.0),
            MovementType.TELEPORT  # Speed 0
        )
        
        assert action['duration_seconds'] == 0.0
    
    def test_very_small_distance(self):
        """Test movement with very small distance"""
        start = Position(0.0, 0.0)
        end = Position(0.001, 0.001)
        
        distance = start.distance_to(end)
        assert distance > 0
        assert distance < 0.01