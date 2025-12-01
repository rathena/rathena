"""
AI Decision Handler for AI IPC Service

This handler processes AI decision requests for NPC action determination,
enabling NPCs to make intelligent decisions based on context and game state.

Request Types:
    - ai_npc_decide_action
    - ai_decision
    - npc_decision

Request Data Format:
    {
        "npc_id": "guard_001",
        "context": {
            "current_state": "patrolling",
            "nearby_players": [...],
            "nearby_monsters": [...],
            "time_of_day": "night",
            "weather": "rain",
            "events": [...]
        },
        "available_actions": ["patrol", "greet", "alert", "rest", "investigate"]
    }

Response Format:
    {
        "status": "ok",
        "npc_id": "guard_001",
        "action": "investigate",
        "confidence": 0.85,
        "reasoning": "Detected suspicious activity nearby",
        "parameters": {
            "target_x": 150,
            "target_y": 200,
            "duration": 30
        }
    }
"""

from __future__ import annotations

import random
from datetime import datetime
from typing import Any

import aiohttp
from aiohttp import ClientTimeout

from .base import BaseHandler, HandlerError, ValidationError


class AIDecisionHandler(BaseHandler):
    """
    Handler for AI-powered NPC decision making.
    
    Processes decision requests by analyzing context and returning
    the most appropriate action for an NPC to take.
    
    The handler supports:
    - Context-aware decision making
    - Multiple action candidates with confidence scores
    - Fallback rule-based decisions when AI is unavailable
    """
    
    # Default action weights for fallback decisions
    DEFAULT_ACTION_WEIGHTS: dict[str, float] = {
        "patrol": 0.3,
        "idle": 0.2,
        "greet": 0.15,
        "rest": 0.1,
        "investigate": 0.1,
        "alert": 0.05,
        "flee": 0.05,
        "attack": 0.05,
    }
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process AI decision request.
        
        Args:
            request: Request dictionary with decision context
            
        Returns:
            Decision response with selected action and parameters
        """
        self.log_request_start(request)
        
        # Parse request data
        data = self.get_request_data(request)
        
        # Validate required fields
        self._validate_decision_request(data)
        
        # Extract decision parameters
        npc_id = data.get("npc_id", "unknown_npc")
        context = data.get("context", {})
        available_actions = data.get("available_actions", [])
        
        # Try external AI service first
        try:
            response = await self._call_ai_service(
                npc_id=npc_id,
                context=context,
                available_actions=available_actions,
            )
            return response
        except Exception as e:
            self.logger.warning(
                f"AI decision service call failed, using fallback: {e}"
            )
            # Fall back to rule-based decision
            return self._generate_fallback_decision(
                npc_id=npc_id,
                context=context,
                available_actions=available_actions,
            )
    
    def _validate_decision_request(self, data: dict[str, Any]) -> None:
        """
        Validate decision request data.
        
        Args:
            data: Request data dictionary
            
        Raises:
            ValidationError: If required fields are missing
        """
        if not data.get("npc_id"):
            raise ValidationError("npc_id is required for decision requests")
    
    async def _call_ai_service(
        self,
        npc_id: str,
        context: dict[str, Any],
        available_actions: list[str],
    ) -> dict[str, Any]:
        """
        Call external AI service for decision making.
        
        Args:
            npc_id: NPC identifier
            context: Decision context
            available_actions: List of possible actions
            
        Returns:
            AI-generated decision response
        """
        base_url = self.config.ai_service.base_url.rstrip("/")
        endpoint = f"{base_url}/api/v1/npc/{npc_id}/decide"
        
        payload = {
            "npc_id": npc_id,
            "context": context,
            "available_actions": available_actions,
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
            Formatted decision response
        """
        return {
            "status": "ok",
            "npc_id": npc_id,
            "action": ai_result.get("action", "idle"),
            "confidence": ai_result.get("confidence", 0.5),
            "reasoning": ai_result.get("reasoning", ""),
            "parameters": ai_result.get("parameters", {}),
            "metadata": {
                "source": "ai_service",
                "model": ai_result.get("model"),
            },
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _generate_fallback_decision(
        self,
        npc_id: str,
        context: dict[str, Any],
        available_actions: list[str],
    ) -> dict[str, Any]:
        """
        Generate a fallback decision using rule-based logic.
        
        Uses context analysis and weighted random selection when
        AI service is unavailable.
        
        Args:
            npc_id: NPC identifier
            context: Decision context
            available_actions: List of possible actions
            
        Returns:
            Rule-based decision response
        """
        # Analyze context for special conditions
        action, confidence, reasoning = self._analyze_context(
            context, available_actions
        )
        
        return {
            "status": "ok",
            "npc_id": npc_id,
            "action": action,
            "confidence": confidence,
            "reasoning": reasoning,
            "parameters": self._generate_action_parameters(action, context),
            "metadata": {
                "source": "fallback",
                "rule_based": True,
            },
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _analyze_context(
        self,
        context: dict[str, Any],
        available_actions: list[str],
    ) -> tuple[str, float, str]:
        """
        Analyze context and select appropriate action.
        
        Args:
            context: Decision context
            available_actions: Available actions
            
        Returns:
            Tuple of (action, confidence, reasoning)
        """
        # Check for threat conditions
        if context.get("threat_level", 0) > 0.7:
            if "alert" in available_actions:
                return "alert", 0.9, "High threat level detected"
            if "flee" in available_actions:
                return "flee", 0.85, "Dangerous threat nearby"
        
        # Check for player interaction opportunities
        nearby_players = context.get("nearby_players", [])
        if nearby_players and "greet" in available_actions:
            return "greet", 0.7, f"Player {nearby_players[0]} nearby"
        
        # Check current state continuity
        current_state = context.get("current_state", "")
        if current_state == "patrolling" and "patrol" in available_actions:
            return "patrol", 0.6, "Continuing patrol route"
        
        # Check for events that need investigation
        events = context.get("events", [])
        if events and "investigate" in available_actions:
            return "investigate", 0.65, f"Event detected: {events[0]}"
        
        # Check time-based conditions
        time_of_day = context.get("time_of_day", "day")
        if time_of_day == "night" and "rest" in available_actions:
            return "rest", 0.5, "Night time rest"
        
        # Default: weighted random from available actions
        if available_actions:
            weights = [
                self.DEFAULT_ACTION_WEIGHTS.get(a, 0.1) 
                for a in available_actions
            ]
            total = sum(weights)
            weights = [w / total for w in weights]
            
            action = random.choices(available_actions, weights=weights, k=1)[0]
            return action, 0.4, "Default action selection"
        
        return "idle", 0.3, "No available actions"
    
    def _generate_action_parameters(
        self,
        action: str,
        context: dict[str, Any],
    ) -> dict[str, Any]:
        """
        Generate parameters for the selected action.
        
        Args:
            action: Selected action
            context: Decision context
            
        Returns:
            Action-specific parameters
        """
        parameters: dict[str, Any] = {}
        
        if action == "patrol":
            # Generate patrol waypoint
            parameters["duration"] = random.randint(30, 120)
            
        elif action == "greet":
            nearby = context.get("nearby_players", [])
            if nearby:
                parameters["target_player"] = nearby[0]
            parameters["greeting_type"] = random.choice(
                ["wave", "bow", "nod", "smile"]
            )
            
        elif action == "investigate":
            events = context.get("events", [])
            if events:
                parameters["target_event"] = events[0]
            parameters["caution_level"] = 0.5
            
        elif action == "rest":
            parameters["duration"] = random.randint(60, 300)
            
        elif action == "alert":
            parameters["alert_level"] = context.get("threat_level", 0.5)
            parameters["notify_guards"] = True
            
        elif action == "flee":
            parameters["flee_distance"] = 10
            parameters["safe_zone"] = "nearest"
        
        return parameters