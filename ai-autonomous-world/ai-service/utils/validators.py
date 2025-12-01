"""
Validation Utilities

Provides decorators and helpers for validating inputs, checking entities,
and enforcing rate limits. Eliminates ~200 lines of duplicated validation logic.

Features:
- Entity validation decorators (@validate_npc, @validate_player)
- Rate limiting decorator (@check_rate_limit)
- Message length validation
- NPC state retrieval helpers
- Personality building helpers
"""

import functools
import logging
from typing import Callable, Optional, Any, Dict
from datetime import datetime, timedelta

from fastapi import HTTPException, status
from pydantic import ValidationError

logger = logging.getLogger(__name__)


# ============================================================================
# VALIDATION DECORATORS
# ============================================================================

def validate_npc(func: Callable) -> Callable:
    """
    Decorator to validate NPC exists and retrieve state.
    
    Usage:
        @validate_npc
        async def my_endpoint(npc_id: str, npc_state=None):
            # npc_state will be populated if NPC exists
            pass
    """
    @functools.wraps(func)
    async def wrapper(*args, **kwargs):
        npc_id = kwargs.get('npc_id')
        if not npc_id:
            # Try to get from args
            for arg in args:
                if isinstance(arg, str) and arg.startswith('npc-'):
                    npc_id = arg
                    break
        
        if not npc_id:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="NPC ID is required"
            )
        
        # TODO: Fetch NPC state from database
        # npc_state = await get_npc_state(npc_id)
        # if not npc_state:
        #     raise HTTPException(
        #         status_code=status.HTTP_404_NOT_FOUND,
        #         detail=f"NPC {npc_id} not found"
        #     )
        
        # Mock NPC state for now
        npc_state = {
            'npc_id': npc_id,
            'name': f'NPC-{npc_id}',
            'is_active': True,
            'is_alive': True
        }
        
        kwargs['npc_state'] = npc_state
        return await func(*args, **kwargs)
    
    return wrapper


def validate_player(func: Callable) -> Callable:
    """
    Decorator to validate player exists and retrieve state.
    
    Usage:
        @validate_player
        async def my_endpoint(player_id: str, player_state=None):
            # player_state will be populated if player exists
            pass
    """
    @functools.wraps(func)
    async def wrapper(*args, **kwargs):
        player_id = kwargs.get('player_id')
        if not player_id:
            # Try to get from args
            for arg in args:
                if isinstance(arg, str) and arg.startswith('player-'):
                    player_id = arg
                    break
        
        if not player_id:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Player ID is required"
            )
        
        # TODO: Fetch player state from database
        # player_state = await get_player_state(player_id)
        # if not player_state:
        #     raise HTTPException(
        #         status_code=status.HTTP_404_NOT_FOUND,
        #         detail=f"Player {player_id} not found"
        #     )
        
        # Mock player state for now
        player_state = {
            'player_id': player_id,
            'username': f'Player-{player_id}',
            'is_active': True
        }
        
        kwargs['player_state'] = player_state
        return await func(*args, **kwargs)
    
    return wrapper


# Simple in-memory rate limit tracker (replace with Redis in production)
_rate_limit_tracker: Dict[str, list] = {}

def check_rate_limit(
    requests_per_minute: int = 60,
    identifier_key: str = 'client_ip'
):
    """
    Decorator to enforce rate limiting.
    
    Args:
        requests_per_minute: Maximum requests allowed per minute
        identifier_key: Key to identify requester (e.g., 'client_ip', 'user_id')
    
    Usage:
        @check_rate_limit(requests_per_minute=30)
        async def my_endpoint(request: Request):
            pass
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            # Get identifier
            identifier = kwargs.get(identifier_key, 'unknown')
            
            # Clean old entries
            now = datetime.utcnow()
            cutoff = now - timedelta(minutes=1)
            
            if identifier in _rate_limit_tracker:
                _rate_limit_tracker[identifier] = [
                    ts for ts in _rate_limit_tracker[identifier] if ts > cutoff
                ]
            else:
                _rate_limit_tracker[identifier] = []
            
            # Check rate limit
            if len(_rate_limit_tracker[identifier]) >= requests_per_minute:
                logger.warning(f"Rate limit exceeded for {identifier}")
                raise HTTPException(
                    status_code=status.HTTP_429_TOO_MANY_REQUESTS,
                    detail="Rate limit exceeded. Please try again later."
                )
            
            # Record this request
            _rate_limit_tracker[identifier].append(now)
            
            return await func(*args, **kwargs)
        
        return wrapper
    return decorator


def validate_message_length(
    min_length: int = 1,
    max_length: int = 1000
):
    """
    Decorator to validate message length.
    
    Args:
        min_length: Minimum message length
        max_length: Maximum message length
    
    Usage:
        @validate_message_length(min_length=1, max_length=500)
        async def send_message(message: str):
            pass
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            message = kwargs.get('message')
            if not message:
                # Try to get from args
                for arg in args:
                    if isinstance(arg, str) and len(arg) > 0:
                        message = arg
                        break
            
            if not message:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail="Message is required"
                )
            
            message_len = len(message)
            
            if message_len < min_length:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"Message too short (min: {min_length} characters)"
                )
            
            if message_len > max_length:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"Message too long (max: {max_length} characters)"
                )
            
            return await func(*args, **kwargs)
        
        return wrapper
    return decorator


# ============================================================================
# VALIDATION HELPERS
# ============================================================================

