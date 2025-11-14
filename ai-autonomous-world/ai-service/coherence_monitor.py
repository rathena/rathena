"""
Coherence Monitor and Feedback Loop System

Implements soft constraints, feedback loops, contradiction detection, narrative smoothing, retcon prevention,
and player influence scaling for the AI Autonomous World. This module is designed to be called periodically
(e.g., every N seconds or after major world events) to ensure world consistency and self-regulation.
"""

from typing import List, Dict, Any
from datetime import datetime
from loguru import logger

class CoherenceMonitor:
    def __init__(self):
        self.last_check = datetime.utcnow()
        self.inconsistencies: List[Dict[str, Any]] = []
        self.retcon_attempts: int = 0

    def check_world_state(self, world_state: Dict[str, Any], event_log: List[Dict[str, Any]]) -> List[str]:
        """
        Checks the world state and event log for contradictions, impossible states, or narrative breaks.
        Returns a list of detected issues.
        """
        issues = []
        # Example: Contradiction detection
        if world_state.get("economy", {}).get("trade_volume", 0) < 0:
            issues.append("Negative trade volume detected (impossible state).")
        if world_state.get("politics", {}).get("conflict_level", 0) < 0:
            issues.append("Negative conflict level detected (impossible state).")
        # Example: Narrative smoothing
        for event in event_log[-10:]:
            if "resurrected" in event.get("description", "").lower() and not event.get("allow_resurrection", False):
                issues.append(f"Retcon prevention: Resurrection event without permission: {event}")
        # Example: Player influence scaling
        for event in event_log[-10:]:
            if event.get("type") == "player_action" and event.get("impact", 0) > 1000:
                issues.append(f"Player action impact too high: {event}")
        # Example: Delayed/Unintended consequences
        for event in event_log[-10:]:
            if event.get("type") == "policy_change" and "riot" not in [e.get("type") for e in event_log[-20:]]:
                issues.append("Policy change without social consequence (missing feedback loop).")
        self.inconsistencies.extend(issues)
        logger.info(f"CoherenceMonitor: {len(issues)} issues detected at {datetime.utcnow().isoformat()}")
        return issues

    def apply_soft_constraints(self, npc_states: List[Dict[str, Any]]) -> None:
        """
        Applies soft constraints to NPCs to prevent extreme or impossible behavior.
        """
        for npc in npc_states:
            # Example: Personality consistency
            if abs(npc.get("personality", {}).get("openness", 0.5) - npc.get("personality", {}).get("conscientiousness", 0.5)) > 0.9:
                npc["personality"]["openness"] = 0.5  # Smooth out extreme difference
            # Example: Social pressure
            if npc.get("reputation", 0) < -900 and npc.get("npc_class") != "criminal":
                npc["reputation"] = max(npc["reputation"], -900)
            # Example: Physical limits
            if npc.get("energy", 100) > 200:
                npc["energy"] = 200
        logger.info("CoherenceMonitor: Soft constraints applied to NPC states.")

    def feedback_loop(self, world_state: Dict[str, Any], event_log: List[Dict[str, Any]]) -> None:
        """
        Implements negative and positive feedback loops to stabilize or reinforce world trends.
        """
        # Negative feedback: If inflation is too high, trigger economic stabilization event
        if world_state.get("economy", {}).get("inflation_rate", 0) > 0.5:
            logger.info("CoherenceMonitor: Triggering economic stabilization due to high inflation.")
            event_log.append({
                "type": "system_event",
                "description": "Central bank intervention to stabilize currency.",
                "timestamp": datetime.utcnow().isoformat()
            })
            world_state["economy"]["inflation_rate"] *= 0.8
        # Positive feedback: If cooperation is high, increase trust
        if world_state.get("social", {}).get("cooperation_level", 0) > 0.8:
            logger.info("CoherenceMonitor: Reinforcing cooperation trend.")
            world_state["social"]["trust"] = min(1.0, world_state["social"].get("trust", 0.5) + 0.05)

    def monitor_and_regulate(self, world_state: Dict[str, Any], npc_states: List[Dict[str, Any]], event_log: List[Dict[str, Any]]) -> None:
        """
        Main entry point: checks coherence, applies constraints, and runs feedback loops.
        """
        issues = self.check_world_state(world_state, event_log)
        self.apply_soft_constraints(npc_states)
        self.feedback_loop(world_state, event_log)
        logger.info(f"CoherenceMonitor: Monitoring complete. Issues: {issues}")

# Example usage:
if __name__ == "__main__":
    monitor = CoherenceMonitor()
    world_state = {
        "economy": {"trade_volume": 100000, "inflation_rate": 0.6},
        "politics": {"conflict_level": 0.1},
        "social": {"cooperation_level": 0.9, "trust": 0.7}
    }
    npc_states = [
        {"npc_id": "npc_001", "personality": {"openness": 1.0, "conscientiousness": 0.0}, "reputation": -1000, "npc_class": "merchant", "energy": 250},
        {"npc_id": "npc_002", "personality": {"openness": 0.5, "conscientiousness": 0.5}, "reputation": 100, "npc_class": "guard", "energy": 80}
    ]
    event_log = [
        {"type": "player_action", "impact": 2000, "description": "Player destroyed the bank."},
        {"type": "policy_change", "description": "King raised taxes."},
        {"type": "system_event", "description": "Resurrected NPC without permission.", "allow_resurrection": False}
    ]
    monitor.monitor_and_regulate(world_state, npc_states, event_log)
    print("Coherence monitoring complete.")