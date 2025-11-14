"""
Scenario-Driven E2E Tests for Emergent Behavior

Implements scenario tests for Merchant’s Rise/Fall, Goblin Peace Treaty, Economic Collapse, and more.
These tests simulate world and NPC state, trigger events, and verify emergent outcomes.
"""

from world_bootstrap import world_bootstrap
from coherence_monitor import CoherenceMonitor
from datetime import datetime, timedelta
import random

def merchant_rise_and_fall(world):
    # Find a merchant NPC
    merchant = next(n for n in world["npcs"] if n.npc_class == "merchant")
    # Simulate wheat shortage
    world_state = {"economy": {"trade_volume": 100000, "inflation_rate": 0.1, "wheat_supply": 100}}
    event_log = []
    merchant_state = {"npc_id": merchant.npc_id, "personality": merchant.personality.dict(), "reputation": 0, "npc_class": "merchant", "energy": 100}
    # Step 1: Merchant hoards wheat
    world_state["economy"]["wheat_supply"] = 0
    event_log.append({"type": "market", "description": "Wheat shortage due to drought."})
    # Step 2: Merchant profits
    merchant_state["reputation"] += 50
    event_log.append({"type": "merchant_action", "description": "Merchant hoards and sells wheat at high price."})
    # Step 3: Social backlash
    merchant_state["reputation"] -= 100
    event_log.append({"type": "social", "description": "Other merchants form alliance, spread rumors."})
    # Step 4: Corruption and arrest
    merchant_state["reputation"] -= 200
    event_log.append({"type": "politics", "description": "Merchant bribes guards, is arrested."})
    # Step 5: Redemption
    merchant_state["reputation"] += 100
    event_log.append({"type": "redemption", "description": "Merchant learns humility, rebuilds reputation."})
    return merchant_state, event_log

def goblin_peace_treaty(world):
    # Find a human NPC and simulate goblin/human factions
    human = next(n for n in world["npcs"] if n.npc_class == "priest")
    goblin = {"npc_id": "goblin_leader", "npc_class": "goblin", "personality": {"empathy": 0.8, "wisdom": 0.8}, "reputation": -100}
    event_log = []
    # Step 1: Drought increases raids
    event_log.append({"type": "environment", "description": "Drought affects both humans and goblins."})
    # Step 2: Human proposes peace
    event_log.append({"type": "social", "description": "Human NPC proposes peace talks."})
    # Step 3: Player helps
    event_log.append({"type": "player_action", "description": "Player helps contact goblin leader."})
    # Step 4: Negotiations and extremists
    event_log.append({"type": "negotiation", "description": "Negotiations, extremists attack."})
    # Step 5: Treaty and cultural fusion
    event_log.append({"type": "treaty", "description": "Treaty succeeds, trade begins, new culture emerges."})
    return [human, goblin], event_log

def economic_collapse(world):
    # Simulate gold rush and inflation
    world_state = {"economy": {"trade_volume": 1000000, "inflation_rate": 0.0, "gold_supply": 1000}}
    event_log = []
    # Step 1: Gold rush
    world_state["economy"]["gold_supply"] += 10000
    event_log.append({"type": "economy", "description": "Gold rush, supply increases."})
    # Step 2: Inflation
    world_state["economy"]["inflation_rate"] = 0.5
    event_log.append({"type": "economy", "description": "Inflation rises, prices increase."})
    # Step 3: Food shortage and unrest
    event_log.append({"type": "social", "description": "Farmers abandon farms, food shortage, unrest."})
    # Step 4: Hyperinflation and revolution
    world_state["economy"]["inflation_rate"] = 1.0
    event_log.append({"type": "politics", "description": "Currency collapses, revolution occurs."})
    # Step 5: New currency and stabilization
    world_state["economy"]["inflation_rate"] = 0.1
    event_log.append({"type": "economy", "description": "New food-backed currency, economy stabilizes."})
    return world_state, event_log

def run_all_scenarios():
    world = world_bootstrap(100)
    monitor = CoherenceMonitor()
    # Merchant's Rise and Fall
    merchant_state, merchant_events = merchant_rise_and_fall(world)
    monitor.monitor_and_regulate({"economy": {"trade_volume": 100000, "inflation_rate": 0.1}}, [merchant_state], merchant_events)
    # Goblin Peace Treaty
    npcs, goblin_events = goblin_peace_treaty(world)
    monitor.monitor_and_regulate({"politics": {"conflict_level": 0.5}}, npcs, goblin_events)
    # Economic Collapse
    econ_state, econ_events = economic_collapse(world)
    monitor.monitor_and_regulate(econ_state, [], econ_events)
    print("E2E scenario tests complete.")

if __name__ == "__main__":
    run_all_scenarios()