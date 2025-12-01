"""
Bridge Command Utilities
Translates AI decisions into rAthena-compatible commands for the Bridge Layer
"""

from typing import Dict, Any, Optional, List
from loguru import logger

try:
    from models.npc import NPCAction, NPCPosition
except ModuleNotFoundError:
    from models.npc import NPCAction, NPCPosition


def translate_action_to_bridge_command(action: NPCAction, npc_id: str) -> Dict[str, Any]:
    """
    Translate an NPC action into a Bridge Layer command
    
    Args:
        action: NPCAction to translate
        npc_id: NPC identifier
        
    Returns:
        Bridge command dictionary
    """
    logger.debug(f"Translating action '{action.action_type}' for NPC {npc_id}")
    
    if action.action_type in ["wander", "patrol", "exploration", "return_home", "goal_directed", "social", "retreat"]:
        return translate_movement_action(action, npc_id)
    elif action.action_type == "idle":
        return translate_idle_action(action, npc_id)
    elif action.action_type == "emote":
        return translate_emote_action(action, npc_id)
    elif action.action_type == "dialogue":
        return translate_dialogue_action(action, npc_id)
    else:
        logger.warning(f"Unknown action type: {action.action_type}, using generic command")
        return {
            "command_type": "generic",
            "npc_id": npc_id,
            "action_type": action.action_type,
            "parameters": action.action_data
        }


def translate_movement_action(action: NPCAction, npc_id: str) -> Dict[str, Any]:
    """
    Translate movement action to rAthena npc_movenpc command
    
    Args:
        action: Movement action
        npc_id: NPC identifier
        
    Returns:
        Movement command for Bridge Layer
    """
    action_data = action.action_data
    target_position = action_data.get("target_position", {})
    
    if not target_position:
        logger.warning(f"Movement action for NPC {npc_id} has no target position")
        return {
            "command_type": "idle",
            "npc_id": npc_id,
            "duration": 5
        }
    
    # Extract target coordinates
    target_map = target_position.get("map", "prontera")
    target_x = target_position.get("x", 150)
    target_y = target_position.get("y", 150)
    
    logger.info(f"NPC {npc_id} movement command: {action.action_type} to {target_map}({target_x},{target_y})")
    
    return {
        "command_type": "npc_move",
        "npc_id": npc_id,
        "target_map": target_map,
        "target_x": int(target_x),
        "target_y": int(target_y),
        "movement_type": action_data.get("movement_type", "walk"),
        "movement_reason": action_data.get("movement_reason", "AI decision"),
        "priority": action.priority,
        "execution_params": {
            "use_pathfinding": True,
            "interrupt_on_interaction": True,
            **action.execution_params
        }
    }


def translate_idle_action(action: NPCAction, npc_id: str) -> Dict[str, Any]:
    """
    Translate idle action to rAthena command
    
    Args:
        action: Idle action
        npc_id: NPC identifier
        
    Returns:
        Idle command for Bridge Layer
    """
    duration = action.action_data.get("duration", 5)
    
    logger.debug(f"NPC {npc_id} idle command: {duration} seconds")
    
    return {
        "command_type": "idle",
        "npc_id": npc_id,
        "duration": duration,
        "priority": action.priority
    }


def translate_emote_action(action: NPCAction, npc_id: str) -> Dict[str, Any]:
    """
    Translate emote action to rAthena command
    
    Args:
        action: Emote action
        npc_id: NPC identifier
        
    Returns:
        Emote command for Bridge Layer
    """
    emote_type = action.action_data.get("emote_type", "wave")
    
    # Map emote names to rAthena emote IDs
    emote_map = {
        "wave": 1,
        "bow": 2,
        "sit": 3,
        "stand": 4,
        "laugh": 5,
        "cry": 6,
        "angry": 7,
        "surprise": 8,
        "think": 9,
        "sweat": 10
    }
    
    emote_id = emote_map.get(emote_type, 1)
    
    logger.debug(f"NPC {npc_id} emote command: {emote_type} (ID: {emote_id})")
    
    return {
        "command_type": "emote",
        "npc_id": npc_id,
        "emote_id": emote_id,
        "emote_type": emote_type,
        "priority": action.priority
    }


def translate_dialogue_action(action: NPCAction, npc_id: str) -> Dict[str, Any]:
    """
    Translate dialogue action to rAthena command
    
    Args:
        action: Dialogue action
        npc_id: NPC identifier
        
    Returns:
        Dialogue command for Bridge Layer
    """
    dialogue_text = action.action_data.get("text", "...")
    
    logger.debug(f"NPC {npc_id} dialogue command: {dialogue_text[:50]}...")
    
    return {
        "command_type": "dialogue",
        "npc_id": npc_id,
        "text": dialogue_text,
        "priority": action.priority
    }

