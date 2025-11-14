"""
Metrics and Monitoring for Emergent AI World

Tracks and exposes metrics for autonomy, emergence, relationship complexity, volatility, player engagement, and more.
Intended for use in dashboards, CI, and scenario/E2E test reporting.
"""

from typing import List, Dict, Any
from datetime import datetime
from loguru import logger

class MetricsCollector:
    def __init__(self):
        self.metrics = {
            "npc_autonomy": [],
            "emergent_events": [],
            "relationship_complexity": [],
            "economic_volatility": [],
            "player_engagement": [],
            "qualitative_stories": [],
            "timestamp": []
        }

    def record_npc_autonomy(self, percent_autonomous: float):
        self.metrics["npc_autonomy"].append(percent_autonomous)
        logger.info(f"Metrics: NPC autonomy {percent_autonomous:.2f}%")

    def record_emergent_event(self, event: Dict[str, Any]):
        self.metrics["emergent_events"].append(event)
        logger.info(f"Metrics: Emergent event recorded: {event.get('description', '')}")

    def record_relationship_complexity(self, avg_relationships: float):
        self.metrics["relationship_complexity"].append(avg_relationships)
        logger.info(f"Metrics: Avg relationships per NPC: {avg_relationships:.2f}")

    def record_economic_volatility(self, volatility: float):
        self.metrics["economic_volatility"].append(volatility)
        logger.info(f"Metrics: Economic volatility: {volatility:.2f}")

    def record_player_engagement(self, engagement: float):
        self.metrics["player_engagement"].append(engagement)
        logger.info(f"Metrics: Player engagement: {engagement:.2f}")

    def record_qualitative_story(self, story: str):
        self.metrics["qualitative_stories"].append(story)
        logger.info(f"Metrics: Player story: {story[:60]}...")

    def snapshot(self):
        self.metrics["timestamp"].append(datetime.utcnow().isoformat())
        logger.info("Metrics: Snapshot taken.")

    def get_metrics(self) -> Dict[str, Any]:
        return self.metrics

    def reset(self):
        for k in self.metrics:
            self.metrics[k] = []
        logger.info("Metrics: All metrics reset.")

# Example usage:
if __name__ == "__main__":
    metrics = MetricsCollector()
    metrics.record_npc_autonomy(92.5)
    metrics.record_emergent_event({"description": "Merchant alliance formed to counter hoarding."})
    metrics.record_relationship_complexity(7.2)
    metrics.record_economic_volatility(0.18)
    metrics.record_player_engagement(0.65)
    metrics.record_qualitative_story("Player helped broker peace between goblins and humans.")
    metrics.snapshot()
    print(metrics.get_metrics())