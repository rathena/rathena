"""
MVP AI Router - API endpoints for MVP/Boss Monster AI
"""

from fastapi import APIRouter, HTTPException, BackgroundTasks
from pydantic import BaseModel, Field
from typing import Dict, Any, Optional, List
from loguru import logger
from datetime import datetime

try:
    from models.mvp_behavior import (
        MVPState, MVPAction, MVPBehaviorConfig, MVPCombatMemory,
        MVPSpawnCondition, MVPPersonality, MVPBehaviorState, CombatPattern
    )
    from agents.mvp_agent import MVPAgent
    from services.mvp_spawn_manager import get_spawn_manager
    from database import db
    from config import settings
except ImportError:
    from models.mvp_behavior import (
        MVPState, MVPAction, MVPBehaviorConfig, MVPCombatMemory,
        MVPSpawnCondition, MVPPersonality, MVPBehaviorState, CombatPattern
    )
    from agents.mvp_agent import MVPAgent
    from services.mvp_spawn_manager import get_spawn_manager
    from database import db
    from config import settings


router = APIRouter(prefix="/ai/mvp", tags=["mvp"])


# Request/Response Models
class MVPActionRequest(BaseModel):
    """Request for MVP action decision"""
    mvp_id: str
    mvp_state: Dict[str, Any]
    engaged_players: List[Dict[str, Any]] = Field(default_factory=list)
    world_state: Dict[str, Any] = Field(default_factory=dict)


class MVPActionResponse(BaseModel):
    """Response with MVP action"""
    success: bool
    action: Optional[Dict[str, Any]] = None
    new_state: Optional[str] = None
    detected_strategy: Optional[Dict[str, Any]] = None
    error: Optional[str] = None


class MVPRegisterRequest(BaseModel):
    """Request to register new MVP"""
    mvp_id: str
    mvp_name: str
    mob_id: int
    base_level: int
    spawn_map: str
    spawn_x: int
    spawn_y: int
    personality: Optional[Dict[str, float]] = None
    spawn_interval_minutes: int = 120
    dynamic_spawn: bool = False


class MVPSpawnRequest(BaseModel):
    """Request to spawn MVP"""
    mvp_id: str
    force_spawn: bool = False


class MVPCombatReportRequest(BaseModel):
    """Report combat encounter for learning"""
    mvp_id: str
    combat_memory: Dict[str, Any]


# In-memory MVP agent registry
mvp_agents: Dict[str, MVPAgent] = {}


@router.post("/action", response_model=MVPActionResponse)
async def get_mvp_action(request: MVPActionRequest):
    """
    Get next action for an MVP based on current state
    
    Args:
        request: MVP state and context
    
    Returns:
        Next action to take
    """
    try:
        # Get or create MVP agent
        if request.mvp_id not in mvp_agents:
            logger.warning(f"MVP {request.mvp_id} not registered, using default config")
            # Create default config
            mvp_config = MVPBehaviorConfig(
                mvp_id=request.mvp_id,
                mvp_name=request.mvp_id,
                mob_id=request.mvp_state.get("mob_id", 1001),
                base_level=request.mvp_state.get("base_level", 99),
                spawn_map="prontera",
                spawn_x=150,
                spawn_y=150
            )
            
            # Create agent
            from agents.base_agent import AgentContext
            mvp_agents[request.mvp_id] = MVPAgent(
                agent_id=request.mvp_id,
                llm_provider=None,  # Will use default
                config={"verbose": False},
                mvp_config=mvp_config
            )
        
        agent = mvp_agents[request.mvp_id]
        
        # Build context
        from agents.base_agent import AgentContext
        context = AgentContext(
            npc_id=request.mvp_id,
            player_id="system",
            interaction_type="mvp_action",
            current_state={
                "mvp_state": request.mvp_state,
                "engaged_players": request.engaged_players,
                "world_state": request.world_state
            }
        )
        
        # Get action from agent
        response = await agent.process(context)
        
        if response.success:
            return MVPActionResponse(
                success=True,
                action=response.data.get("action"),
                new_state=response.data.get("new_state"),
                detected_strategy=response.data.get("detected_strategy")
            )
        else:
            return MVPActionResponse(
                success=False,
                error=response.error
            )
    
    except Exception as e:
        logger.error(f"Error getting MVP action: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/register")
async def register_mvp(request: MVPRegisterRequest):
    """
    Register a new MVP with AI capabilities
    
    Args:
        request: MVP configuration
    
    Returns:
        Registration status
    """
    try:
        # Create personality
        personality = MVPPersonality(**request.personality) if request.personality else MVPPersonality()
        
        # Create MVP config
        mvp_config = MVPBehaviorConfig(
            mvp_id=request.mvp_id,
            mvp_name=request.mvp_name,
            mob_id=request.mob_id,
            base_level=request.base_level,
            spawn_map=request.spawn_map,
            spawn_x=request.spawn_x,
            spawn_y=request.spawn_y,
            personality=personality,
            spawn_interval_minutes=request.spawn_interval_minutes,
            dynamic_spawn=request.dynamic_spawn
        )
        
        # Create MVP agent
        from agents.base_agent import AgentContext
        mvp_agents[request.mvp_id] = MVPAgent(
            agent_id=request.mvp_id,
            llm_provider=None,
            config={"verbose": settings.verbose_logging},
            mvp_config=mvp_config
        )
        
        # Register with spawn manager if dynamic spawn enabled
        if request.dynamic_spawn:
            spawn_condition = MVPSpawnCondition(mvp_id=request.mvp_id)
            spawn_manager = get_spawn_manager()
            spawn_manager.register_mvp(mvp_config, spawn_condition)
        
        logger.info(f"Registered MVP: {request.mvp_name} ({request.mvp_id})")

        return {
            "success": True,
            "message": f"MVP {request.mvp_name} registered successfully",
            "mvp_id": request.mvp_id
        }

    except Exception as e:
        logger.error(f"Error registering MVP: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/spawn")
