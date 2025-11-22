"""
Relationship Manager

Tracks and manages relationships between NPCs and between NPCs and players.

Features:
- Affinity scoring (-1.0 to +1.0)
- Relationship types (friend, rival, family, romantic, neutral)
- Dynamic relationship evolution based on interactions
- PostgreSQL persistence with DragonflyDB cache
- Relationship decay over time without interaction
"""

import json
import logging
from enum import Enum
from dataclasses import dataclass, field, asdict
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple

from prometheus_client import Counter, Gauge

logger = logging.getLogger(__name__)


class RelationshipType(str, Enum):
    """Types of relationships between entities."""
    
    NEUTRAL = "neutral"         # Default, no strong feelings
    FRIEND = "friend"           # Positive relationship
    RIVAL = "rival"             # Competitive/antagonistic
    ENEMY = "enemy"             # Hostile
    FAMILY = "family"           # Family bonds
    ROMANTIC = "romantic"       # Romantic relationship
    ALLY = "ally"               # Strategic alliance
    MENTOR = "mentor"           # Teacher/student
    APPRENTICE = "apprentice"   # Student of mentor


@dataclass
class Relationship:
    """
    Relationship between two entities.
    
    Tracks affinity, type, and interaction history.
    """
    
    entity_a: str  # NPC or player ID
    entity_b: str  # NPC or player ID
    affinity: float = 0.0  # -1.0 (hostile) to +1.0 (close)
    relationship_type: RelationshipType = RelationshipType.NEUTRAL
    interaction_count: int = 0
    last_interaction: Optional[datetime] = None
    formed_at: datetime = field(default_factory=datetime.utcnow)
    notes: List[str] = field(default_factory=list)
    metadata: Dict[str, any] = field(default_factory=dict)
    
    def __post_init__(self):
        """Validate and normalize relationship data."""
        # Ensure affinity is in valid range
        self.affinity = max(-1.0, min(1.0, self.affinity))
        
        # Order entity IDs consistently
        if self.entity_a > self.entity_b:
            self.entity_a, self.entity_b = self.entity_b, self.entity_a
    
    def get_relationship_key(self) -> str:
        """Get unique key for this relationship."""
        return f"{self.entity_a}:{self.entity_b}"
    
    def days_since_interaction(self) -> float:
        """Get days since last interaction."""
        if not self.last_interaction:
            return (datetime.utcnow() - self.formed_at).total_seconds() / 86400
        return (datetime.utcnow() - self.last_interaction).total_seconds() / 86400
    
    def to_dict(self) -> Dict[str, any]:
        """Convert to dictionary."""
        return {
            'entity_a': self.entity_a,
            'entity_b': self.entity_b,
            'affinity': self.affinity,
            'relationship_type': self.relationship_type.value,
            'interaction_count': self.interaction_count,
            'last_interaction': self.last_interaction.isoformat() if self.last_interaction else None,
            'formed_at': self.formed_at.isoformat(),
            'notes': self.notes,
            'metadata': self.metadata
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, any]) -> 'Relationship':
        """Create from dictionary."""
        data['relationship_type'] = RelationshipType(data['relationship_type'])
        if data.get('last_interaction'):
            data['last_interaction'] = datetime.fromisoformat(data['last_interaction'])
        data['formed_at'] = datetime.fromisoformat(data['formed_at'])
        return cls(**data)


