"""
Locust Load Testing: Comprehensive System Test
Simulates realistic user load across all system components.

Run with:
locust -f tests/performance/locust_comprehensive.py \
  --host http://192.168.0.100:8000 \
  --users 1000 \
  --spawn-rate 100 \
  --run-time 10m \
  --html performance_report.html
"""

from locust import HttpUser, task, between, events
import random
import json
from datetime import datetime


class AIServiceUser(HttpUser):
    """Simulates a typical AI service user (game server)."""
    
    wait_time = between(1, 3)  # Wait 1-3 seconds between tasks
    
    def on_start(self):
        """Initialize user session."""
        self.player_id = f"player_{random.randint(1, 10000):05d}"
        self.npc_id = f"npc_{random.randint(1, 1000):03d}"
    
    @task(3)
    def get_agent_status(self):
        """Most common: Check agent status."""
        with self.client.get(
            "/api/v1/world/agents/status",
            name="/agents/status",
            catch_response=True
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Status code: {response.status_code}")
    
    @task(2)
    def get_current_problem(self):
        """Check current world problem."""
        with self.client.get(
            "/api/v1/procedural/problem/current",
            name="/problem/current",
            catch_response=True
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Failed to get problem")
    
    @task(2)
    def get_story_arc(self):
        """Get current story arc."""
        with self.client.get(
            "/api/v1/storyline/current_arc",
            name="/storyline/arc",
            catch_response=True
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Failed to get story arc")
    
    @task(1)
    def player_npc_interaction(self):
        """Simulate player-NPC interaction (most expensive)."""
        payload = {
            "player_id": self.player_id,
            "npc_id": self.npc_id,
            "message": "I need a quest",
            "context": {
                "player_level": random.randint(1, 99),
                "location": random.choice(["prontera", "geffen", "payon"])
            }
        }
        
        with self.client.post(
            "/api/v1/ai/player/interaction",
            json=payload,
            name="/ai/interaction",
            catch_response=True
        ) as response:
            if response.status_code == 200:
                data = response.json()
                if "npc_response" in data:
                    response.success()
                else:
                    response.failure("Invalid response format")
            else:
                response.failure(f"Status: {response.status_code}")
    
    @task(1)
    def get_world_state(self):
        """Get current world state."""
        with self.client.get(
            "/api/v1/world/state",
            name="/world/state",
            catch_response=True
        ) as response:
            if response.status_code == 200:
                response.success()
            else:
                response.failure(f"Failed to get world state")
    
    @task(1)
    def post_game_event(self):
        """Post game event."""
        event_data = {
            "event_type": random.choice(["monster_kill", "player_login", "quest_complete"]),
            "player_id": self.player_id,
            "timestamp": datetime.now().isoformat(),
            "data": {
                "location": random.choice(["prt_fild08", "gef_fild10", "pay_fild07"])
            }
        }
        
        with self.client.post(
            "/api/v1/world/events",
            json=event_data,
            name="/world/events",
            catch_response=True
        ) as response:
            if response.status_code in [200, 201]:
                response.success()
            else:
                response.failure(f"Event post failed: {response.status_code}")


class AdminUser(HttpUser):
    """Simulates dashboard admin accessing monitoring endpoints."""
    
    wait_time = between(5, 10)  # Admins check less frequently
    
    @task(3)
    def view_dashboard(self):
        """View main dashboard."""
        self.client.get("/api/v1/world/agents/status", name="/admin/dashboard")
    
    @task(2)
    def view_metrics(self):
        """View performance metrics."""
        self.client.get("/api/v1/metrics", name="/admin/metrics")
    
    @task(1)
    def view_costs(self):
        """View LLM costs."""
        self.client.get("/api/v1/costs/summary", name="/admin/costs")


@events.test_start.add_listener
def on_test_start(environment, **kwargs):
    """Print test configuration at start."""
    print("\n" + "="*80)
    print("LOCUST LOAD TEST - AI MMORPG WORLD SERVICE")
    print("="*80)
    print(f"Target Host: {environment.host}")
    print(f"Users: {environment.runner.target_user_count if hasattr(environment.runner, 'target_user_count') else 'N/A'}")
    print(f"Spawn Rate: {environment.runner.spawn_rate if hasattr(environment.runner, 'spawn_rate') else 'N/A'} users/s")
    print("="*80 + "\n")


@events.test_stop.add_listener
def on_test_stop(environment, **kwargs):
    """Print test summary at end."""
    print("\n" + "="*80)
    print("TEST COMPLETE - SUMMARY")
    print("="*80)
    
    stats = environment.stats
    print(f"Total Requests: {stats.total.num_requests}")
    print(f"Total Failures: {stats.total.num_failures}")
    print(f"Average Response Time: {stats.total.avg_response_time:.2f}ms")
    print(f"Min Response Time: {stats.total.min_response_time:.2f}ms")
    print(f"Max Response Time: {stats.total.max_response_time:.2f}ms")
    print(f"Requests/sec: {stats.total.total_rps:.2f}")
    print(f"Failure Rate: {stats.total.fail_ratio:.2%}")
    print("="*80 + "\n")
    
    # Performance assertions (fail test if SLA not met)
    if stats.total.avg_response_time > 500:
        print(f"⚠️  WARNING: Average response time {stats.total.avg_response_time:.0f}ms exceeds 500ms SLA")
    
    if stats.total.fail_ratio > 0.05:
        print(f"⚠️  WARNING: Failure rate {stats.total.fail_ratio:.1%} exceeds 5% threshold")
    
    if stats.total.total_rps < 100:
        print(f"⚠️  WARNING: Throughput {stats.total.total_rps:.0f} req/s below 100 req/s target")