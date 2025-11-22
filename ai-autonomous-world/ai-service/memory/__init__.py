"""
Memory Management Package

Provides memory systems for NPCs including:
- OpenMemory SDK integration with DragonflyDB fallback
- Memory types (episodic, semantic, procedural)
- Relationship tracking and management
"""

from .memory_types import (
    MemoryType,
    MemoryImportance,
    EpisodicMemory,
    SemanticMemory,
    ProceduralMemory,
    MemoryDecay
)
from .openmemory_manager import OpenMemoryManager
from .relationship_manager import RelationshipManager

__all__ = [
    'MemoryType',
    'MemoryImportance',
    'EpisodicMemory',
    'SemanticMemory',
    'ProceduralMemory',
    'MemoryDecay',
    'OpenMemoryManager',
    'RelationshipManager'
]