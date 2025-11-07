"""
Load tests for AI Autonomous World Service using Locust

Run with: locust -f tests/locustfile.py --host=http://localhost:8000
"""

from locust import HttpUser, task, between, events
import random
import json
import logging

logger = logging.getLogger(__name__)


class AIServiceUser(HttpUser):
    """Simulates a user interacting with the AI service"""
    
    # Wait 1-3 seconds between tasks
    wait_time = between(1, 3)
    
    def on_start(self):
        """Initialize user session"""
        self.player_id = f"player_{random.randint(1000, 9999)}"
        self.npc_ids = [f"npc_{i:03d}" for i in range(1, 11)]  # 10 NPCs
        
        # Register a test NPC
        self.register_test_npc()
    
    def register_test_npc(self):
        """Register a test NPC for this user"""
        npc_data = {
            "npc_id": f"test_npc_{self.player_id}",
            "name": f"Test Merchant {self.player_id}",
            "level": random.randint(1, 99),
            "npc_class": random.choice(["merchant", "guard", "priest", "blacksmith"]),
            "position": {
                "map": "prontera",
                "x": random.randint(100, 200),
                "y": random.randint(100, 200)
            },
            "personality": {
                "traits": ["friendly", "helpful"],
                "background": "A helpful NPC",
                "goals": ["Assist players"],
                "speech_style": "Polite"
            }
        }
        
        try:
            response = self.client.post("/ai/npc/register", json=npc_data)
            if response.status_code == 200:
                logger.info(f"Registered NPC: {npc_data['npc_id']}")
        except Exception as e:
            logger.error(f"Failed to register NPC: {e}")
    
    @task(5)
    def health_check(self):
        """Test lightweight health check endpoint"""
        self.client.get("/health", name="/health")
    
    @task(2)
    def detailed_health_check(self):
        """Test detailed health check endpoint"""
        self.client.get("/health/detailed", name="/health/detailed")
    
    @task(10)
    def player_interaction(self):
        """Test player interaction with NPC"""
        npc_id = random.choice(self.npc_ids)
        
        interaction_data = {
            "player_id": self.player_id,
            "player_name": f"Hero{self.player_id[-4:]}",
            "player_level": random.randint(1, 99),
            "npc_id": npc_id,
            "interaction_type": random.choice(["talk", "trade", "quest"]),
            "message": random.choice([
                "Hello!",
                "What do you have for sale?",
                "Any quests available?",
                "Tell me about yourself",
                "Goodbye"
            ]),
            "context": {
                "location": {
                    "map": "prontera",
                    "x": random.randint(100, 200),
                    "y": random.randint(100, 200)
                },
                "time_of_day": random.choice(["morning", "afternoon", "evening", "night"]),
                "weather": random.choice(["sunny", "cloudy", "rainy"])
            }
        }
        
        with self.client.post(
            "/ai/player/interact",
            json=interaction_data,
            catch_response=True,
            name="/ai/player/interact"
        ) as response:
            if response.status_code == 200:
                response.success()
            elif response.status_code == 404:
                # NPC not found is acceptable in load test
                response.success()
            else:
                response.failure(f"Got status code {response.status_code}")
    
    @task(8)
    def chat_command(self):
        """Test chat command interface"""
        npc_id = random.choice(self.npc_ids)
        
        command_data = {
            "player_id": self.player_id,
            "npc_id": npc_id,
            "message": random.choice([
                "Hi there!",
                "How are you?",
                "What's new?",
                "Can you help me?",
                "Thanks!"
            ]),
            "player_name": f"Hero{self.player_id[-4:]}",
            "player_level": random.randint(1, 99),
            "player_position": {
                "map": "prontera",
                "x": random.randint(100, 200),
                "y": random.randint(100, 200)
            }
        }
        
        with self.client.post(
            "/ai/chat/command",
            json=command_data,
            catch_response=True,
            name="/ai/chat/command"
        ) as response:
            if response.status_code == 200:
                response.success()
            elif response.status_code == 429:
                # Rate limited is expected
                response.success()
            elif response.status_code == 404:
                # NPC not found is acceptable
                response.success()
            else:
                response.failure(f"Got status code {response.status_code}")
    
    @task(3)
    def npc_event(self):
        """Test NPC event processing"""
        npc_id = random.choice(self.npc_ids)
        
        event_data = {
            "event_type": random.choice(["player_nearby", "time_change", "weather_change"]),
            "event_data": {
                "player_count": random.randint(1, 10),
                "time": random.choice(["morning", "afternoon", "evening"]),
                "weather": random.choice(["sunny", "rainy"])
            }
        }
        
        with self.client.post(
            f"/ai/npc/{npc_id}/event",
            json=event_data,
            catch_response=True,
            name="/ai/npc/{npc_id}/event"
        ) as response:
            if response.status_code in [200, 404]:
                response.success()
            else:
                response.failure(f"Got status code {response.status_code}")


class HighLoadUser(HttpUser):
    """Simulates high-load scenarios"""
    
    wait_time = between(0.1, 0.5)  # Faster requests
    
    def on_start(self):
        self.player_id = f"load_player_{random.randint(10000, 99999)}"
    
    @task
    def rapid_health_checks(self):
        """Rapid health check requests"""
        self.client.get("/health", name="/health [high-load]")
    
    @task
    def rapid_interactions(self):
        """Rapid interaction requests"""
        interaction_data = {
            "player_id": self.player_id,
            "player_name": "LoadTester",
            "player_level": 50,
            "npc_id": f"npc_{random.randint(1, 10):03d}",
            "interaction_type": "talk",
            "message": "Quick test",
            "context": {"location": {"map": "prontera", "x": 150, "y": 150}}
        }
        
        self.client.post(
            "/ai/player/interact",
            json=interaction_data,
            name="/ai/player/interact [high-load]"
        )


@events.test_start.add_listener
def on_test_start(environment, **kwargs):
    """Called when load test starts"""
    logger.info("=" * 80)
    logger.info("LOAD TEST STARTED")
    logger.info("=" * 80)


@events.test_stop.add_listener
def on_test_stop(environment, **kwargs):
    """Called when load test stops"""
    logger.info("=" * 80)
    logger.info("LOAD TEST COMPLETED")
    logger.info("=" * 80)