class RelationshipManager:
    """
    Manages relationships between entities.
    
    Provides relationship tracking, evolution, and persistence.
    """
    
    # Metrics
    _metrics_initialized = False
    
    def __init__(
        self,
        postgres_session = None,
        dragonfly_client = None,
        cache_ttl: int = 3600
    ):
        """
        Initialize Relationship Manager.
        
        Args:
            postgres_session: PostgreSQL session for persistence
            dragonfly_client: DragonflyDB client for caching
            cache_ttl: Cache TTL in seconds
        """
        self.postgres = postgres_session
        self.dragonfly = dragonfly_client
        self.cache_ttl = cache_ttl
        
        # In-memory cache for hot relationships
        self._relationship_cache: Dict[str, Relationship] = {}
        
        if not RelationshipManager._metrics_initialized:
            self._init_metrics()
            RelationshipManager._metrics_initialized = True
        
        logger.info("Relationship Manager initialized")
    
    @classmethod
    def _init_metrics(cls):
        """Initialize Prometheus metrics."""
        cls.relationship_updates = Counter(
            'relationship_updates_total',
            'Total relationship updates',
            ['operation', 'type']
        )
        
        cls.relationship_count = Gauge(
            'relationships_total',
            'Total active relationships',
            ['type']
        )
    
    async def get_relationship(
        self,
        entity_a: str,
        entity_b: str
    ) -> Optional[Relationship]:
        """
        Get relationship between two entities.
        
        Args:
            entity_a: First entity ID
            entity_b: Second entity ID
            
        Returns:
            Relationship or None if not exists
        """
        # Normalize order
        if entity_a > entity_b:
            entity_a, entity_b = entity_b, entity_a
        
        rel_key = f"{entity_a}:{entity_b}"
        
        # Check memory cache first
        if rel_key in self._relationship_cache:
            return self._relationship_cache[rel_key]
        
        # Check DragonflyDB cache
        if self.dragonfly:
            cached = self.dragonfly.get(f"relationship:{rel_key}")
            if cached:
                try:
                    rel = Relationship.from_dict(json.loads(cached))
                    self._relationship_cache[rel_key] = rel
                    return rel
                except Exception as e:
                    logger.warning(f"Failed to parse cached relationship: {e}")
        
        # Fetch from PostgreSQL
        # TODO: Implement actual database fetch
        # rel = await self._fetch_from_db(entity_a, entity_b)
        
        return None
    
    async def update_relationship(
        self,
        entity_a: str,
        entity_b: str,
        affinity_change: float,
        interaction_type: str,
        note: Optional[str] = None
    ) -> Relationship:
        """
        Update relationship based on interaction.
        
        Args:
            entity_a: First entity ID
            entity_b: Second entity ID
            affinity_change: Change in affinity (-1.0 to +1.0)
            interaction_type: Type of interaction
            note: Optional note about interaction
            
        Returns:
            Updated relationship
        """
        # Get or create relationship
        rel = await self.get_relationship(entity_a, entity_b)
        
        if not rel:
            rel = Relationship(
                entity_a=entity_a,
                entity_b=entity_b,
                affinity=0.0
            )
        
        # Update affinity (with diminishing returns)
        old_affinity = rel.affinity
        
        # Diminishing returns: harder to change extreme relationships
        resistance_factor = abs(rel.affinity) * 0.5
        effective_change = affinity_change * (1.0 - resistance_factor)
        
        rel.affinity += effective_change
        rel.affinity = max(-1.0, min(1.0, rel.affinity))
        
        # Update metadata
        rel.interaction_count += 1
        rel.last_interaction = datetime.utcnow()
        
        if note:
            rel.notes.append(f"{datetime.utcnow().isoformat()}: {note}")
            # Keep only last 10 notes
            rel.notes = rel.notes[-10:]
        
        # Update relationship type based on affinity
        rel.relationship_type = self._determine_relationship_type(
            rel.affinity,
            rel.relationship_type
        )
        
        # Persist
        await self._save_relationship(rel)
        
        # Metrics
        if hasattr(self, 'relationship_updates'):
            self.relationship_updates.labels(
                operation='update',
                type=rel.relationship_type.value
            ).inc()
        
        logger.debug(
            f"Relationship updated: {entity_a} <-> {entity_b} "
            f"(affinity: {old_affinity:.2f} → {rel.affinity:.2f})"
        )
        
        return rel
    
    def _determine_relationship_type(
        self,
        affinity: float,
        current_type: RelationshipType
    ) -> RelationshipType:
        """
        Determine relationship type based on affinity.
        
        Args:
            affinity: Current affinity score
            current_type: Current relationship type
            
        Returns:
            RelationshipType: New relationship type
        """
        # Preserve special relationships
        special_types = {
            RelationshipType.FAMILY,
            RelationshipType.MENTOR,
            RelationshipType.APPRENTICE
        }
        
        if current_type in special_types:
            return current_type
        
        # Determine type from affinity
        if affinity >= 0.7:
            return RelationshipType.FRIEND
        elif affinity >= 0.4:
            return RelationshipType.ALLY
        elif affinity <= -0.7:
            return RelationshipType.ENEMY
        elif affinity <= -0.3:
            return RelationshipType.RIVAL
        else:
            return RelationshipType.NEUTRAL
    
    async def _save_relationship(self, relationship: Relationship) -> None:
        """Save relationship to storage."""
        rel_key = relationship.get_relationship_key()
        
        # Update memory cache
        self._relationship_cache[rel_key] = relationship
        
        # Update DragonflyDB cache
        if self.dragonfly:
            try:
                self.dragonfly.setex(
                    f"relationship:{rel_key}",
                    self.cache_ttl,
                    json.dumps(relationship.to_dict())
                )
            except Exception as e:
                logger.error(f"Failed to cache relationship: {e}")
        
        # TODO: Save to PostgreSQL
        # await self._save_to_db(relationship)
    
    async def get_entity_relationships(
        self,
        entity_id: str,
        min_affinity: Optional[float] = None,
        relationship_types: Optional[List[RelationshipType]] = None
    ) -> List[Relationship]:
        """
        Get all relationships for an entity.
        
        Args:
            entity_id: Entity ID
            min_affinity: Minimum affinity filter
            relationship_types: Filter by relationship types
            
        Returns:
            List of relationships
        """
        # TODO: Implement database query
        # For now, return from cache
        relationships = []
        
        for rel in self._relationship_cache.values():
            if entity_id not in (rel.entity_a, rel.entity_b):
                continue
            
            if min_affinity is not None and rel.affinity < min_affinity:
                continue
            
            if relationship_types and rel.relationship_type not in relationship_types:
                continue
            
            relationships.append(rel)
        
        # Sort by affinity
        relationships.sort(key=lambda r: r.affinity, reverse=True)
        
        return relationships
    
    async def decay_relationships(self, entity_id: str) -> int:
        """
        Apply decay to relationships without recent interaction.
        
        Args:
            entity_id: Entity ID to process
            
        Returns:
            int: Number of relationships decayed
        """
        relationships = await self.get_entity_relationships(entity_id)
        decayed_count = 0
        
        for rel in relationships:
            days_inactive = rel.days_since_interaction()
            
            # Apply decay after 7 days of no interaction
            if days_inactive > 7:
                # Decay rate: 0.01 per day after threshold
                decay_amount = 0.01 * (days_inactive - 7)
                
                # Decay towards neutral (0.0)
                if rel.affinity > 0:
                    rel.affinity = max(0.0, rel.affinity - decay_amount)
                elif rel.affinity < 0:
                    rel.affinity = min(0.0, rel.affinity + decay_amount)
                
                await self._save_relationship(rel)
                decayed_count += 1
        
        if decayed_count > 0:
            logger.debug(
                f"Applied decay to {decayed_count} relationships for {entity_id}"
            )
        
        return decayed_count
    
    async def get_relationship_summary(
        self,
        entity_id: str
    ) -> Dict[str, any]:
        """
        Get relationship summary for an entity.
        
        Args:
            entity_id: Entity ID
            
        Returns:
            Summary statistics
        """
        relationships = await self.get_entity_relationships(entity_id)
        
        # Calculate statistics
        type_counts = {}
        affinity_sum = 0.0
        
        for rel in relationships:
            rel_type = rel.relationship_type.value
            type_counts[rel_type] = type_counts.get(rel_type, 0) + 1
            affinity_sum += rel.affinity
        
        avg_affinity = affinity_sum / len(relationships) if relationships else 0.0
        
        return {
            'entity_id': entity_id,
            'total_relationships': len(relationships),
            'average_affinity': avg_affinity,
            'type_distribution': type_counts,
            'friends': len([r for r in relationships if r.relationship_type == RelationshipType.FRIEND]),
            'enemies': len([r for r in relationships if r.relationship_type == RelationshipType.ENEMY]),
            'neutral': len([r for r in relationships if r.relationship_type == RelationshipType.NEUTRAL])
        }
    
    async def cleanup(self) -> None:
        """Cleanup resources."""
        self._relationship_cache.clear()
        logger.info("Relationship Manager cleaned up")