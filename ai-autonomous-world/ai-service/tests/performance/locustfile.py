"""
Locust Performance Testing for AI Autonomous World Service

Usage:
    # Run with web UI
    locust -f tests/performance/locustfile.py --host=http://localhost:8000

    # Run headless with 100 users
    locust -f tests/performance/locustfile.py --host=http://localhost:8000 \
           --users 100 --spawn-rate 10 --run-time 5m --headless

    # Run specific test class
    locust -f tests/performance/locustfile.py NPCInteractionTest --host=http://localhost:8000
"""

import json
import random
import time
from locust import HttpUser, task, between, events
from locust.contrib.fasthttp import FastHttpUser


class NPCInteractionTest(FastHttpUser):
    """Test NPC interaction endpoints performance"""
    
    wait_time = between(1, 3)  # Wait 1-3 seconds between tasks
    
    def on_start(self):
        """Initialize test data"""
        self.npc_ids = [f"npc_{i:03d}" for i in range(1, 101)]  # 100 NPCs
        self.player_ids = [f"player_{i:04d}" for i in range(1, 1001)]  # 1000 players
        
        # Register a test NPC
        self.register_npc()
    
    def register_npc(self):
        """Register a test NPC"""
        npc_id = random.choice(self.npc_ids)
        payload = {
            "npc_id": npc_id,
            "name": f"Test NPC {npc_id}",
            "npc_class": "merchant",
            "level": random.randint(1, 99),
            "position": {
                "map": "prontera",
                "x": random.randint(100, 200),
                "y": random.randint(100, 200)
            },
            "personality": {
                "openness": random.random(),
                "conscientiousness": random.random(),
                "extraversion": random.random(),
                "agreeableness": random.random(),
                "neuroticism": random.random(),
                "moral_alignment": random.choice(["lawful_good", "neutral", "chaotic_evil"])
            }
        }
        
        with self.client.post(
            "/ai/npc/register",
            json=payload,
            catch_response=True,
            name="/ai/npc/register"
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Failed to register NPC: {response.status_code}")
    
    @task(5)
    def player_interaction(self):
        """Test player-NPC interaction (most common operation)"""
        npc_id = random.choice(self.npc_ids)
        player_id = random.choice(self.player_ids)
        
        payload = {
            "npc_id": npc_id,
            "player_id": player_id,
            "player_name": f"Player_{player_id}",
            "interaction_type": random.choice(["talk", "trade", "quest"]),
            "message": "Hello, how are you?",
            "context": {
                "location": {"map": "prontera", "x": 150, "y": 180},
                "time": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "weather": random.choice(["sunny", "rainy", "cloudy"])
            }
        }
        
        with self.client.post(
            "/ai/player/interaction",
            json=payload,
            catch_response=True,
            name="/ai/player/interaction"
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Interaction failed: {response.status_code}")
    
    @task(2)
    def get_npc_state(self):
        """Test NPC state retrieval"""
        npc_id = random.choice(self.npc_ids)
        
        with self.client.get(
            f"/ai/npc/{npc_id}/state",
            catch_response=True,
            name="/ai/npc/{id}/state"
        ) as response:
            if response.status_code in [200, 404]:
                response.success()
            else:
                response.failure(f"Get state failed: {response.status_code}")
    
    @task(1)
    def get_world_state(self):
        """Test world state retrieval"""
        with self.client.get(
            "/ai/world/state",
            catch_response=True,
            name="/ai/world/state"
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Get world state failed: {response.status_code}")
    
    @task(1)
    def get_environment_state(self):
        """Test environment state retrieval"""
        with self.client.get(
            "/ai/world/environment/current",
            catch_response=True,
            name="/ai/world/environment/current"
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Get environment failed: {response.status_code}")


class ChatCommandTest(FastHttpUser):
    """Test chat command performance"""
    
    wait_time = between(2, 5)
    
    def on_start(self):
        self.npc_ids = [f"npc_{i:03d}" for i in range(1, 101)]
        self.player_ids = [f"player_{i:04d}" for i in range(1, 1001)]
        self.messages = [
            "Tell me about the ancient ruins",
            "What quests do you have?",
            "Can you help me find the blacksmith?",
            "What's the weather like today?",
            "Do you know any good hunting spots?"
        ]
    
    @task
    def chat_command(self):
        """Test chat command endpoint"""
        payload = {
            "npc_id": random.choice(self.npc_ids),
            "player_id": random.choice(self.player_ids),
            "player_name": f"Player_{random.choice(self.player_ids)}",
            "message": random.choice(self.messages),
            "context": {
                "location": {"map": "prontera", "x": 150, "y": 180}
            }
        }
        
        self.client.post("/ai/chat/command", json=payload, name="/ai/chat/command")

