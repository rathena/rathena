"""
NPC Movement Tasks - Event-Driven and Fixed Interval Modes
Implements agent-driven movement decisions with learning capability
Phase 8B: Uses Redis Pub/Sub for async NPC action communication
"""

import asyncio
from datetime import datetime, timedelta
from typing import List, Dict, Any, Optional
from loguru import logger

from config import settings
from database import db, postgres_db
from agents.decision_agent import DecisionAgent
from agents.base_agent import AgentContext
from utils.bridge_commands import translate_action_to_bridge_command
from utils.pubsub import get_pubsub_manager  # Phase 8B: Pub/Sub for async actions
from models.npc import NPCAction
from tasks.instant_response import instant_response_manager, EventPriority
from tasks.npc_relationships import npc_relationship_manager


class NPCMovementManager:
    """
    Manages NPC movement with agent-driven decision making
    Supports both event-driven and fixed-interval modes
    """
    
    def __init__(self):
        self.decision_agent = DecisionAgent()
        self.last_movement_times: Dict[str, datetime] = {}
        logger.info("NPCMovementManager initialized")
    
    async def trigger_movement_on_interaction_end(self, npc_id: str, player_id: str, interaction_context: Dict[str, Any]):
        """
        Trigger NPC movement decision after player interaction ends
        Event-driven trigger: NPC decides next action after conversation
        """
        if not settings.npc_movement_enabled or settings.npc_movement_mode != "event_driven":
            return
        
        logger.info(f"Triggering post-interaction movement for NPC {npc_id} after interaction with player {player_id}")
        
        try:
            # Get NPC data
            npc_data = await self._get_npc_data(npc_id)
            if not npc_data or not npc_data.get('can_move', True):
                logger.debug(f"NPC {npc_id} cannot move or not found")
                return
            
            # Build agent context with interaction history
            context = await self._build_movement_context(
                npc_data,
                trigger_type="post_interaction",
                trigger_data={
                    "player_id": player_id,
                    "interaction_summary": interaction_context.get("summary", ""),
                    "player_sentiment": interaction_context.get("sentiment", "neutral")
                }
            )
            
            # Agent decides if and where to move
            decision = await self.decision_agent.process(context)
            
            if decision.success and decision.data.get('should_move', False):
                # Execute movement
                await self._execute_movement(npc_id, decision.data)
                
                # Store decision in memory for learning
                await self._store_movement_decision(npc_id, decision.data, context)
                
                logger.info(f"NPC {npc_id} moved after interaction: {decision.data.get('action_type', 'unknown')}")
            else:
                logger.debug(f"NPC {npc_id} decided not to move after interaction")
        
        except Exception as e:
            logger.error(f"Error in post-interaction movement for NPC {npc_id}: {e}", exc_info=True)

    async def trigger_movement_on_npc_interaction_end(
        self,
        npc_id: str,
        other_npc_id: str,
        interaction_context: Dict[str, Any]
    ):
        """
        Trigger NPC movement decision after NPC-to-NPC interaction ends
        Event-driven trigger: NPC decides next action after talking to another NPC
        """
        if not settings.npc_movement_enabled or settings.npc_movement_mode != "event_driven":
            return

        if not settings.npc_to_npc_interactions_enabled:
            return

        logger.info(f"Triggering post-NPC-interaction movement for NPC {npc_id} "
                   f"after interaction with NPC {other_npc_id}")

        try:
            # Get NPC data
            npc_data = await self._get_npc_data(npc_id)
            if not npc_data or not npc_data.get('can_move', True):
                return

            # Build agent context with NPC interaction history
            context = await self._build_movement_context(
                npc_data,
                trigger_type="post_npc_interaction",
                trigger_data={
                    "other_npc_id": other_npc_id,
                    "interaction_type": interaction_context.get("interaction_type"),
                    "interaction_outcome": interaction_context.get("outcome"),
                    "relationship_change": interaction_context.get("relationship_change", 0.0)
                }
            )

            # Use instant response for high-priority NPC interactions
            priority = EventPriority.HIGH if interaction_context.get("important", False) else EventPriority.NORMAL

            async def movement_handler(event_data):
                decision = await self.decision_agent.process(context)
                if decision.success and decision.data.get('should_move', False):
                    await self._execute_movement(npc_id, decision.data)
                    await self._store_movement_decision(npc_id, decision.data, context)
                    logger.info(f"NPC {npc_id} moved after NPC interaction: "
                              f"{decision.data.get('action_type', 'unknown')}")

            # Handle via instant response system
            await instant_response_manager.handle_event(
                event_type="npc_interaction_end",
                event_data={"npc_id": npc_id, "other_npc_id": other_npc_id},
                handler=movement_handler,
                priority=priority
            )

        except Exception as e:
            logger.error(f"Error in post-NPC-interaction movement for NPC {npc_id}: {e}", exc_info=True)

    async def check_and_update_idle_npcs(self):
        """
        Check for idle NPCs and trigger movement decisions
        Called periodically by scheduler in event-driven mode
        """
        if not settings.npc_movement_enabled or settings.npc_movement_mode != "event_driven":
            return
        
        logger.debug("Checking for idle NPCs...")
        
        try:
            # Get all active NPCs
            active_npcs = await self._get_active_npcs()
            idle_timeout = settings.npc_movement_idle_timeout
            current_time = datetime.utcnow()
            
            idle_count = 0
            moved_count = 0
            
            for npc in active_npcs:
                npc_id = npc['npc_id']
                
                # Check if NPC has been idle long enough
                last_activity = await self._get_last_activity_time(npc_id)
                if last_activity and (current_time - last_activity).total_seconds() < idle_timeout:
                    continue  # Not idle long enough
                
                idle_count += 1
                
                # Build context for idle movement decision
                context = await self._build_movement_context(
                    npc,
                    trigger_type="idle_timeout",
                    trigger_data={"idle_duration": (current_time - last_activity).total_seconds() if last_activity else idle_timeout}
                )
                
                # Agent decides if idle NPC should move
                decision = await self.decision_agent.process(context)
                
                if decision.success and decision.data.get('should_move', False):
                    await self._execute_movement(npc_id, decision.data)
                    await self._store_movement_decision(npc_id, decision.data, context)
                    moved_count += 1
            
            if idle_count > 0:
                logger.info(f"Idle NPC check: {idle_count} idle NPCs found, {moved_count} decided to move")
        
        except Exception as e:
            logger.error(f"Error checking idle NPCs: {e}", exc_info=True)
    
    async def update_all_npc_movements_fixed(self):
        """
        Update all NPC movements in fixed interval mode
        Called periodically by scheduler
        """
        if not settings.npc_movement_enabled or settings.npc_movement_mode != "fixed_interval":
            return
        
        logger.debug("Fixed interval NPC movement update...")
        
        try:
            active_npcs = await self._get_active_npcs()
            moved_count = 0
            
            for npc in active_npcs:
                npc_id = npc['npc_id']
                
                # Build context
                context = await self._build_movement_context(
                    npc,
                    trigger_type="fixed_interval",
                    trigger_data={"interval": settings.npc_movement_update_interval}
                )
                
                # Agent decides movement
                decision = await self.decision_agent.process(context)
                
                if decision.success and decision.data.get('should_move', False):
                    await self._execute_movement(npc_id, decision.data)
                    await self._store_movement_decision(npc_id, decision.data, context)
                    moved_count += 1
            
            logger.info(f"Fixed interval update: {moved_count}/{len(active_npcs)} NPCs moved")
        
        except Exception as e:
            logger.error(f"Error in fixed interval movement update: {e}", exc_info=True)

    # ========================================================================
    # Helper Methods
    # ========================================================================

    async def _get_npc_data(self, npc_id: str) -> Optional[Dict[str, Any]]:
        """Get NPC data from cache or database"""
        try:
            # Try cache first (DragonflyDB)
            npc_key = f"npc:{npc_id}"
            npc_data = await db.get(npc_key)

            if npc_data:
                return npc_data

            # Fallback to PostgreSQL
            query = "SELECT * FROM npcs WHERE npc_id = $1"
            result = await postgres_db.fetch_one(query, npc_id)

            if result:
                npc_data = dict(result)
                # Cache for 5 minutes
                await db.set(npc_key, npc_data, expire=300)
                return npc_data

            return None

        except Exception as e:
            logger.error(f"Error getting NPC data for {npc_id}: {e}")
            return None

    async def _get_active_npcs(self) -> List[Dict[str, Any]]:
        """Get all active NPCs that can move"""
        try:
            # Get from cache if available
            cache_key = "active_npcs:movable"
            cached = await db.get(cache_key)

            if cached:
                return cached

            # Query PostgreSQL
            query = """
                SELECT * FROM npcs
                WHERE can_move = true
                AND is_active = true
                ORDER BY last_activity_time ASC
                LIMIT 100
            """
            results = await postgres_db.fetch_all(query)

            npcs = [dict(row) for row in results]

            # Cache for 1 minute
            await db.set(cache_key, npcs, expire=60)

            return npcs

        except Exception as e:
            logger.error(f"Error getting active NPCs: {e}")
            return []

    async def _get_last_activity_time(self, npc_id: str) -> Optional[datetime]:
        """Get last activity time for NPC"""
        try:
            cache_key = f"npc:{npc_id}:last_activity"
            cached = await db.get(cache_key)

            if cached:
                return datetime.fromisoformat(cached)

            # Query database
            query = "SELECT last_activity_time FROM npcs WHERE npc_id = $1"
            result = await postgres_db.fetch_one(query, npc_id)

            if result and result['last_activity_time']:
                return result['last_activity_time']

            return None

        except Exception as e:
            logger.error(f"Error getting last activity time for {npc_id}: {e}")
            return None

    async def _build_movement_context(
        self,
        npc_data: Dict[str, Any],
        trigger_type: str,
        trigger_data: Dict[str, Any]
    ) -> AgentContext:
        """
        Build agent context for movement decision
        Includes: NPC personality, current state, world state, movement history
        """
        npc_id = npc_data['npc_id']

        # Get movement history for learning
        movement_history = await self._get_movement_history(npc_id, limit=settings.agent_context_window_size)

        # Get nearby players/NPCs
        nearby_entities = await self._get_nearby_entities(npc_data)

        # Get world state
        world_state = await self._get_world_state(npc_data.get('map_name', 'prontera'))

        context = AgentContext(
            npc_id=npc_id,
            npc_name=npc_data.get('name', 'Unknown'),
            npc_personality=npc_data.get('personality', {}),
            current_state=npc_data.get('state', {}),
            world_state=world_state,
            trigger_type=trigger_type,
            trigger_data=trigger_data,
            movement_history=movement_history,
            nearby_entities=nearby_entities,
            learning_enabled=settings.agent_learning_enabled,
            adaptive_behavior=settings.npc_adaptive_behavior
        )

        return context

    async def _get_movement_history(self, npc_id: str, limit: int = 10) -> List[Dict[str, Any]]:
        """Get recent movement decisions for learning"""
        try:
            if not settings.agent_learning_enabled:
                return []

            query = """
                SELECT * FROM npc_movement_history
                WHERE npc_id = $1
                ORDER BY timestamp DESC
                LIMIT $2
            """
            results = await postgres_db.fetch_all(query, npc_id, limit)
            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting movement history for {npc_id}: {e}")
            return []

    async def _get_nearby_entities(self, npc_data: Dict[str, Any]) -> Dict[str, Any]:
        """Get nearby players and NPCs using spatial query"""
        try:
            npc_x = npc_data.get('current_x', 0)
            npc_y = npc_data.get('current_y', 0)
            map_name = npc_data.get('current_map', '')
            distance_threshold = 10  # cells

            # Query nearby NPCs using Manhattan distance
            npc_query = """
                SELECT npc_id, name, current_x, current_y,
                       ABS(current_x - $1) + ABS(current_y - $2) as distance
                FROM npcs
                WHERE current_map = $3
                  AND npc_id != $4
                  AND ABS(current_x - $1) + ABS(current_y - $2) <= $5
                ORDER BY distance
                LIMIT 20
            """

            nearby_npcs = await postgres_db.fetch_all(
                npc_query,
                npc_x, npc_y, map_name, npc_data.get('npc_id'), distance_threshold
            )

            # Query nearby players (from cache or recent interactions)
            # Note: Player positions are typically cached in Redis
            cache_key = f"players:map:{map_name}"
            cached_players = await db.get(cache_key)

            nearby_players = []
            if cached_players:
                for player in cached_players:
                    px, py = player.get('x', 0), player.get('y', 0)
                    distance = abs(px - npc_x) + abs(py - npc_y)
                    if distance <= distance_threshold:
                        nearby_players.append({
                            **player,
                            'distance': distance
                        })

            return {
                "players": nearby_players[:10],  # Limit to 10 nearest
                "npcs": [dict(row) for row in nearby_npcs],
                "distance_threshold": distance_threshold
            }

        except Exception as e:
            logger.error(f"Error getting nearby entities: {e}", exc_info=True)
            return {
                "players": [],
                "npcs": [],
                "distance_threshold": 10
            }

    async def _get_world_state(self, map_name: str) -> Dict[str, Any]:
        """Get current world state for the map"""
        try:
            cache_key = f"world:state:{map_name}"
            cached = await db.get(cache_key)

            if cached:
                return cached

            # Default world state
            return {
                "map_name": map_name,
                "time_of_day": "day",
                "weather": "clear",
                "player_count": 0
            }

        except Exception as e:
            logger.error(f"Error getting world state for {map_name}: {e}")
            return {}

    async def _execute_movement(self, npc_id: str, decision_data: Dict[str, Any]):
        """Execute NPC movement based on agent decision"""
        try:
            # Create NPC action from decision
            action = NPCAction(
                action_type=decision_data.get('action_type', 'wander'),
                target_x=decision_data.get('target_x'),
                target_y=decision_data.get('target_y'),
                parameters=decision_data.get('parameters', {})
            )

            # Translate to bridge command
            command = translate_action_to_bridge_command(action, npc_id)

            # Push to rAthena via callback endpoint
            await self._push_to_rathena(command)

            # Update last movement time
            self.last_movement_times[npc_id] = datetime.utcnow()

            # Update NPC state in database
            await self._update_npc_position(npc_id, decision_data)

        except Exception as e:
            logger.error(f"Error executing movement for NPC {npc_id}: {e}")

    async def _push_to_rathena(self, command: Dict[str, Any]):
        """
        Push movement command to rAthena bridge
        Phase 8B: Uses Redis Pub/Sub for async communication (10-20x faster)
        Falls back to HTTP if Pub/Sub fails
        """
        try:
            npc_id = command.get("npc_id")
            action_type = command.get("action_type", "move")

            # Phase 8B: Try Pub/Sub first (async, much faster)
            pubsub = get_pubsub_manager()

            if action_type == "move":
                success = await pubsub.publish_npc_movement(
                    npc_id=npc_id,
                    target_x=command.get("target_x"),
                    target_y=command.get("target_y"),
                    target_map=command.get("parameters", {}).get("map"),
                    speed=command.get("parameters", {}).get("speed", 150)
                )
            else:
                # Generic action publish
                success = await pubsub.publish_npc_action(
                    npc_id=npc_id,
                    action_type=action_type,
                    action_data={
                        "target_x": command.get("target_x"),
                        "target_y": command.get("target_y"),
                        "parameters": command.get("parameters", {})
                    }
                )

            if success:
                logger.info(f"[PUBSUB] Successfully published {action_type} for NPC {npc_id}")
                return

            # Fallback to HTTP if Pub/Sub fails
            logger.warning(f"[PUBSUB] Failed, falling back to HTTP for NPC {npc_id}")
            await self._push_to_rathena_http(command)

        except Exception as e:
            logger.error(f"Error pushing to rAthena: {e}", exc_info=True)
            # Try HTTP fallback
            try:
                await self._push_to_rathena_http(command)
            except Exception as fallback_error:
                logger.error(f"HTTP fallback also failed: {fallback_error}")

    async def _push_to_rathena_http(self, command: Dict[str, Any]):
        """HTTP fallback for pushing commands to rAthena bridge"""
        try:
            import httpx
            from ..config import settings

            # Build bridge URL (web-server bridge)
            bridge_url = f"http://{settings.rathena_bridge_host}:{settings.rathena_bridge_port}"
            endpoint = "/ai/npc/execute-action"

            # Prepare request payload
            payload = {
                "npc_id": command.get("npc_id"),
                "action": {
                    "type": command.get("action_type", "move"),
                    "target_x": command.get("target_x"),
                    "target_y": command.get("target_y"),
                    "parameters": command.get("parameters", {})
                }
            }

            # Make HTTP POST request to bridge
            async with httpx.AsyncClient(timeout=5.0) as client:
                response = await client.post(
                    f"{bridge_url}{endpoint}",
                    json=payload,
                    headers={"Content-Type": "application/json"}
                )

                if response.status_code == 200:
                    logger.info(f"[HTTP] Successfully pushed movement command to rAthena for NPC {command.get('npc_id')}")
                else:
                    logger.warning(f"[HTTP] Failed to push to rAthena: HTTP {response.status_code} - {response.text}")

        except Exception as e:
            logger.error(f"[HTTP] Error pushing to rAthena: {e}", exc_info=True)

    async def _update_npc_position(self, npc_id: str, decision_data: Dict[str, Any]):
        """Update NPC position in database"""
        try:
            query = """
                UPDATE npcs
                SET current_x = $1, current_y = $2, last_activity_time = $3
                WHERE npc_id = $4
            """
            await postgres_db.execute(
                query,
                decision_data.get('target_x'),
                decision_data.get('target_y'),
                datetime.utcnow(),
                npc_id
            )
        except Exception as e:
            logger.error(f"Error updating NPC position for {npc_id}: {e}")

    async def _store_movement_decision(self, npc_id: str, decision_data: Dict[str, Any], context: AgentContext):
        """Store movement decision in history for learning"""
        if not settings.agent_decision_logging_enabled:
            return

        try:
            query = """
                INSERT INTO npc_movement_history
                (npc_id, decision_data, context_data, timestamp)
                VALUES ($1, $2, $3, $4)
            """
            await postgres_db.execute(
                query,
                npc_id,
                decision_data,
                context.dict(),
                datetime.utcnow()
            )
        except Exception as e:
            logger.error(f"Error storing movement decision for {npc_id}: {e}")


# Global instance
npc_movement_manager = NPCMovementManager()


# Public functions for scheduler
async def check_and_update_idle_npcs():
    """Check for idle NPCs and trigger movement decisions"""
    await npc_movement_manager.check_and_update_idle_npcs()


async def update_all_npc_movements_fixed():
    """Update all NPC movements in fixed interval mode"""
    await npc_movement_manager.update_all_npc_movements_fixed()


async def trigger_movement_on_interaction_end(npc_id: str, player_id: str, interaction_context: Dict[str, Any]):
    """Trigger NPC movement after player interaction ends"""
    await npc_movement_manager.trigger_movement_on_interaction_end(npc_id, player_id, interaction_context)


