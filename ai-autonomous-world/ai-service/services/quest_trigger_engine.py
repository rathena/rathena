"""
Quest trigger evaluation engine
Evaluates quest triggers to determine when quests should be activated
"""

from loguru import logger
from typing import Dict, Any, List, Optional
from datetime import datetime, time as datetime_time
import asyncio

try:
    from ai_service.models.quest_trigger import (
        QuestTrigger, TriggerType, WeatherCondition, Season
    )
    from ai_service.database import db
except ImportError:
    from models.quest_trigger import (
        QuestTrigger, TriggerType, WeatherCondition, Season
    )
    from database import db


class QuestTriggerEngine:
    """Engine for evaluating quest triggers"""
    
    def __init__(self):
        self.logger = logger.bind(component="QuestTriggerEngine")
    
    async def evaluate_trigger(
        self,
        trigger: QuestTrigger,
        context: Dict[str, Any]
    ) -> bool:
        """
        Evaluate if a trigger condition is met
        
        Args:
            trigger: The trigger to evaluate
            context: Context data including player, NPC, world state, etc.
        
        Returns:
            True if trigger condition is met
        """
        if not trigger.enabled:
            return False
        
        # Check cooldown
        if trigger.cooldown_seconds and trigger.last_triggered:
            elapsed = (datetime.utcnow() - trigger.last_triggered).total_seconds()
            if elapsed < trigger.cooldown_seconds:
                return False
        
        # Evaluate based on trigger type
        try:
            if trigger.trigger_type == TriggerType.REAL_TIME:
                return await self._evaluate_real_time(trigger, context)
            elif trigger.trigger_type == TriggerType.SCHEDULED:
                return await self._evaluate_scheduled(trigger, context)
            elif trigger.trigger_type == TriggerType.WEATHER:
                return await self._evaluate_weather(trigger, context)
            elif trigger.trigger_type == TriggerType.NPC_SPECIFIC:
                return await self._evaluate_npc_specific(trigger, context)
            elif trigger.trigger_type == TriggerType.LOCATION:
                return await self._evaluate_location(trigger, context)
            elif trigger.trigger_type == TriggerType.WORLD_STATE:
                return await self._evaluate_world_state(trigger, context)
            elif trigger.trigger_type == TriggerType.INTERNAL:
                return await self._evaluate_internal(trigger, context)
            elif trigger.trigger_type == TriggerType.EXTERNAL:
                return await self._evaluate_external(trigger, context)
            elif trigger.trigger_type == TriggerType.RELATIONSHIP:
                return await self._evaluate_relationship(trigger, context)
            elif trigger.trigger_type == TriggerType.GIFT:
                return await self._evaluate_gift(trigger, context)
            elif trigger.trigger_type == TriggerType.SEQUENCE:
                return await self._evaluate_sequence(trigger, context)
            else:
                self.logger.warning(f"Unknown trigger type: {trigger.trigger_type}")
                return False
                
        except Exception as e:
            self.logger.error(f"Error evaluating trigger {trigger.trigger_id}: {e}", exc_info=True)
            return False
    
    async def _evaluate_real_time(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate real-time trigger"""
        if not trigger.real_time:
            return False
        
        now = datetime.utcnow()
        rt = trigger.real_time
        
        # Check date range
        if rt.start_date and now < rt.start_date:
            return False
        if rt.end_date and now > rt.end_date:
            return False
        
        # Check day of week
        if rt.days_of_week and now.weekday() not in rt.days_of_week:
            return False
        
        # Check hour
        if rt.hours and now.hour not in rt.hours:
            return False
        
        return True
    
    async def _evaluate_scheduled(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate scheduled (in-game time) trigger"""
        if not trigger.scheduled:
            return False
        
        # Get in-game time from context
        game_time = context.get("game_time")
        if not game_time:
            return True  # If no game time system, allow trigger
        
        st = trigger.scheduled
        
        # Check game time range
        if st.game_time_start and st.game_time_end:
            current_time = game_time.get("time")
            if current_time:
                if not (st.game_time_start <= current_time <= st.game_time_end):
                    return False
        
        # Check game day
        if st.game_day:
            current_day = game_time.get("day")
            if current_day and current_day != st.game_day:
                return False
        
        # Check season
        if st.season:
            current_season = game_time.get("season")
            if current_season and current_season != st.season.value:
                return False
        
        return True
    
    async def _evaluate_weather(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate weather trigger"""
        if not trigger.weather:
            return False
        
        current_weather = context.get("weather", {})
        wt = trigger.weather
        
        # Check weather condition
        if wt.required_weather != WeatherCondition.ANY:
            weather_type = current_weather.get("type", "clear")
            if weather_type != wt.required_weather.value:
                return False
        
        # Check map
        if wt.map_name:
            current_map = context.get("map_name")
            if current_map != wt.map_name:
                return False

        return True

    async def _evaluate_npc_specific(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate NPC-specific trigger"""
        if not trigger.npc_specific:
            return False

        npc_state = context.get("npc_state", {})
        nt = trigger.npc_specific

        # Check mood
        if nt.npc_mood:
            current_mood = npc_state.get("mood")
            if current_mood != nt.npc_mood:
                return False

        # Check health
        if nt.npc_health_below is not None:
            health = npc_state.get("health", 1.0)
            if health >= nt.npc_health_below:
                return False

        # Check busy status
        if nt.npc_busy is not None:
            is_busy = npc_state.get("busy", False)
            if is_busy != nt.npc_busy:
                return False

        # Check location
        if nt.npc_location:
            location = npc_state.get("location")
            if location != nt.npc_location:
                return False

        return True

    async def _evaluate_location(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate location trigger"""
        if not trigger.location:
            return False

        player_pos = context.get("player_position", {})
        npc_pos = context.get("npc_position", {})
        lt = trigger.location

        # Check map
        player_map = player_pos.get("map")
        if player_map != lt.map_name:
            return False

        # Check coordinates
        player_x = player_pos.get("x", 0)
        player_y = player_pos.get("y", 0)

        # Check bounds
        if lt.x_min is not None and player_x < lt.x_min:
            return False
        if lt.x_max is not None and player_x > lt.x_max:
            return False
        if lt.y_min is not None and player_y < lt.y_min:
            return False
        if lt.y_max is not None and player_y > lt.y_max:
            return False

        # Check radius from NPC
        if lt.radius is not None:
            npc_x = npc_pos.get("x", 0)
            npc_y = npc_pos.get("y", 0)
            distance = ((player_x - npc_x) ** 2 + (player_y - npc_y) ** 2) ** 0.5
            if distance > lt.radius:
                return False

        return True

    async def _evaluate_world_state(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate world state trigger"""
        if not trigger.world_state:
            return False

        world_state = context.get("world_state", {})
        wst = trigger.world_state

        # Check event
        if wst.event_name:
            active_events = world_state.get("active_events", [])
            if wst.event_name not in active_events:
                return False

        # Check faction war
        if wst.faction_war is not None:
            is_war = world_state.get("faction_war", False)
            if is_war != wst.faction_war:
                return False

        # Check economy
        if wst.economy_state:
            economy = world_state.get("economy_state")
            if economy != wst.economy_state:
                return False

        # Check global variables
        if wst.global_variable:
            for key, expected_value in wst.global_variable.items():
                actual_value = world_state.get(key)
                if actual_value != expected_value:
                    return False

        return True

    async def _evaluate_internal(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate internal (NPC mental state) trigger"""
        if not trigger.internal:
            return False

        npc_state = context.get("npc_state", {})
        it = trigger.internal

        # Check memory
        if it.memory_contains:
            memories = npc_state.get("recent_memories", [])
            found = any(it.memory_contains.lower() in str(mem).lower() for mem in memories)
            if not found:
                return False

        # Check emotion
        if it.emotion_state:
            emotion = npc_state.get("emotion")
            if emotion != it.emotion_state:
                return False

        # Check active goal
        if it.goal_active:
            active_goals = npc_state.get("active_goals", [])
            if it.goal_active not in active_goals:
                return False

        # Check stress level
        if it.stress_level_above is not None:
            stress = npc_state.get("stress_level", 0.0)
            if stress <= it.stress_level_above:
                return False

        return True

    async def _evaluate_external(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate external (player-based) trigger"""
        if not trigger.external:
            return False

        player_state = context.get("player_state", {})
        et = trigger.external

        # Check item possession
        if et.player_has_item is not None:
            inventory = player_state.get("inventory", [])
            has_item = any(item.get("item_id") == et.player_has_item for item in inventory)
            if not has_item:
                return False

        # Check level
        player_level = player_state.get("level", 1)
        if et.player_level_min is not None and player_level < et.player_level_min:
            return False
        if et.player_level_max is not None and player_level > et.player_level_max:
            return False

        # Check class
        if et.player_class:
            player_class = player_state.get("class")
            if player_class != et.player_class:
                return False

        # Check quest completion
        if et.quest_completed:
            completed_quests = player_state.get("completed_quests", [])
            if et.quest_completed not in completed_quests:
                return False

        # Check reputation
        if et.player_reputation_min is not None:
            reputation = player_state.get("reputation", 0.0)
            if reputation < et.player_reputation_min:
                return False

        return True

    async def _evaluate_relationship(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate relationship trigger"""
        if not trigger.relationship:
            return False

        npc_id = context.get("npc_id")
        player_id = context.get("player_id")

        if not npc_id or not player_id:
            return False

        # Get relationship level
        relationship_key = f"relationship:{npc_id}:{player_id}"
        relationship_level = await db.client.get(relationship_key)
        relationship_level = float(relationship_level) if relationship_level else 0.0

        rt = trigger.relationship

        # Check minimum
        if relationship_level < rt.relationship_level_min:
            return False

        # Check maximum
        if rt.relationship_level_max is not None:
            if relationship_level > rt.relationship_level_max:
                return False

        # Check type (if implemented)
        if rt.relationship_type:
            # This would require additional relationship type tracking
            pass

        return True

    async def _evaluate_gift(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate gift trigger"""
        if not trigger.gift:
            return False

        npc_id = context.get("npc_id")
        if not npc_id:
            return False

        # Get gift preferences
        preferences_key = f"gift_preferences:{npc_id}"
        preferences_data = await db.client.get(preferences_key)

        if not preferences_data:
            return False

        import json
        preferences = json.loads(preferences_data)
        gt = trigger.gift

        # Check total gifts
        if gt.total_gifts_min is not None:
            total_gifts = preferences.get("total_gifts_received", 0)
            if total_gifts < gt.total_gifts_min:
                return False

        # Check specific item (from recent gift in context)
        if gt.specific_item_received is not None:
            recent_gift = context.get("recent_gift")
            if not recent_gift or recent_gift.get("item_id") != gt.specific_item_received:
                return False

        # Check gift category
        if gt.gift_category_received:
            recent_gift = context.get("recent_gift")
            if not recent_gift or recent_gift.get("category") != gt.gift_category_received:
                return False

        # Check total value
        if gt.total_gift_value_min is not None:
            gift_history = preferences.get("gift_history", [])
            total_value = sum(gift.get("gift", {}).get("base_value", 0) for gift in gift_history)
            if total_value < gt.total_gift_value_min:
                return False

        return True

    async def _evaluate_sequence(self, trigger: QuestTrigger, context: Dict[str, Any]) -> bool:
        """Evaluate sequence (prerequisite quest) trigger"""
        if not trigger.sequence:
            return False

        player_state = context.get("player_state", {})
        completed_quests = player_state.get("completed_quests", [])

        st = trigger.sequence

        if st.all_required:
            # All prerequisites must be completed
            return all(quest_id in completed_quests for quest_id in st.prerequisite_quests)
        else:
            # Any prerequisite can be completed
            return any(quest_id in completed_quests for quest_id in st.prerequisite_quests)

    async def evaluate_all_triggers(
        self,
        triggers: List[QuestTrigger],
        context: Dict[str, Any]
    ) -> List[QuestTrigger]:
        """
        Evaluate all triggers and return those that are met

        Args:
            triggers: List of triggers to evaluate
            context: Context data

        Returns:
            List of triggers that are met, sorted by priority
        """
        met_triggers = []

        for trigger in triggers:
            if await self.evaluate_trigger(trigger, context):
                met_triggers.append(trigger)

        # Sort by priority (higher first)
        met_triggers.sort(key=lambda t: t.priority, reverse=True)

        return met_triggers


# Global instance
_trigger_engine = None

def get_trigger_engine() -> QuestTriggerEngine:
    """Get global trigger engine instance"""
    global _trigger_engine
    if _trigger_engine is None:
        _trigger_engine = QuestTriggerEngine()
    return _trigger_engine