async def get_npc_state_safe(npc_id: str) -> Optional[Dict[str, Any]]:
    """
    Safely retrieve NPC state without raising exceptions.
    
    Args:
        npc_id: NPC identifier
        
    Returns:
        NPC state dict or None if not found
    """
    try:
        # TODO: Implement actual database fetch
        # from database import db_manager
        # npc_state = await db_manager.postgres.fetch_npc(npc_id)
        # return npc_state
        
        # Mock implementation
        return {
            'npc_id': npc_id,
            'name': f'NPC-{npc_id}',
            'is_active': True,
            'is_alive': True,
            'location': 'prontera',
            'position_x': 150.0,
            'position_y': 200.0
        }
    except Exception as e:
        logger.error(f"Failed to fetch NPC state: {e}")
        return None


async def get_player_state_safe(player_id: str) -> Optional[Dict[str, Any]]:
    """
    Safely retrieve player state without raising exceptions.
    
    Args:
        player_id: Player identifier
        
    Returns:
        Player state dict or None if not found
    """
    try:
        # TODO: Implement actual database fetch
        # from database import db_manager
        # player_state = await db_manager.postgres.fetch_player(player_id)
        # return player_state
        
        # Mock implementation
        return {
            'player_id': player_id,
            'username': f'Player-{player_id}',
            'is_active': True,
            'level': 50,
            'location': 'prontera'
        }
    except Exception as e:
        logger.error(f"Failed to fetch player state: {e}")
        return None


def build_npc_personality_context(npc_state: Dict[str, Any]) -> str:
    """
    Build personality context string for LLM from NPC state.
    
    Args:
        npc_state: NPC state dictionary
        
    Returns:
        str: Formatted personality context
    """
    personality = npc_state.get('personality', {})
    physical = npc_state.get('physical', {})
    background = npc_state.get('background', {})
    
    traits = personality.get('traits', [])
    traits_str = ', '.join(traits) if traits else 'neutral'
    
    context_parts = [
        f"You are {npc_state.get('name', 'an NPC')}",
    ]
    
    if npc_state.get('title'):
        context_parts.append(f", {npc_state.get('title')}")
    
    context_parts.append(".")
    
    if physical:
        race = physical.get('race', 'unknown')
        age = physical.get('age', 'unknown')
        context_parts.append(f" You are a {age}-year-old {race}.")
    
    if background:
        occupation = background.get('occupation', 'wanderer')
        context_parts.append(f" You work as a {occupation}.")
    
    if traits:
        context_parts.append(f" Your personality: {traits_str}.")
    
    mood = npc_state.get('emotion', 'neutral')
    context_parts.append(f" Current mood: {mood}.")
    
    return ''.join(context_parts)


def validate_coordinates(
    x: float,
    y: float,
    min_x: float = 0.0,
    max_x: float = 1000.0,
    min_y: float = 0.0,
    max_y: float = 1000.0
) -> bool:
    """
    Validate if coordinates are within bounds.
    
    Args:
        x: X coordinate
        y: Y coordinate
        min_x: Minimum X boundary
        max_x: Maximum X boundary
        min_y: Minimum Y boundary
        max_y: Maximum Y boundary
        
    Returns:
        bool: True if valid
    """
    return min_x <= x <= max_x and min_y <= y <= max_y


def sanitize_user_input(text: str, max_length: int = 1000) -> str:
    """
    Sanitize user input to prevent injection attacks.
    
    Args:
        text: Input text
        max_length: Maximum allowed length
        
    Returns:
        str: Sanitized text
    """
    if not text:
        return ""
    
    # Truncate to max length
    text = text[:max_length]
    
    # Remove control characters
    text = ''.join(char for char in text if ord(char) >= 32 or char in '\n\r\t')
    
    # Strip leading/trailing whitespace
    text = text.strip()
    
    return text


def validate_entity_id(entity_id: str, entity_type: str = 'entity') -> bool:
    """
    Validate entity ID format.
    
    Args:
        entity_id: Entity identifier
        entity_type: Type of entity (npc, player, etc.)
        
    Returns:
        bool: True if valid format
    """
    if not entity_id:
        return False
    
    # Check format: {type}-{id}
    expected_prefix = f"{entity_type}-"
    
    if not entity_id.startswith(expected_prefix):
        return False
    
    # Check ID part is alphanumeric
    id_part = entity_id[len(expected_prefix):]
    return id_part.isalnum() and len(id_part) > 0


def validate_location_name(location: str) -> bool:
    """
    Validate location name format.
    
    Args:
        location: Location name
        
    Returns:
        bool: True if valid
    """
    if not location:
        return False
    
    # Allow alphanumeric, underscore, hyphen
    return all(c.isalnum() or c in '_-' for c in location) and len(location) <= 50


# ============================================================================
# BATCH VALIDATION
# ============================================================================

async def validate_entities_batch(
    entity_ids: list[str],
    entity_type: str = 'npc'
) -> Dict[str, bool]:
    """
    Validate multiple entities in batch.
    
    Args:
        entity_ids: List of entity IDs
        entity_type: Type of entities
        
    Returns:
        Dict mapping entity_id to validation result
    """
    results = {}
    
    for entity_id in entity_ids:
        # Validate format
        if not validate_entity_id(entity_id, entity_type):
            results[entity_id] = False
            continue
        
        # Check existence
        if entity_type == 'npc':
            state = await get_npc_state_safe(entity_id)
        elif entity_type == 'player':
            state = await get_player_state_safe(entity_id)
        else:
            state = None
        
        results[entity_id] = state is not None
    
    return results