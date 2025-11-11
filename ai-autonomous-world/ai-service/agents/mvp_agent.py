"""
MVP AI Agent - Controls boss monster behavior with adaptive AI
"""

from typing import Dict, Any, Optional, List
from loguru import logger
import json
import random
from datetime import datetime, timedelta

from crewai import Agent
try:
    from ai_service.agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from ai_service.models.mvp_behavior import (
        MVPState, MVPAction, MVPBehaviorState, CombatPattern,
        PlayerStrategy, MVPCombatMemory, MVPPersonality, MVPBehaviorConfig
    )
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from models.mvp_behavior import (
        MVPState, MVPAction, MVPBehaviorState, CombatPattern,
        PlayerStrategy, MVPCombatMemory, MVPPersonality, MVPBehaviorConfig
    )


class MVPAgent(BaseAIAgent):
    """
    Specialized agent for MVP/Boss Monster AI
    
    Features:
    - Adaptive combat patterns based on player strategies
    - Learning from past encounters
    - Dynamic behavior state machine
    - Intelligent skill usage
    - Strategic positioning
    """
    
    def __init__(
        self,
        agent_id: str,
        llm_provider: Any,
        config: Dict[str, Any],
        mvp_config: MVPBehaviorConfig
    ):
        super().__init__(
            agent_id=agent_id,
            agent_type="mvp",
            llm_provider=llm_provider,
            config=config
        )
        
        self.mvp_config = mvp_config
        self.crew_agent = self._create_crew_agent()
        logger.info(f"MVP Agent {agent_id} initialized for {mvp_config.mvp_name}")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for MVP AI"""
        from crewai import LLM
        import os
        
        try:
            from ai_service.config import settings
            
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version
            
            llm = LLM(
                model=f"azure/{settings.azure_openai_deployment}",
                temperature=0.8,  # Higher temperature for more varied behavior
                max_tokens=1500
            )
        except Exception as e:
            logger.error(f"Failed to create Azure LLM: {e}")
            raise
        
        return Agent(
            role="Boss Monster AI",
            goal="Provide challenging and adaptive combat encounters for players",
            backstory=f"""You are the AI controlling {self.mvp_config.mvp_name}, a powerful boss monster.
            Your personality traits:
            - Aggression: {self.mvp_config.personality.aggression}
            - Intelligence: {self.mvp_config.personality.intelligence}
            - Adaptability: {self.mvp_config.personality.adaptability}
            - Cunning: {self.mvp_config.personality.cunning}
            
            You learn from each encounter and adapt your tactics to counter player strategies.
            You make strategic decisions about when to attack, retreat, use skills, or summon minions.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
        )
    
    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Process MVP AI decision making
        
        Args:
            context: Current game state and MVP state
        
        Returns:
            AgentResponse with next action
        """
        try:
            mvp_state = MVPState(**context.current_state.get("mvp_state", {}))
            
            # Detect player strategy
            player_strategy = await self._detect_player_strategy(context)
            
            # Update behavior state
            new_state = self._determine_behavior_state(mvp_state, player_strategy)
            
            # Decide next action
            action = await self._decide_action(
                mvp_state=mvp_state,
                behavior_state=new_state,
                player_strategy=player_strategy,
                context=context
            )
            
            logger.info(
                f"MVP {self.mvp_config.mvp_name} - State: {new_state}, "
                f"Action: {action.action_type}"
            )
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "action": action.dict(),
                    "new_state": new_state.value,
                    "detected_strategy": player_strategy.dict() if player_strategy else None
                },
                message=f"MVP action: {action.action_type}"
            )
            
        except Exception as e:
            logger.error(f"Error in MVP agent processing: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                error=str(e)
            )
    
    async def _detect_player_strategy(
        self,
        context: AgentContext
    ) -> Optional[PlayerStrategy]:
        """Detect current player strategy"""
        players = context.current_state.get("engaged_players", [])
        
        if not players:
            return None
        
        # Analyze player composition and behavior
        player_count = len(players)
        total_level = sum(p.get("level", 1) for p in players)
        avg_level = total_level / player_count if player_count > 0 else 1
        
        # Detect damage types
        damage_types = [p.get("damage_type", "physical") for p in players]
        primary_damage = max(set(damage_types), key=damage_types.count)
        
        # Detect roles
        has_healer = any(p.get("class", "").lower() in ["priest", "acolyte"] for p in players)
        has_tank = any(p.get("class", "").lower() in ["knight", "crusader"] for p in players)
        
        # Detect formation
        positions = [(p.get("x", 0), p.get("y", 0)) for p in players]
        formation = self._analyze_formation(positions)
        
        return PlayerStrategy(
            strategy_type=self._classify_strategy(
                player_count, has_healer, has_tank, primary_damage
            ),
            player_count=player_count,
            avg_player_level=avg_level,
            primary_damage_type=primary_damage,
            has_healer=has_healer,
            has_tank=has_tank,
            formation=formation
        )
    
    def _classify_strategy(
        self,
        player_count: int,
        has_healer: bool,
        has_tank: bool,
        damage_type: str
    ) -> str:
        """Classify player strategy type"""
        if player_count == 1:
            return "solo"
        elif player_count <= 3:
            return "small_party"
        elif has_healer and has_tank:
            return "organized_raid"
        elif damage_type == "ranged":
            return "ranged_kiting"
        else:
            return "zerg_rush"

    def _analyze_formation(self, positions: List[tuple]) -> str:
        """Analyze player formation pattern"""
        if len(positions) < 2:
            return "solo"

        # Calculate spread
        xs = [p[0] for p in positions]
        ys = [p[1] for p in positions]
        spread_x = max(xs) - min(xs)
        spread_y = max(ys) - min(ys)

        if spread_x < 3 and spread_y < 3:
            return "clustered"
        elif spread_x > 10 or spread_y > 10:
            return "scattered"
        else:
            return "spread"

    def _determine_behavior_state(
        self,
        mvp_state: MVPState,
        player_strategy: Optional[PlayerStrategy]
    ) -> MVPBehaviorState:
        """Determine MVP behavior state based on conditions"""
        hp_percent = mvp_state.current_hp / mvp_state.max_hp if mvp_state.max_hp > 0 else 0

        # Check retreat condition
        if hp_percent < self.mvp_config.retreat_hp_threshold:
            return MVPBehaviorState.RETREATING

        # Check enrage condition
        if hp_percent < self.mvp_config.enrage_hp_threshold:
            return MVPBehaviorState.ENRAGED

        # Check combat state
        if not mvp_state.in_combat or not player_strategy:
            return MVPBehaviorState.IDLE

        # Determine combat state based on personality and strategy
        if self.mvp_config.personality.aggression > 0.7:
            return MVPBehaviorState.AGGRESSIVE
        elif self.mvp_config.personality.cunning > 0.7:
            return MVPBehaviorState.DEFENSIVE
        else:
            return MVPBehaviorState.AGGRESSIVE

    async def _decide_action(
        self,
        mvp_state: MVPState,
        behavior_state: MVPBehaviorState,
        player_strategy: Optional[PlayerStrategy],
        context: AgentContext
    ) -> MVPAction:
        """Decide next action based on state and strategy"""

        # Retreating behavior
        if behavior_state == MVPBehaviorState.RETREATING:
            return self._create_retreat_action(mvp_state)

        # Enraged behavior
        if behavior_state == MVPBehaviorState.ENRAGED:
            return self._create_enraged_action(mvp_state, player_strategy)

        # Idle behavior
        if behavior_state == MVPBehaviorState.IDLE:
            return self._create_patrol_action(mvp_state)

        # Combat behavior - use LLM for intelligent decision
        if self.mvp_config.personality.intelligence > 0.6:
            return await self._create_intelligent_action(
                mvp_state, player_strategy, context
            )
        else:
            return self._create_aggressive_action(mvp_state, player_strategy)

    def _create_retreat_action(self, mvp_state: MVPState) -> MVPAction:
        """Create retreat action"""
        # Move away from players
        retreat_x = mvp_state.x + random.randint(-5, 5)
        retreat_y = mvp_state.y + random.randint(-5, 5)

        return MVPAction(
            action_type="move",
            target_x=retreat_x,
            target_y=retreat_y,
            priority=10,
            reasoning="HP critical, retreating to recover"
        )

    def _create_enraged_action(
        self,
        mvp_state: MVPState,
        player_strategy: Optional[PlayerStrategy]
    ) -> MVPAction:
        """Create enraged action - aggressive AOE or summon"""
        if random.random() < 0.6:
            return MVPAction(
                action_type="skill",
                skill_id=random.randint(1, 100),  # Placeholder
                skill_level=5,
                priority=9,
                reasoning="Enraged - using powerful AOE skill"
            )
        else:
            return MVPAction(
                action_type="summon",
                priority=8,
                reasoning="Enraged - summoning minions for support"
            )

    def _create_patrol_action(self, mvp_state: MVPState) -> MVPAction:
        """Create patrol action"""
        patrol_x = mvp_state.x + random.randint(-3, 3)
        patrol_y = mvp_state.y + random.randint(-3, 3)

        return MVPAction(
            action_type="move",
            target_x=patrol_x,
            target_y=patrol_y,
            priority=1,
            reasoning="Idle - patrolling area"
        )

    def _create_aggressive_action(
        self,
        mvp_state: MVPState,
        player_strategy: Optional[PlayerStrategy]
    ) -> MVPAction:
        """Create aggressive action"""
        if mvp_state.target_player_id:
            return MVPAction(
                action_type="attack",
                target_id=mvp_state.target_player_id,
                priority=7,
                reasoning="Aggressive - attacking target"
            )
        else:
            return MVPAction(
                action_type="move",
                target_x=mvp_state.x,
                target_y=mvp_state.y,
                priority=3,
                reasoning="No target - waiting"
            )

    async def _create_intelligent_action(
        self,
        mvp_state: MVPState,
        player_strategy: Optional[PlayerStrategy],
        context: AgentContext
    ) -> MVPAction:
        """Use LLM to create intelligent action"""
        try:
            # Build prompt for LLM
            prompt = self._build_action_prompt(mvp_state, player_strategy)

            # Get LLM decision
            response = await self.llm_provider.generate(prompt, max_tokens=200)

            # Parse response into action
            action = self._parse_llm_action(response, mvp_state)
            return action

        except Exception as e:
            logger.error(f"Error in intelligent action: {e}")
            # Fallback to aggressive action
            return self._create_aggressive_action(mvp_state, player_strategy)

    def _build_action_prompt(
        self,
        mvp_state: MVPState,
        player_strategy: Optional[PlayerStrategy]
    ) -> str:
        """Build prompt for LLM action decision"""
        hp_percent = (mvp_state.current_hp / mvp_state.max_hp * 100) if mvp_state.max_hp > 0 else 0

        prompt = f"""You are {self.mvp_config.mvp_name}, a boss monster in combat.

Current Status:
- HP: {hp_percent:.1f}%
- Players engaged: {len(mvp_state.engaged_players)}
- Current pattern: {mvp_state.current_pattern}

Player Strategy: {player_strategy.strategy_type if player_strategy else "unknown"}

Your Personality:
- Aggression: {self.mvp_config.personality.aggression}
- Intelligence: {self.mvp_config.personality.intelligence}
- Cunning: {self.mvp_config.personality.cunning}

Decide your next action. Choose ONE:
1. ATTACK <target> - Direct attack
2. SKILL <skill_name> - Use special skill
3. MOVE <direction> - Reposition
4. SUMMON - Call minions
5. DEFEND - Defensive stance

Respond with just the action and brief reasoning."""

        return prompt

    def _parse_llm_action(self, response: str, mvp_state: MVPState) -> MVPAction:
        """Parse LLM response into MVPAction"""
        response_lower = response.lower()

        if "attack" in response_lower:
            return MVPAction(
                action_type="attack",
                target_id=mvp_state.target_player_id,
                priority=7,
                reasoning=response
            )
        elif "skill" in response_lower:
            return MVPAction(
                action_type="skill",
                skill_id=random.randint(1, 100),
                skill_level=5,
                priority=8,
                reasoning=response
            )
        elif "summon" in response_lower:
            return MVPAction(
                action_type="summon",
                priority=7,
                reasoning=response
            )
        elif "move" in response_lower or "retreat" in response_lower:
            return MVPAction(
                action_type="move",
                target_x=mvp_state.x + random.randint(-5, 5),
                target_y=mvp_state.y + random.randint(-5, 5),
                priority=6,
                reasoning=response
            )
        else:
            # Default to attack
            return MVPAction(
                action_type="attack",
                target_id=mvp_state.target_player_id,
                priority=5,
                reasoning="Default action"
            )

    async def learn_from_encounter(self, combat_memory: MVPCombatMemory):
        """Learn from combat encounter to improve future performance"""
        if not self.mvp_config.learning_enabled:
            return

        logger.info(
            f"MVP {self.mvp_config.mvp_name} learning from encounter: "
            f"{combat_memory.outcome}"
        )

        # Store memory for future reference
        # This would be persisted to database in full implementation

        # Adjust adaptation level
        if combat_memory.outcome == "defeat":
            # Learn more from defeats
            logger.info(f"Learning from defeat - adapting strategies")
        elif combat_memory.outcome == "victory":
            # Reinforce successful patterns
            logger.info(f"Reinforcing successful patterns")

