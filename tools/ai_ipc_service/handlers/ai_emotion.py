"""
AI Emotion Handler for AI IPC Service

This handler processes AI emotion requests for NPC emotional state queries,
enabling NPCs to express and maintain emotional states based on interactions.

Request Types:
    - ai_npc_get_emotion
    - ai_emotion
    - npc_emotion

Request Data Format:
    {
        "npc_id": "merchant_001",
        "context": {
            "recent_interactions": [...],
            "current_emotion": "neutral",
            "personality_traits": ["friendly", "cautious"],
            "relationship_level": 50
        },
        "trigger_event": "player_purchase"
    }

Response Format:
    {
        "status": "ok",
        "npc_id": "merchant_001",
        "emotion": "happy",
        "intensity": 0.75,
        "expression": "smile",
        "dialogue_modifier": "enthusiastic",
        "duration_seconds": 30
    }
"""

from __future__ import annotations

import random
from datetime import datetime
from typing import Any

import aiohttp
from aiohttp import ClientTimeout

from .base import BaseHandler, HandlerError, ValidationError


# Emotion definitions with relationships
EMOTION_TRANSITIONS: dict[str, dict[str, float]] = {
    "neutral": {"happy": 0.3, "sad": 0.15, "angry": 0.1, "fearful": 0.1, "surprised": 0.15, "neutral": 0.2},
    "happy": {"happy": 0.5, "neutral": 0.35, "surprised": 0.1, "sad": 0.05},
    "sad": {"sad": 0.4, "neutral": 0.35, "angry": 0.15, "fearful": 0.1},
    "angry": {"angry": 0.35, "neutral": 0.4, "sad": 0.15, "fearful": 0.1},
    "fearful": {"fearful": 0.3, "neutral": 0.35, "sad": 0.2, "angry": 0.15},
    "surprised": {"surprised": 0.2, "neutral": 0.4, "happy": 0.2, "fearful": 0.1, "angry": 0.1},
}

EMOTION_EXPRESSIONS: dict[str, list[str]] = {
    "happy": ["smile", "laugh", "beam", "grin"],
    "sad": ["frown", "sigh", "tear", "drooped"],
    "angry": ["scowl", "glare", "snarl", "tense"],
    "fearful": ["tremble", "wide_eyes", "pale", "recoil"],
    "surprised": ["gasp", "wide_eyes", "jump", "exclaim"],
    "neutral": ["calm", "relaxed", "attentive", "composed"],
}

DIALOGUE_MODIFIERS: dict[str, list[str]] = {
    "happy": ["enthusiastic", "cheerful", "warm", "friendly"],
    "sad": ["melancholic", "subdued", "quiet", "wistful"],
    "angry": ["harsh", "terse", "biting", "cold"],
    "fearful": ["nervous", "hesitant", "shaky", "quiet"],
    "surprised": ["exclamatory", "breathless", "animated", "startled"],
    "neutral": ["calm", "professional", "measured", "steady"],
}

# Event-to-emotion mappings
EVENT_EMOTIONS: dict[str, tuple[str, float]] = {
    # Positive events
    "player_purchase": ("happy", 0.6),
    "player_gift": ("happy", 0.8),
    "player_help": ("happy", 0.7),
    "player_compliment": ("happy", 0.65),
    "quest_complete": ("happy", 0.75),
    "good_news": ("happy", 0.7),
    
    # Negative events
    "player_insult": ("angry", 0.7),
    "player_attack": ("fearful", 0.8),
    "theft_attempt": ("angry", 0.75),
    "monster_nearby": ("fearful", 0.6),
    "bad_news": ("sad", 0.65),
    "friend_hurt": ("sad", 0.7),
    
    # Surprising events
    "unexpected_visitor": ("surprised", 0.6),
    "sudden_noise": ("surprised", 0.5),
    "rare_item_seen": ("surprised", 0.7),
    
    # Neutral events
    "player_greeting": ("happy", 0.3),
    "time_passing": ("neutral", 0.2),
    "routine_task": ("neutral", 0.1),
}


