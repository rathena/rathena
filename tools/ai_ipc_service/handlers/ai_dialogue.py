"""
AI Dialogue Handler for AI IPC Service

This handler processes AI dialogue requests for NPC conversations,
enabling dynamic, AI-generated responses based on player interactions.

Request Types:
    - dialogue
    - ai_dialogue
    - npc_dialogue

Request Data Format:
    {
        "npc_id": "merchant_001",
        "npc_name": "Alice the Merchant",
        "player_id": 12345,
        "player_name": "Hero",
        "player_message": "Hello! What items do you have?",
        "context": {
            "map": "prontera",
            "previous_messages": [...],
            "player_class": "Knight",
            "player_level": 50
        }
    }

Response Format:
    {
        "status": "ok",
        "npc_id": "merchant_001",
        "response": "Welcome, brave Knight! I have fine weapons...",
        "emotion": "friendly",
        "actions": [
            {"type": "emote", "value": "smile"},
            {"type": "effect", "value": "sparkle"}
        ],
        "menu_options": ["Buy Items", "Sell Items", "Leave"]
    }
"""

from __future__ import annotations

import asyncio
import json
import time
from datetime import datetime
from typing import Any

import aiohttp
from aiohttp import ClientTimeout

from .base import BaseHandler, HandlerError, ValidationError


class AIDialogueHandler(BaseHandler):
    """
    Handler for AI-powered NPC dialogue generation.
    
    Processes dialogue requests by forwarding to an AI/LLM service
    or generating placeholder responses for testing.
    
    The handler supports:
    - Forwarding to external AI service
    - Context-aware dialogue generation
    - Emotion and action annotations
    - Menu option generation for NPC choices
    """
    
    # Default NPC personality templates
    DEFAULT_PERSONALITIES: dict[str, str] = {
        "merchant": "You are a friendly merchant NPC. You help players buy and sell items.",
        "guard": "You are a stern city guard. You maintain order and provide directions.",
        "quest_giver": "You are a mysterious quest giver. You offer adventure and rewards.",
        "healer": "You are a kind healer. You restore health and offer advice.",
        "blacksmith": "You are a skilled blacksmith. You craft and upgrade weapons.",
        "default": "You are an NPC in a fantasy MMORPG. Be helpful and stay in character.",
    }
    
    # Default emotions for response variety
    EMOTIONS = ["neutral", "happy", "friendly", "serious", "mysterious", "excited"]
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process AI dialogue request.
        
        Args:
            request: Request dictionary with dialogue context
            
        Returns:
            AI-generated dialogue response
        """
        self.log_request_start(request)
        
        # Parse request data
        data = self.get_request_data(request)
        
        # Validate required fields
        self._validate_dialogue_request(data)
        
        # Extract dialogue parameters
        npc_id = data.get("npc_id", "unknown_npc")
        npc_name = data.get("npc_name", "NPC")
        player_message = data.get("player_message", "")
        context = data.get("context", {})
        
        # Try external AI service first
        try:
            response = await self._call_ai_service(
                npc_id=npc_id,
                npc_name=npc_name,
                player_message=player_message,
                context=context,
                request_data=data,
            )
            return response
        except Exception as e:
            self.logger.warning(
                f"AI service call failed, using fallback: {e}"
            )
            # Fall back to template response
            return self._generate_fallback_response(
                npc_id=npc_id,
                npc_name=npc_name,
                player_message=player_message,
                context=context,
            )
    
    def _validate_dialogue_request(self, data: dict[str, Any]) -> None:
        """
        Validate dialogue request data.
        
        Args:
            data: Request data dictionary
            
        Raises:
            ValidationError: If required fields are missing
        """
        # npc_id is required for routing
        if not data.get("npc_id") and not data.get("npc_name"):
            raise ValidationError(
                "Either npc_id or npc_name is required for dialogue requests"
            )
    
    async def _call_ai_service(
        self,
        npc_id: str,
        npc_name: str,
        player_message: str,
        context: dict[str, Any],
        request_data: dict[str, Any],
    ) -> dict[str, Any]:
        """
        Call external AI service for dialogue generation.
        
        Args:
            npc_id: NPC identifier
            npc_name: NPC display name
            player_message: Player's message to NPC
            context: Conversation context
            request_data: Full request data
            
        Returns:
            AI-generated dialogue response
        """
        base_url = self.config.ai_service.base_url.rstrip("/")
        endpoint = f"{base_url}/api/v1/npc/{npc_id}/dialogue"
        
        payload = {
            "npc_id": npc_id,
            "npc_name": npc_name,
            "player_message": player_message,
            "context": context,
            "timestamp": datetime.utcnow().isoformat(),
        }
        
        # Add optional fields if present
        for field in ["player_id", "player_name", "player_class", "player_level"]:
            if field in request_data:
                payload[field] = request_data[field]
        
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
            Formatted dialogue response
        """
        return {
            "status": "ok",
            "npc_id": npc_id,
            "response": ai_result.get("response", ai_result.get("message", "")),
            "emotion": ai_result.get("emotion", "neutral"),
            "actions": ai_result.get("actions", []),
            "menu_options": ai_result.get("menu_options", []),
            "metadata": {
                "source": "ai_service",
                "model": ai_result.get("model"),
                "tokens_used": ai_result.get("tokens_used"),
            },
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _generate_fallback_response(
        self,
        npc_id: str,
        npc_name: str,
        player_message: str,
        context: dict[str, Any],
    ) -> dict[str, Any]:
        """
        Generate a fallback response when AI service is unavailable.
        
        Uses template-based responses based on NPC type and player input.
        
        Args:
            npc_id: NPC identifier
            npc_name: NPC display name
            player_message: Player's message
            context: Conversation context
            
        Returns:
            Template-based dialogue response
        """
        # Determine NPC type from id or name
        npc_type = self._determine_npc_type(npc_id, npc_name)
        
        # Generate appropriate response
        response_text = self._generate_template_response(
            npc_type=npc_type,
            npc_name=npc_name,
            player_message=player_message,
            context=context,
        )
        
        # Determine emotion
        emotion = self._determine_emotion(player_message)
        
        return {
            "status": "ok",
            "npc_id": npc_id,
            "response": response_text,
            "emotion": emotion,
            "actions": self._generate_actions(npc_type, emotion),
            "menu_options": self._generate_menu_options(npc_type),
            "metadata": {
                "source": "fallback",
                "npc_type": npc_type,
            },
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _determine_npc_type(self, npc_id: str, npc_name: str) -> str:
        """
        Determine NPC type from identifier or name.
        
        Args:
            npc_id: NPC identifier
            npc_name: NPC display name
            
        Returns:
            NPC type string
        """
        # Check id and name for type keywords
        combined = f"{npc_id} {npc_name}".lower()
        
        type_keywords = {
            "merchant": ["merchant", "shop", "store", "vendor", "trader"],
            "guard": ["guard", "soldier", "knight", "warrior", "patrol"],
            "quest_giver": ["quest", "mission", "adventure", "elder", "sage"],
            "healer": ["healer", "priest", "nurse", "doctor", "cleric"],
            "blacksmith": ["blacksmith", "smith", "forge", "armorer", "weaponsmith"],
        }
        
        for npc_type, keywords in type_keywords.items():
            if any(kw in combined for kw in keywords):
                return npc_type
        
        return "default"
    
    def _generate_template_response(
        self,
        npc_type: str,
        npc_name: str,
        player_message: str,
        context: dict[str, Any],
    ) -> str:
        """
        Generate a template-based response.
        
        Args:
            npc_type: Type of NPC
            npc_name: NPC display name
            player_message: Player's message
            context: Conversation context
            
        Returns:
            Generated response text
        """
        # Default responses by NPC type
        responses = {
            "merchant": [
                f"Welcome, traveler! I'm {npc_name}. What would you like to buy today?",
                "I have the finest goods in all the land. Take a look!",
                "Ah, a customer! Let me show you my wares.",
            ],
            "guard": [
                f"Halt! I'm {npc_name}, guardian of this area. State your business.",
                "Keep the peace, citizen. I'm watching.",
                "Need directions? I know every corner of this city.",
            ],
            "quest_giver": [
                f"Greetings, adventurer. I am {npc_name}. I sense great potential in you.",
                "I have a task that requires someone of your abilities...",
                "Legend speaks of a hero who will answer the call. Are you that hero?",
            ],
            "healer": [
                f"Welcome to my sanctuary. I'm {npc_name}. How may I help you heal?",
                "Rest here, weary traveler. I can restore your strength.",
                "The light protects all who seek aid. Let me help you.",
            ],
            "blacksmith": [
                f"*clang* *clang* Oh! Welcome! I'm {npc_name}. Need something forged?",
                "My weapons are the finest steel. Want to see what I can make?",
                "Every hero needs good equipment. Let me outfit you properly.",
            ],
            "default": [
                f"Hello there! I'm {npc_name}. How can I help you?",
                "Good day, traveler. What brings you here?",
                "Ah, a visitor! Welcome!",
            ],
        }
        
        # Select response based on type
        import random
        type_responses = responses.get(npc_type, responses["default"])
        return random.choice(type_responses)
    
    def _determine_emotion(self, player_message: str) -> str:
        """
        Determine appropriate emotion based on player message.
        
        Args:
            player_message: Player's message
            
        Returns:
            Emotion string
        """
        message_lower = player_message.lower()
        
        # Simple keyword matching for emotion
        if any(w in message_lower for w in ["hello", "hi", "hey", "greetings"]):
            return "friendly"
        elif any(w in message_lower for w in ["help", "please", "need"]):
            return "helpful"
        elif any(w in message_lower for w in ["buy", "sell", "trade"]):
            return "interested"
        elif any(w in message_lower for w in ["quest", "adventure", "mission"]):
            return "excited"
        elif any(w in message_lower for w in ["bye", "goodbye", "leave"]):
            return "neutral"
        
        return "neutral"
    
    def _generate_actions(self, npc_type: str, emotion: str) -> list[dict[str, Any]]:
        """
        Generate NPC actions based on type and emotion.
        
        Args:
            npc_type: Type of NPC
            emotion: Current emotion
            
        Returns:
            List of action dictionaries
        """
        actions = []
        
        # Add emotion-based emote
        emote_map = {
            "friendly": "wave",
            "helpful": "nod",
            "interested": "curious",
            "excited": "jump",
            "neutral": None,
        }
        
        emote = emote_map.get(emotion)
        if emote:
            actions.append({"type": "emote", "value": emote})
        
        # Add type-specific actions
        type_actions = {
            "merchant": [{"type": "effect", "value": "shop_sparkle"}],
            "healer": [{"type": "effect", "value": "holy_light"}],
            "blacksmith": [{"type": "sound", "value": "anvil_strike"}],
        }
        
        actions.extend(type_actions.get(npc_type, []))
        
        return actions
    
    def _generate_menu_options(self, npc_type: str) -> list[str]:
        """
        Generate menu options based on NPC type.
        
        Args:
            npc_type: Type of NPC
            
        Returns:
            List of menu option strings
        """
        menus = {
            "merchant": ["Buy Items", "Sell Items", "View Special Deals", "Leave"],
            "guard": ["Ask for Directions", "Report a Crime", "Leave"],
            "quest_giver": ["Accept Quest", "Ask About Rewards", "Decline", "Leave"],
            "healer": ["Heal HP/SP", "Remove Status Effects", "Buy Potions", "Leave"],
            "blacksmith": ["Forge Weapon", "Repair Equipment", "Upgrade Item", "Leave"],
            "default": ["Continue Conversation", "Leave"],
        }
        
        return menus.get(npc_type, menus["default"])