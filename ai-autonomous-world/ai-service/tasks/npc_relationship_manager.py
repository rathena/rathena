"""
NPC Relationship Manager

Background task managing NPC relationship dynamics and decay.
Runs periodically to update relationships based on interactions and time.

Features:
- Relationship decay for inactive NPCs
- Relationship strengthening with frequent interactions
- Social graph updates
- Relationship event generation
- Configurable update intervals
"""

import asyncio
import logging
from typing import Dict, List, Optional, Any
from datetime import datetime, timedelta
from dataclasses import dataclass

logger = logging.getLogger(__name__)


@dataclass
class RelationshipUpdate:
    """Represents a relationship update."""
    npc_id: str
    target_id: str
    old_affinity: float
    new_affinity: float
    reason: str
    timestamp: datetime


class NPCRelationshipManager:
    """
    Manages periodic relationship updates and decay.
    
    Processes NPC relationships in the background, applying decay
    to inactive relationships and strengthening active ones.
    """
    
    def __init__(
        self,
        db,
        interval_seconds: int = 300,
        decay_rate: float = 0.01,
        strengthen_rate: float = 0.05
    ):
        """
        Initialize relationship manager.
        
        Args:
            db: Database connection
            interval_seconds: Processing interval (default: 300s = 5min)
            decay_rate: Decay rate for inactive relationships
            strengthen_rate: Strengthen rate for active relationships
        """
        self.db = db
        self.interval = interval_seconds
        self.decay_rate = decay_rate
        self.strengthen_rate = strengthen_rate
        self.running = False
        self.task: Optional[asyncio.Task] = None
        
        # Metrics
        self.metrics = {
            'relationships_processed': 0,
            'relationships_decayed': 0,
            'relationships_strengthened': 0,
            'events_generated': 0,
            'errors': 0
        }
    
    async def start(self):
        """Start periodic relationship processing."""
        if self.running:
            logger.warning("Relationship manager already running")
            return
        
        self.running = True
        self.task = asyncio.create_task(self._run_loop())
        logger.info(f"NPC relationship manager started (interval: {self.interval}s)")
    
    async def stop(self):
        """Stop relationship processing."""
        self.running = False
        
        if self.task:
            self.task.cancel()
            try:
                await self.task
            except asyncio.CancelledError:
                pass
        
        logger.info("NPC relationship manager stopped")
    
    async def _run_loop(self):
        """Main processing loop."""
        while self.running:
            try:
                await self._process_relationships()
                await asyncio.sleep(self.interval)
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Relationship processing error: {e}")
                self.metrics['errors'] += 1
                await asyncio.sleep(60)  # Wait before retry
    
    async def _process_relationships(self):
        """Process all active relationships."""
        try:
            # Fetch active relationships from database
            relationships = await self._fetch_active_relationships()
            
            logger.debug(f"Processing {len(relationships)} relationships")
            
            updates = []
            
            for rel in relationships:
                update = await self._process_single_relationship(rel)
                if update:
                    updates.append(update)
            
            # Apply updates to database
            if updates:
                await self._apply_updates(updates)
            
            # Generate events for significant changes
            await self._generate_relationship_events(updates)
            
            # Update metrics
            self.metrics['relationships_processed'] += len(relationships)
            
            logger.info(
                f"Processed {len(relationships)} relationships, "
                f"{len(updates)} updates applied"
            )
            
        except Exception as e:
            logger.error(f"Failed to process relationships: {e}")
            raise
    
    async def _fetch_active_relationships(self) -> List[Dict[str, Any]]:
        """
        Fetch active relationships from database.
        
        Returns:
            List of relationship records
        """
        # TODO: Implement actual database query
        # Mock implementation
        return [
            {
                'npc_id': 'npc-1',
                'target_id': 'player-1',
                'affinity': 0.7,
                'last_interaction': datetime.utcnow() - timedelta(hours=2),
                'interaction_count': 5
            },
            {
                'npc_id': 'npc-1',
                'target_id': 'npc-2',
                'affinity': 0.5,
                'last_interaction': datetime.utcnow() - timedelta(days=1),
                'interaction_count': 2
            }
        ]
    
    async def _process_single_relationship(
        self,
        rel: Dict[str, Any]
    ) -> Optional[RelationshipUpdate]:
        """
        Process a single relationship.
        
        Args:
            rel: Relationship record
            
        Returns:
            RelationshipUpdate or None if no change
        """
        old_affinity = rel['affinity']
        new_affinity = old_affinity
        reason = ""
        
        # Check time since last interaction
        time_since_interaction = datetime.utcnow() - rel['last_interaction']
        
        # Apply decay for inactive relationships
        if time_since_interaction > timedelta(hours=24):
            decay = self.decay_rate * (time_since_interaction.days + 1)
            new_affinity = max(0.0, old_affinity - decay)
            reason = f"Decay due to {time_since_interaction.days} days inactivity"
            self.metrics['relationships_decayed'] += 1
        
        # Strengthen relationships with recent interactions
        elif time_since_interaction < timedelta(hours=1):
            if rel['interaction_count'] >= 3:
                new_affinity = min(1.0, old_affinity + self.strengthen_rate)
                reason = f"Strengthened due to {rel['interaction_count']} recent interactions"
                self.metrics['relationships_strengthened'] += 1
        
        # Return update if changed
        if abs(new_affinity - old_affinity) > 0.001:
            return RelationshipUpdate(
                npc_id=rel['npc_id'],
                target_id=rel['target_id'],
                old_affinity=old_affinity,
                new_affinity=new_affinity,
                reason=reason,
                timestamp=datetime.utcnow()
            )
        
        return None
    
    async def _apply_updates(self, updates: List[RelationshipUpdate]):
        """
        Apply relationship updates to database.
        
        Args:
            updates: List of relationship updates
        """
        # TODO: Implement batch database update
        # Mock implementation
        for update in updates:
            logger.debug(
                f"Update: {update.npc_id} -> {update.target_id}: "
                f"{update.old_affinity:.2f} -> {update.new_affinity:.2f} "
                f"({update.reason})"
            )
    
    async def _generate_relationship_events(
        self,
        updates: List[RelationshipUpdate]
    ):
        """
        Generate events for significant relationship changes.
        
        Args:
            updates: List of relationship updates
        """
        for update in updates:
            # Generate event for significant changes
            delta = abs(update.new_affinity - update.old_affinity)
            
            if delta >= 0.1:
                event = {
                    'type': 'relationship_change',
                    'npc_id': update.npc_id,
                    'target_id': update.target_id,
                    'old_value': update.old_affinity,
                    'new_value': update.new_affinity,
                    'change': delta,
                    'reason': update.reason,
                    'timestamp': update.timestamp.isoformat()
                }
                
                # TODO: Publish event to event bus
                logger.info(f"Relationship event: {event}")
                self.metrics['events_generated'] += 1
    
    async def force_update(self):
        """Force an immediate relationship update (for testing/admin)."""
        logger.info("Forcing relationship update")
        await self._process_relationships()
    
    def get_metrics(self) -> Dict[str, int]:
        """
        Get processing metrics.
        
        Returns:
            Dict with metrics
        """
        return self.metrics.copy()
    
    def reset_metrics(self):
        """Reset all metrics to zero."""
        self.metrics = {
            'relationships_processed': 0,
            'relationships_decayed': 0,
            'relationships_strengthened': 0,
            'events_generated': 0,
            'errors': 0
        }