class AIEmotionHandler(BaseHandler):
    """
    Handler for AI-powered NPC emotion processing.
    
    Processes emotion queries and updates by analyzing context,
    recent interactions, and trigger events to determine appropriate
    emotional responses.
    
    The handler supports:
    - Emotion state transitions based on events
    - Personality-influenced emotion responses
    - Expression and dialogue modifier generation
    - Emotion decay over time
    """
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process AI emotion request.
        
        Args:
            request: Request dictionary with emotion context
            
        Returns:
            Emotion response with state and expression details
        """
        self.log_request_start(request)
        
        # Parse request data
        data = self.get_request_data(request)
        
        # Validate request
        self._validate_emotion_request(data)
        
        # Extract parameters
        npc_id = data.get("npc_id", "unknown_npc")
        context = data.get("context", {})
        trigger_event = data.get("trigger_event")
        
        # Try external AI service first
        try:
            response = await self._call_ai_service(
                npc_id=npc_id,
                context=context,
                trigger_event=trigger_event,
            )
            return response
        except Exception as e:
            self.logger.warning(
                f"AI emotion service call failed, using fallback: {e}"
            )
            # Fall back to rule-based emotion
            return self._generate_fallback_emotion(
                npc_id=npc_id,
                context=context,
                trigger_event=trigger_event,
            )
    
    def _validate_emotion_request(self, data: dict[str, Any]) -> None:
        """
        Validate emotion request data.
        
        Args:
            data: Request data dictionary
            
        Raises:
            ValidationError: If required fields are missing
        """
        if not data.get("npc_id"):
            raise ValidationError("npc_id is required for emotion requests")
    
    async def _call_ai_service(
        self,
        npc_id: str,
        context: dict[str, Any],
        trigger_event: str | None,
    ) -> dict[str, Any]:
        """
        Call external AI service for emotion processing.
        
        Args:
            npc_id: NPC identifier
            context: Emotion context
            trigger_event: Event that triggered emotion query
            
        Returns:
            AI-generated emotion response
        """
        base_url = self.config.ai_service.base_url.rstrip("/")
        endpoint = f"{base_url}/api/v1/npc/{npc_id}/emotion"
        
        payload = {
            "npc_id": npc_id,
            "context": context,
            "trigger_event": trigger_event,
            "timestamp": datetime.utcnow().isoformat(),
        }
        
        timeout = ClientTimeout(total=self.config.ai_service.timeout_seconds)
        
        async with aiohttp.ClientSession(timeout=timeout) as session:
            async with session.post(endpoint, json=payload) as response:
                if response.status == 200:
                    result = await response.json()
                    return self._format_ai_response(result, npc_id)
                else:
                    error_text = await response.text()
                    raise HandlerError(
                        f"AI service returned {response.status}: {error_text}"
                    )
    
    def _format_ai_response(
        self, 
        ai_result: dict[str, Any], 
        npc_id: str
    ) -> dict[str, Any]:
        """
        Format AI service response into standard format.
        
        Args:
            ai_result: Raw response from AI service
            npc_id: NPC identifier
            
        Returns:
            Formatted emotion response
        """
        return {
            "status": "ok",
            "npc_id": npc_id,
            "emotion": ai_result.get("emotion", "neutral"),
            "intensity": ai_result.get("intensity", 0.5),
            "expression": ai_result.get("expression", "calm"),
            "dialogue_modifier": ai_result.get("dialogue_modifier", "neutral"),
            "duration_seconds": ai_result.get("duration_seconds", 30),
            "metadata": {
                "source": "ai_service",
                "model": ai_result.get("model"),
            },
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _generate_fallback_emotion(
        self,
        npc_id: str,
        context: dict[str, Any],
        trigger_event: str | None,
    ) -> dict[str, Any]:
        """
        Generate a fallback emotion using rule-based logic.
        
        Uses event mappings and personality modifiers when
        AI service is unavailable.
        
        Args:
            npc_id: NPC identifier
            context: Emotion context
            trigger_event: Event that triggered query
            
        Returns:
            Rule-based emotion response
        """
        current_emotion = context.get("current_emotion", "neutral")
        personality_traits = context.get("personality_traits", [])
        relationship_level = context.get("relationship_level", 50)
        
        # Determine new emotion based on trigger event
        if trigger_event and trigger_event in EVENT_EMOTIONS:
            emotion, base_intensity = EVENT_EMOTIONS[trigger_event]
        else:
            # Use emotion transitions from current state
            emotion = self._transition_emotion(current_emotion)
            base_intensity = 0.4
        
        # Apply personality modifiers
        intensity = self._apply_personality_modifier(
            base_intensity, 
            emotion, 
            personality_traits,
            relationship_level,
        )
        
        # Get expression and dialogue modifier
        expression = random.choice(EMOTION_EXPRESSIONS.get(emotion, ["calm"]))
        dialogue_modifier = random.choice(
            DIALOGUE_MODIFIERS.get(emotion, ["neutral"])
        )
        
        # Calculate duration based on intensity
        duration = self._calculate_duration(intensity)
        
        return {
            "status": "ok",
            "npc_id": npc_id,
            "emotion": emotion,
            "intensity": round(intensity, 2),
            "expression": expression,
            "dialogue_modifier": dialogue_modifier,
            "duration_seconds": duration,
            "metadata": {
                "source": "fallback",
                "rule_based": True,
                "trigger_event": trigger_event,
                "previous_emotion": current_emotion,
            },
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _transition_emotion(self, current: str) -> str:
        """
        Calculate emotion transition from current state.
        
        Args:
            current: Current emotion state
            
        Returns:
            New emotion state
        """
        transitions = EMOTION_TRANSITIONS.get(current, EMOTION_TRANSITIONS["neutral"])
        emotions = list(transitions.keys())
        weights = list(transitions.values())
        return random.choices(emotions, weights=weights, k=1)[0]
    
    def _apply_personality_modifier(
        self,
        base_intensity: float,
        emotion: str,
        personality_traits: list[str],
        relationship_level: int,
    ) -> float:
        """
        Apply personality-based modifiers to emotion intensity.
        
        Args:
            base_intensity: Base emotion intensity
            emotion: Target emotion
            personality_traits: NPC personality traits
            relationship_level: Player-NPC relationship (0-100)
            
        Returns:
            Modified intensity value (0.0-1.0)
        """
        intensity = base_intensity
        
        # Personality trait modifiers
        trait_modifiers = {
            # Trait: {emotion: modifier}
            "friendly": {"happy": 0.1, "angry": -0.1, "fearful": -0.05},
            "cautious": {"fearful": 0.1, "surprised": 0.05, "happy": -0.05},
            "aggressive": {"angry": 0.15, "fearful": -0.1},
            "timid": {"fearful": 0.15, "angry": -0.1, "happy": -0.05},
            "cheerful": {"happy": 0.15, "sad": -0.1},
            "melancholic": {"sad": 0.1, "happy": -0.1},
            "stoic": {"neutral": 0.1, "happy": -0.1, "sad": -0.1},
        }
        
        for trait in personality_traits:
            if trait in trait_modifiers:
                modifiers = trait_modifiers[trait]
                if emotion in modifiers:
                    intensity += modifiers[emotion]
        
        # Relationship modifier (high relationship = more positive emotions)
        relationship_factor = (relationship_level - 50) / 100  # -0.5 to +0.5
        
        if emotion in ("happy", "neutral"):
            intensity += relationship_factor * 0.1
        elif emotion in ("angry", "fearful"):
            intensity -= relationship_factor * 0.1
        
        # Clamp to valid range
        return max(0.1, min(1.0, intensity))
    
    def _calculate_duration(self, intensity: float) -> int:
        """
        Calculate emotion duration based on intensity.
        
        Higher intensity emotions last longer.
        
        Args:
            intensity: Emotion intensity (0.0-1.0)
            
        Returns:
            Duration in seconds
        """
        # Base duration: 10-60 seconds, scaled by intensity
        base_duration = 10
        max_additional = 50
        
        duration = base_duration + int(intensity * max_additional)
        
        # Add some randomness (±20%)
        variance = int(duration * 0.2)
        duration += random.randint(-variance, variance)
        
        return max(10, duration)