async def spawn_mvp(request: MVPSpawnRequest, background_tasks: BackgroundTasks):
    """
    Trigger MVP spawn

    Args:
        request: Spawn request
        background_tasks: FastAPI background tasks

    Returns:
        Spawn status
    """
    try:
        spawn_manager = get_spawn_manager()

        # Get MVP config
        if request.mvp_id not in mvp_agents:
            raise HTTPException(
                status_code=404,
                detail=f"MVP {request.mvp_id} not registered"
            )

        agent = mvp_agents[request.mvp_id]
        mvp_config = agent.mvp_config

        # Check spawn conditions unless force spawn
        if not request.force_spawn:
            world_state = await spawn_manager._get_world_state()
            can_spawn = await spawn_manager.check_spawn_conditions(
                request.mvp_id,
                world_state
            )

            if not can_spawn:
                return {
                    "success": False,
                    "message": "Spawn conditions not met",
                    "mvp_id": request.mvp_id
                }

        # Spawn MVP
        success = await spawn_manager.spawn_mvp(request.mvp_id, mvp_config)

        if success:
            return {
                "success": True,
                "message": f"MVP {mvp_config.mvp_name} spawned",
                "mvp_id": request.mvp_id,
                "location": {
                    "map": mvp_config.spawn_map,
                    "x": mvp_config.spawn_x,
                    "y": mvp_config.spawn_y
                }
            }
        else:
            return {
                "success": False,
                "message": "Failed to spawn MVP",
                "mvp_id": request.mvp_id
            }

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error spawning MVP: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/despawn/{mvp_id}")
async def despawn_mvp(mvp_id: str, reason: str = "manual"):
    """
    Despawn an MVP

    Args:
        mvp_id: MVP identifier
        reason: Reason for despawn

    Returns:
        Despawn status
    """
    try:
        spawn_manager = get_spawn_manager()
        await spawn_manager.despawn_mvp(mvp_id, reason)

        return {
            "success": True,
            "message": f"MVP {mvp_id} despawned",
            "reason": reason
        }

    except Exception as e:
        logger.error(f"Error despawning MVP: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/combat/report")
async def report_combat(request: MVPCombatReportRequest):
    """
    Report combat encounter for MVP learning

    Args:
        request: Combat report

    Returns:
        Learning status
    """
    try:
        if request.mvp_id not in mvp_agents:
            raise HTTPException(
                status_code=404,
                detail=f"MVP {request.mvp_id} not registered"
            )

        agent = mvp_agents[request.mvp_id]

        # Create combat memory
        combat_memory = MVPCombatMemory(**request.combat_memory)

        # Let agent learn from encounter
        await agent.learn_from_encounter(combat_memory)

        logger.info(
            f"MVP {request.mvp_id} learned from combat: "
            f"{combat_memory.outcome}"
        )

        return {
            "success": True,
            "message": "Combat report processed",
            "mvp_id": request.mvp_id,
            "outcome": combat_memory.outcome
        }

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error processing combat report: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/list")
async def list_mvps():
    """
    List all registered MVPs

    Returns:
        List of MVPs
    """
    try:
        mvp_list = []

        for mvp_id, agent in mvp_agents.items():
            config = agent.mvp_config
            mvp_list.append({
                "mvp_id": mvp_id,
                "mvp_name": config.mvp_name,
                "mob_id": config.mob_id,
                "level": config.base_level,
                "spawn_location": {
                    "map": config.spawn_map,
                    "x": config.spawn_x,
                    "y": config.spawn_y
                },
                "dynamic_spawn": config.dynamic_spawn,
                "personality": {
                    "aggression": config.personality.aggression,
                    "intelligence": config.personality.intelligence,
                    "adaptability": config.personality.adaptability
                }
            })

        return {
            "success": True,
            "count": len(mvp_list),
            "mvps": mvp_list
        }

    except Exception as e:
        logger.error(f"Error listing MVPs: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/state/{mvp_id}")
async def get_mvp_state(mvp_id: str):
    """
    Get current state of an MVP

    Args:
        mvp_id: MVP identifier

    Returns:
        MVP state
    """
    try:
        spawn_manager = get_spawn_manager()

        # Check if MVP is spawned
        is_spawned = mvp_id in spawn_manager.active_mvps

        if is_spawned:
            spawn_time = spawn_manager.active_mvps[mvp_id]
            duration = (datetime.utcnow() - spawn_time).total_seconds()

            # Get spawn data from database
            import json
            spawn_data_str = await db.client.get(f"mvp_spawn:{mvp_id}")
            spawn_data = json.loads(spawn_data_str) if spawn_data_str else {}

            return {
                "success": True,
                "mvp_id": mvp_id,
                "spawned": True,
                "spawn_time": spawn_time.isoformat(),
                "duration_seconds": duration,
                "spawn_data": spawn_data
            }
        else:
            return {
                "success": True,
                "mvp_id": mvp_id,
                "spawned": False
            }

    except Exception as e:
        logger.error(f"Error getting MVP state: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))

