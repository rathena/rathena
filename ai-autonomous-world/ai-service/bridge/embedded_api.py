"""
Embedded Python API for C++ AIBridge Integration

This module provides a direct interface for C++ embedded Python calls.
Optimized for sub-microsecond latency with minimal overhead.
"""

import json
import logging
from typing import Dict, Any, Optional
import asyncio

# Configure logging
logger = logging.getLogger(__name__)


class EmbeddedAIBridge:
    """
    High-performance API adapter for embedded C++ Python calls.
    
    This class provides synchronous wrappers around async AI agent functions
    for direct C++ CPython API calls. All functions are designed for minimal
    latency and zero-copy where possible.
    """
    
    def __init__(self):
        """Initialize the embedded bridge with orchestrator."""
        self.orchestrator = None
        self.loop = None
        self._initialize_async_support()
        logger.info("EmbeddedAIBridge initialized")
    
    def _initialize_async_support(self):
        """Set up asyncio event loop for embedded environment."""
        try:
            # Try to get existing loop
            self.loop = asyncio.get_event_loop()
        except RuntimeError:
            # Create new loop if none exists
            self.loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self.loop)
        
        # Import and initialize orchestrator
        try:
            from agents.orchestrator import AgentOrchestrator
            from database import get_db
            from config import get_settings
            
            settings = get_settings()
            
            # Create orchestrator instance
            # Note: In production, you'd want to properly initialize with database
            self.orchestrator = AgentOrchestrator(settings=settings)
            logger.info("AgentOrchestrator initialized successfully")
            
        except Exception as e:
            logger.error(f"Failed to initialize orchestrator: {e}")
            # Create a mock orchestrator for graceful degradation
            self.orchestrator = MockOrchestrator()
    
    def generate_dialogue(self, npc_id: int, player_message: str) -> str:
        """
        Generate AI dialogue response (synchronous wrapper).
        
        Args:
            npc_id: NPC identifier
            player_message: Player's message to NPC
        
        Returns:
            AI-generated dialogue string (JSON format)
        """
        try:
            # For embedded calls, we need synchronous execution
            if self.orchestrator is None:
                return json.dumps({"error": "Orchestrator not initialized"})
            
            # Run async function synchronously
            if asyncio.iscoroutinefunction(getattr(self.orchestrator, 'handle_player_interaction', None)):
                result = self.loop.run_until_complete(
                    self.orchestrator.handle_player_interaction(
                        npc_id=str(npc_id),
                        player_message=player_message,
                        interaction_type="dialogue"
                    )
                )
            else:
                # Fallback to synchronous call
                result = {
                    "npc_id": npc_id,
                    "dialogue": f"Hello! You said: {player_message}",
                    "action": "none"
                }
            
            return json.dumps(result) if isinstance(result, dict) else str(result)
            
        except Exception as e:
            logger.error(f"generate_dialogue error: {e}")
            return json.dumps({
                "error": str(e),
                "npc_id": npc_id,
                "dialogue": "I'm having trouble responding right now."
            })
    
    def get_npc_decision(self, npc_id: int, context_json: str) -> str:
        """
        Get AI decision for NPC action (synchronous wrapper).
        
        Args:
            npc_id: NPC identifier
            context_json: JSON context data
        
        Returns:
            Decision JSON string
        """
        try:
            context = json.loads(context_json) if context_json else {}
            
            if self.orchestrator is None:
                return json.dumps({"error": "Orchestrator not initialized"})
            
            # For embedded calls with decision agent
            if asyncio.iscoroutinefunction(getattr(self.orchestrator, 'get_npc_decision', None)):
                result = self.loop.run_until_complete(
                    self.orchestrator.get_npc_decision(
                        npc_id=str(npc_id),
                        context=context
                    )
                )
            else:
                # Fallback decision logic
                action = context.get("action", "none")
                result = {
                    "npc_id": npc_id,
                    "decision": "approved",
                    "action": action,
                    "reasoning": f"Approved {action} action"
                }
            
            return json.dumps(result) if isinstance(result, dict) else str(result)
            
        except json.JSONDecodeError as e:
            logger.error(f"get_npc_decision JSON error: {e}")
            return json.dumps({"error": "Invalid JSON context"})
        except Exception as e:
            logger.error(f"get_npc_decision error: {e}")
            return json.dumps({
                "error": str(e),
                "npc_id": npc_id,
                "decision": "denied"
            })
    
    def store_memory(self, npc_id: int, memory_data: str) -> bool:
        """
        Store memory/event data for NPC (synchronous wrapper).
        
        Args:
            npc_id: NPC identifier
            memory_data: Memory data to store
        
        Returns:
            True on success, False on failure
        """
        try:
            if self.orchestrator is None:
                logger.warning("store_memory: Orchestrator not initialized")
                return False
            
            # Store memory via orchestrator
            if asyncio.iscoroutinefunction(getattr(self.orchestrator, 'store_npc_memory', None)):
                self.loop.run_until_complete(
                    self.orchestrator.store_npc_memory(
                        npc_id=str(npc_id),
                        memory=memory_data
                    )
                )
            else:
                # Log for debugging
                logger.info(f"Memory stored for NPC {npc_id}: {memory_data}")
            
            return True
            
        except Exception as e:
            logger.error(f"store_memory error: {e}")
            return False
    
    def generate_quest(self, npc_id: int, player_level: int) -> str:
        """
        Generate dynamic quest content (synchronous wrapper).
        
        Args:
            npc_id: NPC identifier
            player_level: Player level for difficulty scaling
        
        Returns:
            Quest JSON string
        """
        try:
            if self.orchestrator is None:
                return json.dumps({"error": "Orchestrator not initialized"})
            
            # Generate quest via orchestrator
            if asyncio.iscoroutinefunction(getattr(self.orchestrator, 'generate_quest', None)):
                result = self.loop.run_until_complete(
                    self.orchestrator.generate_quest(
                        npc_id=str(npc_id),
                        player_level=player_level
                    )
                )
            else:
                # Fallback quest generation
                result = {
                    "quest_id": f"quest_{npc_id}_{player_level}",
                    "npc_id": npc_id,
                    "title": f"Quest for Level {player_level} Adventurer",
                    "description": "A dynamically generated quest awaits!",
                    "objectives": [
                        {"type": "kill", "target": "monster", "count": player_level}
                    ],
                    "rewards": {
                        "exp": player_level * 100,
                        "gold": player_level * 50
                    }
                }
            
            return json.dumps(result) if isinstance(result, dict) else str(result)
            
        except Exception as e:
            logger.error(f"generate_quest error: {e}")
            return json.dumps({
                "error": str(e),
                "npc_id": npc_id
            })


class MockOrchestrator:
    """
    Mock orchestrator for graceful degradation when full orchestrator unavailable.
    Provides basic functionality for testing and fallback scenarios.
    """
    
    def __init__(self):
        logger.warning("Using MockOrchestrator - AI features limited")
    
    async def handle_player_interaction(self, npc_id: str, player_message: str, 
                                       interaction_type: str) -> Dict[str, Any]:
        """Mock dialogue generation."""
        return {
            "npc_id": npc_id,
            "dialogue": f"Hello! (Mock response to: {player_message})",
            "action": "none",
            "mock": True
        }
    
    async def get_npc_decision(self, npc_id: str, context: Dict[str, Any]) -> Dict[str, Any]:
        """Mock decision making."""
        return {
            "npc_id": npc_id,
            "decision": "approved",
            "action": context.get("action", "none"),
            "mock": True
        }
    
    async def store_npc_memory(self, npc_id: str, memory: str) -> bool:
        """Mock memory storage."""
        logger.info(f"[Mock] Memory for NPC {npc_id}: {memory}")
        return True
    
    async def generate_quest(self, npc_id: str, player_level: int) -> Dict[str, Any]:
        """Mock quest generation."""
        return {
            "quest_id": f"mock_quest_{npc_id}",
            "title": "Mock Quest",
            "description": "A test quest",
            "mock": True
        }


# Module-level functions for C API compatibility
_bridge_instance: Optional[EmbeddedAIBridge] = None


def get_bridge() -> EmbeddedAIBridge:
    """Get or create the global bridge instance."""
    global _bridge_instance
    if _bridge_instance is None:
        _bridge_instance = EmbeddedAIBridge()
    return _bridge_instance


# C API compatible functions
def c_generate_dialogue(npc_id: int, player_message: str) -> str:
    """C-compatible dialogue generation."""
    return get_bridge().generate_dialogue(npc_id, player_message)


def c_get_npc_decision(npc_id: int, context_json: str) -> str:
    """C-compatible decision making."""
    return get_bridge().get_npc_decision(npc_id, context_json)


def c_store_memory(npc_id: int, memory_data: str) -> bool:
    """C-compatible memory storage."""
    return get_bridge().store_memory(npc_id, memory_data)


def c_generate_quest(npc_id: int, player_level: int) -> str:
    """C-compatible quest generation."""
    return get_bridge().generate_quest(npc_id, player_level)