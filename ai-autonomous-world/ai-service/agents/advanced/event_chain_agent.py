"""
Event Chain Agent - Multi-step Dynamic Story Events
Implements 4-tier LLM optimization for branching narrative chains
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any, Tuple
from enum import Enum
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class ChainEventType(str, Enum):
    """Event chain types"""
    INVESTIGATION = "investigation"
    INVASION = "invasion"
    RESCUE = "rescue"
    TREASURE_HUNT = "treasure_hunt"
    FACTION_WAR = "faction_war"


class EventChainAgent(BaseAIAgent):
    """
    Creates multi-step events with player-influenced outcomes.
    
    Chain Structure:
    - 3-7 steps per chain
    - Each step unlocks next
    - Branching based on success/failure
    - Tomorrow's events depend on today's outcome
    - Persistent story across days/weeks
    
    Example Chain:
    1. Strange Traveler appears → gives hint
    2. Map corrupted → mini mobs spawn
    3. Boss appears → must be defeated
    4. Treasure chest spawns → special rewards
    5. Ending dialogue → faction shift or next arc
    
    Features:
    - Branching dialogues
    - Multiple endings
    - Player choice matters
    - Story persistence
    - LLM-generated narrative (optional)
    
    4-Tier Optimization:
    - Tier 1 (60%): Template-based chains
    - Tier 2 (25%): Cached chain specs
    - Tier 3 (10%): Batched step generation
    - Tier 4 (5%): LLM creative narratives
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Event Chain Agent"""
        super().__init__(
            agent_id="event_chain_agent",
            agent_type="event_chain",
            config=config
        )
        
        # Chain templates (Tier 1: Rule-based)
        self.chain_templates = {
            ChainEventType.INVESTIGATION: {
                "name": "Mystery Investigation",
                "min_steps": 4,
                "max_steps": 6,
                "step_types": [
                    "npc_dialogue",
                    "item_collection",
                    "location_discovery",
                    "boss_battle",
                    "resolution"
                ]
            },
            ChainEventType.INVASION: {
                "name": "Monster Invasion",
                "min_steps": 3,
                "max_steps": 5,
                "step_types": [
                    "warning_signs",
                    "wave_defense",
                    "boss_battle",
                    "aftermath"
                ]
            },
            ChainEventType.RESCUE: {
                "name": "Rescue Mission",
                "min_steps": 3,
                "max_steps": 5,
                "step_types": [
                    "distress_call",
                    "combat_rescue",
                    "escort_mission",
                    "safe_return"
                ]
            },
            ChainEventType.TREASURE_HUNT: {
                "name": "Treasure Hunt",
                "min_steps": 4,
                "max_steps": 7,
                "step_types": [
                    "map_discovery",
                    "location_clue",
                    "puzzle_solve",
                    "treasure_guardian",
                    "treasure_reveal"
                ]
            },
            ChainEventType.FACTION_WAR: {
                "name": "Faction Conflict",
                "min_steps": 5,
                "max_steps": 7,
                "step_types": [
                    "tensions_rise",
                    "choose_side",
                    "faction_quest",
                    "major_battle",
                    "faction_shift",
                    "peace_treaty"
                ]
            }
        }
        
        # Dialogue templates for each step type
        self.dialogue_templates = {
            "npc_dialogue": [
                "Greetings, traveler. I have information that might interest you...",
                "Something strange has been happening lately. Will you help investigate?",
                "I've been waiting for someone brave enough to hear this tale..."
            ],
            "warning_signs": [
                "Dark clouds gather on the horizon. Something evil approaches...",
                "The monsters grow restless. An invasion is imminent!",
                "Citizens report sightings of unusual creatures near the city..."
            ],
            "distress_call": [
                "Help! Someone please help! They've been taken!",
                "We need brave souls! Our people are in danger!",
                "A caravan has been ambushed! Survivors need rescue!"
            ]
        }
        
        # Success/failure branch logic
        self.branch_logic = {
            "success": {
                "next_step_modifier": 1.0,
                "reward_multiplier": 1.5,
                "world_impact": "positive"
            },
            "failure": {
                "next_step_modifier": 0.8,
                "reward_multiplier": 0.5,
                "world_impact": "negative"
            },
            "partial": {
                "next_step_modifier": 0.9,
                "reward_multiplier": 1.0,
                "world_impact": "neutral"
            }
        }
        
        # World outcome templates
        self.outcome_templates = {
            "positive": {
                "faction_shift": 100,
                "problem_resolution": True,
                "npc_spawns": "friendly",
                "map_modifier": "blessing"
            },
            "negative": {
                "faction_shift": -100,
                "problem_creation": True,
                "npc_spawns": "hostile",
                "map_modifier": "corruption"
            },
            "neutral": {
                "faction_shift": 0,
                "npc_spawns": "neutral",
                "map_modifier": None
            }
        }
        
        logger.info("Event Chain Agent initialized")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for event chain management"""
        from crewai import Agent
        
        return Agent(
            role="Dynamic Story Event Manager",
            goal="Create engaging multi-step story events",
            backstory="An AI system that weaves persistent narratives",
            verbose=settings.crewai_verbose,
            allow_delegation=False,
            llm=self.llm_provider if hasattr(self, 'llm_provider') else None
        )
    
    async def _process(self, context: AgentContext) -> AgentResponse:
        """Process method required by BaseAIAgent"""
        return AgentResponse(
            agent_type=self.agent_type,
            success=True,
            data={},
            confidence=1.0
        )
    
    # ========================================================================
    # PUBLIC API METHODS
    # ========================================================================
    
    async def start_event_chain(
        self,
        chain_type: ChainEventType,
        trigger_condition: Dict[str, Any]
    ) -> AgentResponse:
        """
        Start a new event chain (Tier 1: Template-based).
        
        Chain Initialization:
        1. Select chain template or generate with LLM
        2. Create first step (NPC, dialogue, objective)
        3. Prepare branching logic
        4. Set expiration (1-7 days)
        5. Store chain state
        
        Args:
            chain_type: Type of event chain
            trigger_condition: Conditions that triggered this chain
            
        Returns:
            First step specification
        """
        try:
            logger.info(f"Starting event chain: {chain_type}")
            
            # Get template
            template = self.chain_templates.get(chain_type)
            if not template:
                raise ValueError(f"Unknown chain type: {chain_type}")
            
            # Determine total steps
            total_steps = random.randint(template['min_steps'], template['max_steps'])
            
            # Generate chain data
            chain_data = {
                "chain_type": chain_type.value,
                "chain_name": template['name'],
                "total_steps": total_steps,
                "step_types": template['step_types'][:total_steps],
                "trigger_condition": trigger_condition
            }
            
            # Create first step
            first_step = await self._generate_chain_step(
                chain_type=chain_type,
                step_number=1,
                total_steps=total_steps,
                previous_outcome=None
            )
            
            # Store chain in database
            query = """
                INSERT INTO event_chains
                (chain_type, chain_name, total_steps, chain_data, trigger_condition, expires_at)
                VALUES ($1, $2, $3, $4, $5, NOW() + INTERVAL '7 days')
                RETURNING chain_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                chain_type.value,
                template['name'],
                total_steps,
                json.dumps(chain_data),
                json.dumps(trigger_condition)
            )
            
            chain_id = result['chain_id']
            
            # Store first step
            await self._store_chain_step(chain_id, 1, first_step)
            
            # Cache for fast access
            cache_key = f"chain:active:{chain_id}"
            await db.set(cache_key, {
                "chain_id": chain_id,
                "current_step": 1,
                "total_steps": total_steps
            }, expire=604800)  # 7 days
            
            result_data = {
                "chain_id": chain_id,
                "chain_type": chain_type.value,
                "chain_name": template['name'],
                "total_steps": total_steps,
                "current_step": 1,
                "first_step": first_step,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Started chain {chain_id}: {template['name']}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Event chain started successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to start event chain: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def advance_chain_step(
        self,
        chain_id: int,
        outcome: str  # "success", "failure", "partial"
    ) -> AgentResponse:
        """
        Advance to next step based on outcome.
        
        Branching Logic:
        - Success → standard progression
        - Failure → alternate path or retry
        - Partial → modified next step
        
        Tomorrow's Impact:
        - Success chains → positive world changes
        - Failure chains → world becomes darker/harder
        - Mixed outcomes → neutral progression
        
        Args:
            chain_id: Event chain ID
            outcome: Step outcome
            
        Returns:
            Next step specification or chain end
        """
        try:
            # Get current chain state
            query = """
                SELECT chain_type, chain_name, total_steps, current_step, chain_data
                FROM event_chains
                WHERE chain_id = $1 AND status = 'active'
            """
            
            chain = await postgres_db.fetch_one(query, chain_id)
            
            if not chain:
                raise ValueError(f"Chain {chain_id} not found or not active")
            
            current_step = chain['current_step']
            total_steps = chain['total_steps']
            
            # Update current step outcome
            query = """
                UPDATE chain_steps
                SET completed = TRUE, outcome = $1, completed_at = NOW()
                WHERE chain_id = $2 AND step_number = $3
            """
            await postgres_db.execute(query, outcome, chain_id, current_step)
            
            # Check if chain is complete
            if current_step >= total_steps:
                return await self.evaluate_chain_completion(chain_id)
            
            # Generate next step based on outcome
            next_step_number = current_step + 1
            next_step = await self._generate_chain_step(
                chain_type=ChainEventType(chain['chain_type']),
                step_number=next_step_number,
                total_steps=total_steps,
                previous_outcome=outcome
            )
            
            # Apply outcome modifiers
            branch_mod = self.branch_logic.get(outcome, self.branch_logic["partial"])
            next_step['reward_multiplier'] = branch_mod['reward_multiplier']
            next_step['difficulty_modifier'] = branch_mod['next_step_modifier']
            
            # Store next step
            await self._store_chain_step(chain_id, next_step_number, next_step)
            
            # Update chain state
            query = """
                UPDATE event_chains
                SET current_step = $1
                WHERE chain_id = $2
            """
            await postgres_db.execute(query, next_step_number, chain_id)
            
            # Update cache
            cache_key = f"chain:active:{chain_id}"
            await db.set(cache_key, {
                "chain_id": chain_id,
                "current_step": next_step_number,
                "total_steps": total_steps
            }, expire=604800)
            
            result_data = {
                "chain_id": chain_id,
                "previous_step": current_step,
                "previous_outcome": outcome,
                "current_step": next_step_number,
                "total_steps": total_steps,
                "next_step": next_step,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Advanced chain {chain_id} to step {next_step_number}/{total_steps}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Chain advanced successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to advance chain: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def generate_chain_dialogue(
        self,
        chain_context: Dict[str, Any],
        step_number: int
    ) -> str:
        """
        Generate contextual dialogue for chain step.
        
        Uses:
        - Template-based (Tier 1, 60%)
        - LLM-generated (Tier 4, 40%) for unique chains
        
        Context includes:
        - Previous step outcomes
        - Player faction alignment
        - World karma state
        - Active problems/events
        
        Args:
            chain_context: Current chain context
            step_number: Current step number
            
        Returns:
            Dialogue text
        """
        try:
            # Use template dialogue (Tier 1) by default
            step_type = chain_context.get('step_type', 'npc_dialogue')
            templates = self.dialogue_templates.get(step_type, [
                "Continue your quest, brave adventurer...",
                "The path ahead is dangerous. Proceed with caution.",
                "Your actions will shape the future of this land."
            ])
            
            # Select random template
            dialogue = random.choice(templates)
            
            # Optionally use LLM for creative dialogue (Tier 4, 5% chance)
            use_llm = getattr(settings, 'chain_llm_narrative', False) and random.random() < 0.05
            
            if use_llm:
                try:
                    prompt = f"""Generate a short NPC dialogue for a {step_type} event.
Context: {json.dumps(chain_context, indent=2)}
Step: {step_number}
Keep it under 100 words and in-character."""
                    
                    dialogue = await self._generate_with_llm(
                        prompt=prompt,
                        temperature=0.8
                    )
                except Exception as e:
                    logger.warning(f"LLM dialogue generation failed, using template: {e}")
            
            return dialogue
            
        except Exception as e:
            logger.error(f"Failed to generate dialogue: {e}")
            return "..."
    
    async def evaluate_chain_completion(
        self,
        chain_id: int
    ) -> AgentResponse:
        """
        Evaluate chain completion and apply outcomes.
        
        Outcomes:
        - Faction alignment shift
        - World problem resolution
        - New NPC unlocks
        - Map transformations
        - Special item rewards
        - Reputation bonuses
        
        Args:
            chain_id: Event chain ID
            
        Returns:
            AgentResponse with completion results
        """
        try:
            # Get all chain steps and outcomes
            query = """
                SELECT step_number, outcome, completed
                FROM chain_steps
                WHERE chain_id = $1
                ORDER BY step_number
            """
            
            steps = await postgres_db.fetch_all(query, chain_id)
            
            # Calculate overall success rate
            completed_steps = [s for s in steps if s['completed']]
            if not completed_steps:
                success_rate = 0.0
            else:
                success_count = sum(1 for s in completed_steps if s['outcome'] == 'success')
                success_rate = success_count / len(completed_steps)
            
            # Determine final outcome
            if success_rate >= 0.7:
                final_outcome = "success"
                world_impact = "positive"
            elif success_rate >= 0.4:
                final_outcome = "partial"
                world_impact = "neutral"
            else:
                final_outcome = "failure"
                world_impact = "negative"
            
            # Get outcome template
            outcome_data = self.outcome_templates.get(world_impact, {})
            
            # Apply world changes
            world_changes = {
                "faction_shifts": outcome_data.get('faction_shift', 0),
                "problem_resolved": outcome_data.get('problem_resolution', False),
                "problem_created": outcome_data.get('problem_creation', False),
                "npc_spawns": outcome_data.get('npc_spawns'),
                "map_modifiers": outcome_data.get('map_modifier')
            }
            
            # Get participant count
            query = """
                SELECT COUNT(DISTINCT player_id) as participant_count
                FROM chain_participation
                WHERE chain_id = $1
            """
            result = await postgres_db.fetch_one(query, chain_id)
            participant_count = result['participant_count'] if result else 0
            
            # Store completion outcome
            query = """
                INSERT INTO chain_outcomes
                (chain_id, final_outcome, world_changes, participant_count, success_rate)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                chain_id,
                final_outcome,
                json.dumps(world_changes),
                participant_count,
                success_rate
            )
            
            # Update chain status
            query = """
                UPDATE event_chains
                SET status = 'completed', completed_at = NOW()
                WHERE chain_id = $1
            """
            await postgres_db.execute(query, chain_id)
            
            # Clear cache
            await db.delete(f"chain:active:{chain_id}")
            
            result_data = {
                "chain_id": chain_id,
                "final_outcome": final_outcome,
                "success_rate": success_rate,
                "world_changes": world_changes,
                "participant_count": participant_count,
                "completed_steps": len(completed_steps),
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Completed chain {chain_id}: {final_outcome} ({success_rate:.1%} success)")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=1.0,
                reasoning="Chain completed successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to evaluate chain completion: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _generate_chain_step(
        self,
        chain_type: ChainEventType,
        step_number: int,
        total_steps: int,
        previous_outcome: Optional[str]
    ) -> Dict[str, Any]:
        """Generate a chain step specification"""
        try:
            template = self.chain_templates.get(chain_type)
            step_type = template['step_types'][min(step_number - 1, len(template['step_types']) - 1)]
            
            # Generate dialogue
            dialogue = await self.generate_chain_dialogue(
                chain_context={
                    "chain_type": chain_type.value,
                    "step_type": step_type,
                    "step_number": step_number,
                    "total_steps": total_steps,
                    "previous_outcome": previous_outcome
                },
                step_number=step_number
            )
            
            # Generate objective based on step type
            objectives = {
                "npc_dialogue": "Talk to the NPC",
                "item_collection": "Collect required items",
                "location_discovery": "Find the marked location",
                "boss_battle": "Defeat the boss",
                "resolution": "Complete the quest",
                "warning_signs": "Investigate the disturbance",
                "wave_defense": "Defend against waves",
                "distress_call": "Respond to the call",
                "combat_rescue": "Rescue the captives",
                "escort_mission": "Escort safely",
                "puzzle_solve": "Solve the puzzle",
                "treasure_guardian": "Defeat the guardian",
                "tensions_rise": "Observe the conflict",
                "choose_side": "Choose your faction",
                "faction_quest": "Complete faction task",
                "major_battle": "Join the battle"
            }
            
            objective = objectives.get(step_type, "Complete the step")
            
            # Generate success/failure conditions
            success_condition = {
                "type": step_type,
                "required_actions": self._get_required_actions(step_type),
                "time_limit": None if step_type in ["npc_dialogue", "resolution"] else 3600
            }
            
            step = {
                "step_number": step_number,
                "step_type": step_type,
                "objective": objective,
                "dialogue": dialogue,
                "success_condition": success_condition,
                "npc_data": self._generate_npc_data(step_type) if "npc" in step_type or "dialogue" in step_type else None,
                "spawn_data": self._generate_spawn_data(step_type) if "battle" in step_type or "wave" in step_type else None
            }
            
            return step
            
        except Exception as e:
            logger.error(f"Failed to generate chain step: {e}")
            return {
                "step_number": step_number,
                "step_type": "error",
                "objective": "Continue",
                "dialogue": "...",
                "error": str(e)
            }
    
    def _get_required_actions(self, step_type: str) -> List[str]:
        """Get required actions for step type"""
        action_map = {
            "npc_dialogue": ["talk_to_npc"],
            "item_collection": ["collect_items"],
            "location_discovery": ["reach_location"],
            "boss_battle": ["defeat_boss"],
            "wave_defense": ["survive_waves"],
            "combat_rescue": ["rescue_npcs"],
            "puzzle_solve": ["solve_puzzle"]
        }
        
        return action_map.get(step_type, ["complete_objective"])
    
    def _generate_npc_data(self, step_type: str) -> Dict[str, Any]:
        """Generate NPC data for step"""
        return {
            "npc_name": f"Chain_NPC_{random.randint(1000, 9999)}",
            "npc_sprite": "1_M_MERCHANT",
            "spawn_map": "prontera",
            "spawn_x": random.randint(100, 200),
            "spawn_y": random.randint(100, 200)
        }
    
    def _generate_spawn_data(self, step_type: str) -> Dict[str, Any]:
        """Generate monster spawn data for step"""
        monster_counts = {
            "boss_battle": 1,
            "wave_defense": 20,
            "combat_rescue": 10
        }
        
        return {
            "monsters": ["PORING", "DROPS", "LUNATIC"],  # Simplified
            "count": monster_counts.get(step_type, 5),
            "spawn_map": "prontera",
            "spawn_area": {"x": 150, "y": 150, "range": 20}
        }
    
    async def _store_chain_step(
        self,
        chain_id: int,
        step_number: int,
        step_data: Dict[str, Any]
    ) -> None:
        """Store chain step in database"""
        try:
            query = """
                INSERT INTO chain_steps
                (chain_id, step_number, step_type, objective, success_condition, 
                 npc_data, spawn_data, dialogue)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
            """
            
            await postgres_db.execute(
                query,
                chain_id,
                step_number,
                step_data.get('step_type'),
                step_data.get('objective'),
                json.dumps(step_data.get('success_condition', {})),
                json.dumps(step_data.get('npc_data')) if step_data.get('npc_data') else None,
                json.dumps(step_data.get('spawn_data')) if step_data.get('spawn_data') else None,
                step_data.get('dialogue')
            )
            
        except Exception as e:
            logger.error(f"Failed to store chain step: {e}")
            raise


# Global instance
event_chain_agent = EventChainAgent()