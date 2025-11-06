"""
Quest API endpoints for dynamic quest generation and management
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
from datetime import datetime
from typing import List, Optional

from ..models.quest import (
    Quest, QuestGenerationRequest, QuestGenerationResponse,
    QuestProgressUpdate, QuestCompletionRequest, QuestStatus
)
from ..models.npc import NPCPersonality
from ..database import db
from ..llm import get_llm_provider
from ..agents.base_agent import AgentContext
from ..agents.quest_agent import QuestAgent

router = APIRouter(prefix="/ai/quest", tags=["quest"])

# Global quest agent instance
_quest_agent = None


def get_quest_agent() -> QuestAgent:
    """Get or create the global quest agent instance"""
    global _quest_agent
    if _quest_agent is None:
        llm = get_llm_provider()
        config = {"verbose": False}
        _quest_agent = QuestAgent(
            agent_id="quest_001",
            llm_provider=llm,
            config=config
        )
        logger.info("Quest Agent initialized")
    return _quest_agent


@router.post("/generate", response_model=QuestGenerationResponse)
async def generate_quest(request: QuestGenerationRequest):
    """
    Generate a dynamic quest based on NPC and world state
    
    Uses AI to create contextual, engaging quests that fit the world.
    """
    try:
        logger.info(f"Generating quest for NPC: {request.npc_id}")
        
        # Get NPC state
        npc_state = await db.get_npc_state(request.npc_id)
        
        if not npc_state:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"NPC {request.npc_id} not found"
            )
        
        # Build NPC personality
        personality = NPCPersonality(
            openness=npc_state.get("openness", 0.5),
            conscientiousness=npc_state.get("conscientiousness", 0.5),
            extraversion=npc_state.get("extraversion", 0.5),
            agreeableness=npc_state.get("agreeableness", 0.5),
            neuroticism=npc_state.get("neuroticism", 0.5),
            moral_alignment=npc_state.get("moral_alignment", "true_neutral")
        )
        
        # Build agent context
        agent_context = AgentContext(
            npc_id=request.npc_id,
            npc_name=request.npc_name,
            personality=personality,
            current_state={
                "npc_class": request.npc_class,
                "player_level": request.player_level,
                "player_class": request.player_class,
                "location": npc_state.get("location", {}),
                "preferred_type": request.preferred_type,
                "preferred_difficulty": request.preferred_difficulty
            },
            world_state=request.world_state,
            recent_events=request.recent_events,
            timestamp=datetime.utcnow()
        )
        
        # Generate quest using Quest Agent
        quest_agent = get_quest_agent()
        result = await quest_agent.process(agent_context)
        
        if not result.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=f"Quest generation failed: {result.reasoning}"
            )
        
        # Extract quest from result
        quest_dict = result.data.get("quest")
        quest = Quest(**quest_dict)
        
        # Store quest in database
        await db.store_quest(quest.quest_id, quest.dict())
        
        logger.info(f"Quest generated and stored: {quest.quest_id}")
        
        return QuestGenerationResponse(
            success=True,
            quest=quest,
            generation_reasoning=result.reasoning
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Quest generation error: {e}")
        return QuestGenerationResponse(
            success=False,
            error=str(e)
        )


@router.get("/{quest_id}", response_model=Quest)
async def get_quest(quest_id: str):
    """Get quest details by ID"""
    try:
        quest_data = await db.get_quest(quest_id)
        
        if not quest_data:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Quest {quest_id} not found"
            )
        
        return Quest(**quest_data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error retrieving quest: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/progress", response_model=dict)
async def update_quest_progress(update: QuestProgressUpdate):
    """Update progress on a quest objective"""
    try:
        logger.info(f"Updating quest progress: {update.quest_id}")
        
        # Get quest
        quest_data = await db.get_quest(update.quest_id)
        
        if not quest_data:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Quest {update.quest_id} not found"
            )
        
        quest = Quest(**quest_data)
        
        # Find and update objective
        objective_found = False
        for objective in quest.objectives:
            if objective.objective_id == update.objective_id:
                objective.current_count = min(
                    objective.current_count + update.progress_amount,
                    objective.required_count
                )
                objective.completed = objective.current_count >= objective.required_count
                objective_found = True
                break
        
        if not objective_found:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Objective {update.objective_id} not found in quest"
            )
        
        # Check if all objectives completed
        all_completed = all(obj.completed or obj.optional for obj in quest.objectives)
        
        if all_completed and quest.status == QuestStatus.ACTIVE:
            quest.status = QuestStatus.COMPLETED
            quest.completed_at = datetime.utcnow()
        
        # Update quest in database
        await db.store_quest(quest.quest_id, quest.dict())
        
        logger.info(f"Quest progress updated: {update.quest_id}")
        
        return {
            "quest_id": quest.quest_id,
            "objective_id": update.objective_id,
            "completed": all_completed,
            "status": quest.status
        }
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating quest progress: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )

