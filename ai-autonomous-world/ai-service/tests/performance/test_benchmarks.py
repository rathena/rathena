"""
Performance Benchmarks for AI Autonomous World Service

Run with:
    pytest tests/performance/test_benchmarks.py -v --benchmark-only
    pytest tests/performance/test_benchmarks.py -v --benchmark-compare
"""

import pytest
import asyncio
import time
from unittest.mock import AsyncMock, MagicMock, patch


class TestNPCPerformance:
    """Benchmark NPC-related operations"""
    
    @pytest.mark.benchmark(group="npc_registration")
    def test_npc_registration_performance(self, benchmark):
        """Benchmark NPC registration"""
        from models.npc import NPCPersonality, NPCPosition, NPCRegisterRequest

        def register_npc():
            personality = NPCPersonality(
                openness=0.7,
                conscientiousness=0.8,
                extraversion=0.6,
                agreeableness=0.9,
                neuroticism=0.3,
                moral_alignment="lawful_good"
            )

            position = NPCPosition(
                map="prontera",
                x=150,
                y=180
            )

            request = NPCRegisterRequest(
                npc_id="test_npc_001",
                name="Test NPC",
                npc_class="merchant",
                level=50,
                position=position,
                personality=personality
            )
            return request

        result = benchmark(register_npc)
        assert result is not None
    
    @pytest.mark.benchmark(group="npc_state_update")
    def test_npc_state_update_performance(self, benchmark):
        """Benchmark NPC state updates"""
        from models.npc import NPCPersonality
        
        personality = NPCPersonality(
            openness=0.7,
            conscientiousness=0.8,
            extraversion=0.6,
            agreeableness=0.9,
            neuroticism=0.3,
            moral_alignment="lawful_good"
        )

        # Use dict to represent NPC state
        state = {
            "npc_id": "test_npc_001",
            "name": "Test NPC",
            "npc_class": "merchant",
            "level": 50,
            "position": {"map": "prontera", "x": 150, "y": 180},
            "personality": personality.dict()
        }

        def update_state():
            state["current_action"] = "idle"
            state["current_dialogue"] = "Hello, traveler!"
            state["emotional_state"] = {"happiness": 0.8, "anger": 0.1}
            return state

        result = benchmark(update_state)
        assert result["current_action"] == "idle"


class TestEnvironmentPerformance:
    """Benchmark environment system operations"""
    
    @pytest.mark.benchmark(group="environment_update")
    def test_weather_update_performance(self, benchmark):
        """Benchmark weather update"""
        from tasks.environment import EnvironmentManager

        manager = EnvironmentManager()
        
        def update_weather():
            # Simulate weather update
            current_weather = "sunny"
            weather_types = ["clear", "sunny", "cloudy", "rainy", "stormy"]
            weights = [0.3, 0.5, 0.15, 0.03, 0.02]
            
            import random
            new_weather = random.choices(weather_types, weights=weights, k=1)[0]
            return new_weather
        
        result = benchmark(update_weather)
        assert result in ["clear", "sunny", "cloudy", "rainy", "stormy"]
    
    @pytest.mark.benchmark(group="environment_update")
    def test_time_of_day_update_performance(self, benchmark):
        """Benchmark time of day update"""
        from datetime import datetime, timedelta
        
        def update_time_of_day():
            current_time = datetime.utcnow()
            cycle_duration = 60  # minutes
            elapsed_minutes = (current_time.hour * 60 + current_time.minute) % cycle_duration
            
            if elapsed_minutes < 15:
                phase = "dawn"
            elif elapsed_minutes < 45:
                phase = "day"
            elif elapsed_minutes < 55:
                phase = "dusk"
            else:
                phase = "night"
            
            return phase
        
        result = benchmark(update_time_of_day)
        assert result in ["dawn", "day", "dusk", "night"]


class TestDatabasePerformance:
    """Benchmark database operations"""
    
    @pytest.mark.benchmark(group="database_operations")
    @pytest.mark.asyncio
    async def test_redis_set_performance(self, benchmark, mock_database):
        """Benchmark Redis SET operation"""
        import json

        async def redis_set():
            key = "test:performance:key"
            value = json.dumps({"test": "data", "timestamp": time.time()})
            await mock_database.set(key, value)
            return True

        # Run benchmark
        result = await asyncio.create_task(asyncio.to_thread(benchmark, lambda: asyncio.run(redis_set())))
        assert result is True
    
    @pytest.mark.benchmark(group="database_operations")
    @pytest.mark.asyncio
    async def test_redis_get_performance(self, benchmark, mock_database):
        """Benchmark Redis GET operation"""
        import json

        # Setup
        key = "test:performance:key"
        value = json.dumps({"test": "data", "timestamp": time.time()})
        await mock_database.set(key, value)

        async def redis_get():
            result = await mock_database.get(key)
            return result

        # Run benchmark
        result = await asyncio.create_task(asyncio.to_thread(benchmark, lambda: asyncio.run(redis_get())))
        assert result is not None


class TestConfigurationPerformance:
    """Benchmark configuration loading"""
    
    @pytest.mark.benchmark(group="configuration")
    def test_config_load_performance(self, benchmark):
        """Benchmark configuration loading"""
        from config import Settings
        
        def load_config():
            settings = Settings()
            return settings
        
        result = benchmark(load_config)
        assert result is not None
        assert result.service_name == "ai-service"

