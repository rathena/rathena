"""
NPC-to-NPC Relationship Manager
Handles NPC interactions, relationship building, and information sharing
"""

import asyncio
import json
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, Any
from loguru import logger
import uuid

from config import settings
from database import db, postgres_db
from models.npc_relationship import (
    NPCRelationship,
    NPCInteraction,
    SharedInformation,
    NPCProximityEvent,
    NPCInteractionContext,
    NPCInteractionDecision
)


class NPCRelationshipManager:
    """Manages NPC-to-NPC relationships and interactions"""

    def __init__(self):
        self.db = db  # DragonflyDB/Redis client
        self.postgres_db = postgres_db  # PostgreSQL manager
        self._proximity_check_task = None
        self._relationship_decay_task = None

    async def initialize(self):
        """Initialize the relationship manager"""
        # Database connections are already initialized in main.py
        logger.info("✓ NPC Relationship Manager initialized")
    
    async def start_background_tasks(self):
        """Start background tasks for proximity checking and relationship decay"""
        if settings.npc_to_npc_interactions_enabled:
            # Start proximity checking task
            self._proximity_check_task = asyncio.create_task(
                self._proximity_check_loop()
            )
            logger.info(f"✓ NPC proximity checking started (interval: {settings.npc_proximity_check_interval}s)")
            
            # Start relationship decay task
            if settings.npc_relationship_enabled:
                self._relationship_decay_task = asyncio.create_task(
                    self._relationship_decay_loop()
                )
                logger.info(f"✓ NPC relationship decay started (rate: {settings.npc_relationship_decay_rate}/day)")
    
    async def stop_background_tasks(self):
        """Stop background tasks"""
        if self._proximity_check_task:
            self._proximity_check_task.cancel()
        if self._relationship_decay_task:
            self._relationship_decay_task.cancel()
        logger.info("✓ NPC relationship background tasks stopped")
    
    async def _proximity_check_loop(self):
        """Periodically check for NPCs in proximity"""
        while True:
            try:
                await asyncio.sleep(settings.npc_proximity_check_interval)
                await self.check_npc_proximity()
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in proximity check loop: {e}")
    
    async def _relationship_decay_loop(self):
        """Periodically decay NPC relationships"""
        while True:
            try:
                # Run once per day
                await asyncio.sleep(86400)
                await self.decay_relationships()
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in relationship decay loop: {e}")
    
    async def check_npc_proximity(self):
        """Check for NPCs in proximity and trigger interactions"""
        try:
            # Get all active NPCs from cache
            npc_keys = await self.db.client.keys("npc:*:state")
            
            if len(npc_keys) < 2:
                return  # Need at least 2 NPCs
            
            # Get NPC positions
            npc_positions = {}
            for key in npc_keys:
                npc_id = key.decode().split(":")[1]
                state_data = await self.db.client.get(key)
                if state_data:
                    state = json.loads(state_data)
                    if "position" in state:
                        npc_positions[npc_id] = state["position"]
            
            # Check proximity between all NPC pairs
            proximity_events = []
            npc_ids = list(npc_positions.keys())
            
            for i in range(len(npc_ids)):
                for j in range(i + 1, len(npc_ids)):
                    npc_id_1 = npc_ids[i]
                    npc_id_2 = npc_ids[j]
                    
                    pos1 = npc_positions[npc_id_1]
                    pos2 = npc_positions[npc_id_2]
                    
                    # Calculate distance
                    distance = ((pos1["x"] - pos2["x"]) ** 2 + (pos1["y"] - pos2["y"]) ** 2) ** 0.5
                    
                    if distance <= settings.npc_interaction_range:
                        # NPCs are in proximity
                        event = await self._create_proximity_event(
                            npc_id_1, npc_id_2, distance, pos1
                        )
                        proximity_events.append(event)
            
            # Process proximity events
            for event in proximity_events:
                if event.should_interact:
                    await self.trigger_npc_interaction(
                        event.npc_id_1,
                        event.npc_id_2,
                        "proximity"
                    )
            
            if proximity_events:
                logger.info(f"Processed {len(proximity_events)} NPC proximity events, "
                          f"{sum(1 for e in proximity_events if e.should_interact)} triggered interactions")
        
        except Exception as e:
            logger.error(f"Error checking NPC proximity: {e}")
    
    async def _create_proximity_event(
        self,
        npc_id_1: str,
        npc_id_2: str,
        distance: float,
        location: Dict[str, Any]
    ) -> NPCProximityEvent:
        """Create a proximity event and determine if interaction should occur"""
        # Get relationship
        relationship = await self.get_relationship(npc_id_1, npc_id_2)
        
        # Calculate interaction probability based on relationship
        base_probability = 0.3  # 30% base chance
        
        if relationship:
            # Adjust probability based on relationship value
            # Positive relationships increase probability, negative decrease
            relationship_modifier = relationship.relationship_value / 200.0  # -0.5 to 0.5
            interaction_probability = max(0.0, min(1.0, base_probability + relationship_modifier))
        else:
            interaction_probability = base_probability
        
        # Determine if interaction should occur
        import random
        should_interact = random.random() < interaction_probability
        
        return NPCProximityEvent(
            event_id=str(uuid.uuid4()),
            npc_id_1=npc_id_1,
            npc_id_2=npc_id_2,
            distance=distance,
            location=location,
            should_interact=should_interact,
            interaction_probability=interaction_probability
        )

    async def trigger_npc_interaction(
        self,
        npc_id_1: str,
        npc_id_2: str,
        trigger_type: str = "proximity"
    ):
        """Trigger an interaction between two NPCs"""
        try:
            logger.info(f"Triggering NPC interaction: {npc_id_1} <-> {npc_id_2} (trigger: {trigger_type})")

            # Build interaction context
            context = await self._build_interaction_context(npc_id_1, npc_id_2)

            # Make interaction decision (using decision agent)
            decision = await self._make_interaction_decision(context)

            if decision.should_interact:
                # Execute interaction
                interaction = await self._execute_interaction(
                    npc_id_1, npc_id_2, decision, context
                )

                # Update relationship
                if settings.npc_relationship_enabled:
                    await self.update_relationship(
                        npc_id_1,
                        npc_id_2,
                        decision.relationship_change,
                        decision.trust_change
                    )

                # Share information if applicable
                if decision.information_to_share and settings.npc_information_sharing_enabled:
                    await self._share_information(
                        npc_id_1, npc_id_2, decision.information_to_share
                    )

                logger.info(f"✓ NPC interaction completed: {interaction.interaction_id}")
                return interaction
            else:
                logger.debug(f"NPCs decided not to interact: {decision.reasoning}")
                return None

        except Exception as e:
            logger.error(f"Error triggering NPC interaction: {e}")
            return None

    async def _build_interaction_context(
        self,
        npc_id_1: str,
        npc_id_2: str
    ) -> NPCInteractionContext:
        """Build context for NPC interaction decision"""
        # Get NPC states
        npc_1_state = await self._get_npc_state(npc_id_1)
        npc_2_state = await self._get_npc_state(npc_id_2)

        # Get relationship
        relationship = await self.get_relationship(npc_id_1, npc_id_2)

        # Get recent interactions
        recent_interactions = await self._get_recent_interactions(npc_id_1, npc_id_2, limit=5)

        # Get shared information
        shared_info = await self._get_shared_information(npc_id_1, npc_id_2)

        # Calculate proximity distance
        pos1 = npc_1_state.get("position", {"x": 0, "y": 0})
        pos2 = npc_2_state.get("position", {"x": 0, "y": 0})
        distance = ((pos1["x"] - pos2["x"]) ** 2 + (pos1["y"] - pos2["y"]) ** 2) ** 0.5

        # Get world state
        world_state = await self._get_world_state()

        return NPCInteractionContext(
            npc_id_1=npc_id_1,
            npc_id_2=npc_id_2,
            npc_1_personality=npc_1_state.get("personality", {}),
            npc_2_personality=npc_2_state.get("personality", {}),
            current_relationship=relationship,
            recent_interactions=recent_interactions,
            shared_information=shared_info,
            proximity_distance=distance,
            location=pos1,
            world_state=world_state,
            npc_1_goals=npc_1_state.get("goals", []),
            npc_2_goals=npc_2_state.get("goals", []),
            npc_1_current_activity=npc_1_state.get("current_activity"),
            npc_2_current_activity=npc_2_state.get("current_activity")
        )

    async def _make_interaction_decision(
        self,
        context: NPCInteractionContext
    ) -> NPCInteractionDecision:
        """Make decision about NPC interaction using agent"""
        # This will be implemented with the decision optimization system
        # For now, return a simple decision

        # Check if this is a simple rule-based decision (Tier 1)
        if settings.rule_based_decisions_enabled:
            tier1_decision = await self._try_rule_based_decision(context)
            if tier1_decision:
                return tier1_decision

        # Check decision cache (Tier 2)
        if settings.decision_cache_enabled:
            cached_decision = await self._try_cached_decision(context)
            if cached_decision:
                return cached_decision

        # Full LLM decision (Tier 4) - will be implemented in Phase 4
        # For now, return a default decision
        return NPCInteractionDecision(
            should_interact=True,
            interaction_type="conversation",
            interaction_duration=30.0,
            dialogue=f"Hello, {context.npc_id_2}!",
            relationship_change=1.0,
            trust_change=0.5,
            reasoning="Default interaction decision (full implementation pending)",
            complexity_tier=4,
            cached=False
        )

    async def _try_rule_based_decision(
        self,
        context: NPCInteractionContext
    ) -> Optional[NPCInteractionDecision]:
        """Try to make a rule-based decision (Tier 1 - no LLM)"""
        # Rule 1: If NPCs are enemies, don't interact
        if context.current_relationship and context.current_relationship.relationship_value < -50:
            return NPCInteractionDecision(
                should_interact=False,
                reasoning="Rule-based: NPCs are enemies (relationship < -50)",
                complexity_tier=1,
                cached=False
            )

        # Rule 2: If both NPCs are busy with critical activities, don't interact
        critical_activities = ["combat", "quest_critical", "trading"]
        if (context.npc_1_current_activity in critical_activities and
            context.npc_2_current_activity in critical_activities):
            return NPCInteractionDecision(
                should_interact=False,
                reasoning="Rule-based: Both NPCs busy with critical activities",
                complexity_tier=1,
                cached=False
            )

        return None  # No rule-based decision possible

    async def _try_cached_decision(
        self,
        context: NPCInteractionContext
    ) -> Optional[NPCInteractionDecision]:
        """Try to find a cached similar decision (Tier 2)"""
        # This will be implemented in Phase 4 with embedding similarity
        return None

    async def _execute_interaction(
        self,
        npc_id_1: str,
        npc_id_2: str,
        decision: NPCInteractionDecision,
        context: NPCInteractionContext
    ) -> NPCInteraction:
        """Execute the NPC interaction"""
        interaction_id = str(uuid.uuid4())

        interaction = NPCInteraction(
            interaction_id=interaction_id,
            npc_id_1=npc_id_1,
            npc_id_2=npc_id_2,
            interaction_type=decision.interaction_type or "conversation",
            location=context.location,
            duration=decision.interaction_duration,
            outcome="positive" if decision.relationship_change > 0 else "negative" if decision.relationship_change < 0 else "neutral",
            relationship_change=decision.relationship_change,
            information_shared=decision.information_to_share
        )

        # Store interaction in database
        await self._store_interaction(interaction)

        # Update interaction count in cache
        await self._increment_interaction_count(npc_id_1, npc_id_2)

        return interaction

    async def _share_information(
        self,
        from_npc_id: str,
        to_npc_id: str,
        information_ids: List[str]
    ):
        """Share information from one NPC to another"""
        for info_id in information_ids:
            # Get information
            info = await self._get_information(info_id)
            if info:
                # Add to_npc_id to holders
                if to_npc_id not in info.current_holder_npc_ids:
                    info.current_holder_npc_ids.append(to_npc_id)
                    info.spread_count += 1
                    await self._update_information(info)
                    logger.debug(f"Information {info_id} shared: {from_npc_id} -> {to_npc_id}")

    async def get_relationship(
        self,
        npc_id_1: str,
        npc_id_2: str
    ) -> Optional[NPCRelationship]:
        """Get relationship between two NPCs"""
        # Ensure consistent ordering
        npc_ids = sorted([npc_id_1, npc_id_2])

        # Try cache first
        cache_key = f"npc_relationship:{npc_ids[0]}:{npc_ids[1]}"
        cached = await self.db.get(cache_key)
        if cached:
            return NPCRelationship(**json.loads(cached))

        # Query database
        if self.postgres_pool:
            async with self.postgres_pool.acquire() as conn:
                row = await conn.fetchrow(
                    """
                    SELECT * FROM npc_relationships
                    WHERE npc_id_1 = $1 AND npc_id_2 = $2
                    """,
                    npc_ids[0], npc_ids[1]
                )
                if row:
                    relationship = NPCRelationship(**dict(row))
                    # Cache for 5 minutes
                    await self.db.setex(
                        cache_key,
                        300,
                        json.dumps(relationship.model_dump(), default=str)
                    )
                    return relationship

        return None

    async def update_relationship(
        self,
        npc_id_1: str,
        npc_id_2: str,
        relationship_change: float,
        trust_change: float = 0.0
    ):
        """Update relationship between two NPCs"""
        npc_ids = sorted([npc_id_1, npc_id_2])

        # Get current relationship or create new
        relationship = await self.get_relationship(npc_ids[0], npc_ids[1])

        if relationship:
            # Update existing
            relationship.relationship_value = max(-100.0, min(100.0,
                relationship.relationship_value + relationship_change))
            relationship.trust_level = max(0.0, min(100.0,
                relationship.trust_level + trust_change))
            relationship.updated_at = datetime.utcnow()
        else:
            # Create new
            relationship = NPCRelationship(
                npc_id_1=npc_ids[0],
                npc_id_2=npc_ids[1],
                relationship_value=relationship_change,
                trust_level=50.0 + trust_change
            )

        # Store in database
        if self.postgres_pool:
            async with self.postgres_pool.acquire() as conn:
                await conn.execute(
                    """
                    INSERT INTO npc_relationships (
                        npc_id_1, npc_id_2, relationship_type, relationship_value,
                        trust_level, interaction_count, last_interaction, created_at, updated_at
                    ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
                    ON CONFLICT (npc_id_1, npc_id_2)
                    DO UPDATE SET
                        relationship_value = $4,
                        trust_level = $5,
                        last_interaction = $7,
                        updated_at = $9
                    """,
                    relationship.npc_id_1,
                    relationship.npc_id_2,
                    relationship.relationship_type,
                    relationship.relationship_value,
                    relationship.trust_level,
                    relationship.interaction_count,
                    relationship.last_interaction,
                    relationship.created_at,
                    relationship.updated_at
                )

        # Update cache
        cache_key = f"npc_relationship:{npc_ids[0]}:{npc_ids[1]}"
        await self.db.setex(
            cache_key,
            300,
            json.dumps(relationship.model_dump(), default=str)
        )

        logger.debug(f"Updated relationship: {npc_ids[0]} <-> {npc_ids[1]} "
                    f"(value: {relationship.relationship_value:.1f}, trust: {relationship.trust_level:.1f})")

    async def decay_relationships(self):
        """Decay all NPC relationships towards neutral"""
        if not settings.npc_relationship_enabled:
            return

        try:
            if self.postgres_pool:
                async with self.postgres_pool.acquire() as conn:
                    # Decay all relationships
                    result = await conn.execute(
                        """
                        UPDATE npc_relationships
                        SET relationship_value = relationship_value * (1 - $1),
                            updated_at = $2
                        WHERE ABS(relationship_value) > 1.0
                        """,
                        settings.npc_relationship_decay_rate,
                        datetime.utcnow()
                    )
                    logger.info(f"✓ Decayed NPC relationships (rate: {settings.npc_relationship_decay_rate})")
        except Exception as e:
            logger.error(f"Error decaying relationships: {e}")

    # Helper methods

    async def _get_npc_state(self, npc_id: str) -> Dict[str, Any]:
        """Get NPC state from cache"""
        state_data = await self.db.get(f"npc:{npc_id}:state")
        if state_data:
            return json.loads(state_data)
        return {}

    async def _get_world_state(self) -> Dict[str, Any]:
        """Get current world state"""
        world_data = await self.db.get("world:state")
        if world_data:
            return json.loads(world_data)
        return {"time": "day", "weather": "clear", "player_count": 0}

    async def _get_recent_interactions(
        self,
        npc_id_1: str,
        npc_id_2: str,
        limit: int = 5
    ) -> List[NPCInteraction]:
        """Get recent interactions between two NPCs"""
        if not self.postgres_pool:
            return []

        npc_ids = sorted([npc_id_1, npc_id_2])

        async with self.postgres_pool.acquire() as conn:
            rows = await conn.fetch(
                """
                SELECT * FROM npc_interactions
                WHERE (npc_id_1 = $1 AND npc_id_2 = $2)
                   OR (npc_id_1 = $2 AND npc_id_2 = $1)
                ORDER BY timestamp DESC
                LIMIT $3
                """,
                npc_ids[0], npc_ids[1], limit
            )
            return [NPCInteraction(**dict(row)) for row in rows]

    async def _get_shared_information(
        self,
        npc_id_1: str,
        npc_id_2: str
    ) -> List[SharedInformation]:
        """Get information shared between two NPCs"""
        if not self.postgres_pool:
            return []

        async with self.postgres_pool.acquire() as conn:
            rows = await conn.fetch(
                """
                SELECT * FROM shared_information
                WHERE $1 = ANY(current_holder_npc_ids)
                  AND $2 = ANY(current_holder_npc_ids)
                  AND (expiry IS NULL OR expiry > $3)
                ORDER BY importance DESC, created_at DESC
                LIMIT 10
                """,
                npc_id_1, npc_id_2, datetime.utcnow()
            )
            return [SharedInformation(**dict(row)) for row in rows]

    async def _store_interaction(self, interaction: NPCInteraction):
        """Store interaction in database"""
        if not self.postgres_pool:
            return

        async with self.postgres_pool.acquire() as conn:
            await conn.execute(
                """
                INSERT INTO npc_interactions (
                    interaction_id, npc_id_1, npc_id_2, interaction_type,
                    location, duration, outcome, relationship_change,
                    information_shared, timestamp, metadata
                ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
                """,
                interaction.interaction_id,
                interaction.npc_id_1,
                interaction.npc_id_2,
                interaction.interaction_type,
                json.dumps(interaction.location),
                interaction.duration,
                interaction.outcome,
                interaction.relationship_change,
                interaction.information_shared,
                interaction.timestamp,
                json.dumps(interaction.metadata)
            )

    async def _increment_interaction_count(self, npc_id_1: str, npc_id_2: str):
        """Increment interaction count for relationship"""
        if not self.postgres_pool:
            return

        npc_ids = sorted([npc_id_1, npc_id_2])

        async with self.postgres_pool.acquire() as conn:
            await conn.execute(
                """
                UPDATE npc_relationships
                SET interaction_count = interaction_count + 1,
                    last_interaction = $3
                WHERE npc_id_1 = $1 AND npc_id_2 = $2
                """,
                npc_ids[0], npc_ids[1], datetime.utcnow()
            )

    async def _get_information(self, info_id: str) -> Optional[SharedInformation]:
        """Get shared information by ID"""
        if not self.postgres_pool:
            return None

        async with self.postgres_pool.acquire() as conn:
            row = await conn.fetchrow(
                "SELECT * FROM shared_information WHERE info_id = $1",
                info_id
            )
            if row:
                return SharedInformation(**dict(row))
        return None

    async def _update_information(self, info: SharedInformation):
        """Update shared information"""
        if not self.postgres_pool:
            return

        async with self.postgres_pool.acquire() as conn:
            await conn.execute(
                """
                UPDATE shared_information
                SET current_holder_npc_ids = $2,
                    spread_count = $3
                WHERE info_id = $1
                """,
                info.info_id,
                info.current_holder_npc_ids,
                info.spread_count
            )


# Global instance
npc_relationship_manager = NPCRelationshipManager()

