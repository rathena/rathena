"""
Quest Agent - Generates dynamic quests based on world state and NPC personality
"""

from typing import Dict, Any, Optional
from loguru import logger
import json
import random
from datetime import datetime

from crewai import Agent
try:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from models.quest import (
        Quest, QuestType, QuestDifficulty, QuestStatus,
        QuestObjective, QuestReward, QuestRequirements
    )
    from models.quest_trigger import (
        QuestTrigger, TriggerType, RelationshipTrigger, GiftTrigger,
        SequenceTrigger, RealTimeTrigger, ScheduledTrigger
    )
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from models.quest import (
        Quest, QuestType, QuestDifficulty, QuestStatus,
        QuestObjective, QuestReward, QuestRequirements
    )
    from models.quest_trigger import (
        QuestTrigger, TriggerType, RelationshipTrigger, GiftTrigger,
        SequenceTrigger, RealTimeTrigger, ScheduledTrigger
    )


class QuestAgent(BaseAIAgent):
    """
    Specialized agent for dynamic quest generation
    
    Generates contextual quests based on:
    - NPC personality and class
    - World state and recent events
    - Player level and history
    - Economic and political conditions
    """
    
    def __init__(
        self,
        agent_id: str,
        llm_provider: Any,
        config: Dict[str, Any]
    ):
        super().__init__(
            agent_id=agent_id,
            agent_type="quest",
            llm_provider=llm_provider,
            config=config
        )
        
        self.crew_agent = self._create_crew_agent()
        logger.info(f"Quest Agent {agent_id} initialized")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for quest generation"""
        # Import CrewAI's LLM class
        from crewai import LLM
        import os

        # Create CrewAI-compatible LLM using litellm format for Azure OpenAI
        try:
            from config import settings

            # Set Azure OpenAI environment variables for litellm
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version

            # Use litellm format: azure/<deployment_name>
            llm = LLM(
                model=f"azure/{settings.azure_openai_deployment}",
                temperature=0.7,
                max_tokens=2000
            )
            logger.info(f"Created Azure OpenAI LLM with deployment: {settings.azure_openai_deployment}")
        except Exception as e:
            logger.error(f"Failed to create Azure LLM: {e}")
            raise

        return Agent(
            role="Quest Designer",
            goal="Generate engaging, contextual quests that fit the world state and NPC personality",
            backstory="""You are an expert quest designer for MMORPGs. You create quests that are:
            - Contextually appropriate for the NPC and world state
            - Balanced for the player's level
            - Engaging and story-driven
            - Rewarding but fair
            You consider the NPC's personality, recent world events, and player history.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
        )
    
    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Generate a quest based on context
        
        Args:
            context: Agent context with NPC and world information
            
        Returns:
            AgentResponse with generated quest
        """
        try:
            logger.info(f"Generating quest for NPC: {context.npc_id}")
            
            # Extract generation parameters
            player_level = context.current_state.get("player_level", 1)
            player_class = context.current_state.get("player_class", "Novice")
            npc_class = context.current_state.get("npc_class", "generic")
            
            # Determine quest type based on NPC class
            quest_type = self._determine_quest_type(npc_class, context.personality)
            
            # Determine difficulty based on player level
            difficulty = self._determine_difficulty(player_level)
            
            # Generate quest using LLM
            quest_data = await self._generate_quest_with_llm(
                context=context,
                quest_type=quest_type,
                difficulty=difficulty,
                player_level=player_level,
                player_class=player_class
            )
            
            if not quest_data:
                logger.warning("LLM quest generation failed, using template")
                quest_data = self._generate_template_quest(
                    context, quest_type, difficulty, player_level
                )
            
            # Create Quest object
            quest = self._build_quest_object(
                quest_data=quest_data,
                context=context,
                quest_type=quest_type,
                difficulty=difficulty
            )
            
            logger.info(f"Quest generated: {quest.quest_id} - {quest.title}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "quest": quest.dict(),
                    "quest_id": quest.quest_id,
                    "title": quest.title,
                    "type": quest.quest_type,
                    "difficulty": quest.difficulty
                },
                confidence=0.85,
                reasoning=f"Generated {quest_type} quest for {npc_class} NPC",
                metadata={
                    "npc_id": context.npc_id,
                    "player_level": player_level,
                    "quest_type": quest_type,
                    "difficulty": difficulty
                },
                timestamp=datetime.now(__import__('datetime').timezone.utc)
            )
            
        except Exception as e:
            logger.error(f"Quest generation failed: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={},
                confidence=0.0,
                reasoning=f"Quest generation error: {str(e)}",
                metadata={"error": str(e)},
                timestamp=datetime.now(__import__('datetime').timezone.utc)
            )
    
    def _determine_quest_type(self, npc_class: str, personality: Any) -> QuestType:
        """Determine quest type based on NPC class and personality"""
        
        # Class-specific quest types
        class_quest_types = {
            "merchant": [QuestType.DELIVERY, QuestType.FETCH, QuestType.CRAFT],
            "guard": [QuestType.KILL, QuestType.ESCORT, QuestType.INVESTIGATE],
            "quest_giver": [QuestType.EXPLORE, QuestType.DIALOGUE, QuestType.INVESTIGATE],
            "blacksmith": [QuestType.CRAFT, QuestType.FETCH],
            "priest": [QuestType.ESCORT, QuestType.DIALOGUE],
            "scholar": [QuestType.INVESTIGATE, QuestType.EXPLORE, QuestType.DIALOGUE]
        }
        
        possible_types = class_quest_types.get(npc_class, list(QuestType))
        
        # Personality influence
        if personality:
            if personality.openness > 0.7:
                # High openness prefers exploration and investigation
                if QuestType.EXPLORE in possible_types:
                    return QuestType.EXPLORE
            if personality.conscientiousness > 0.7:
                # High conscientiousness prefers delivery and craft
                if QuestType.DELIVERY in possible_types:
                    return QuestType.DELIVERY
        
        return random.choice(possible_types)
    
    def _determine_difficulty(self, player_level: int) -> QuestDifficulty:
        """Determine quest difficulty based on player level"""

        if player_level < 10:
            return random.choice([QuestDifficulty.TRIVIAL, QuestDifficulty.EASY])
        elif player_level < 30:
            return random.choice([QuestDifficulty.EASY, QuestDifficulty.NORMAL])
        elif player_level < 50:
            return random.choice([QuestDifficulty.NORMAL, QuestDifficulty.HARD])
        elif player_level < 80:
            return random.choice([QuestDifficulty.HARD, QuestDifficulty.VERY_HARD])
        else:
            return random.choice([QuestDifficulty.VERY_HARD, QuestDifficulty.EPIC])

    async def _generate_quest_with_llm(
        self,
        context: AgentContext,
        quest_type: QuestType,
        difficulty: QuestDifficulty,
        player_level: int,
        player_class: str
    ) -> Optional[Dict[str, Any]]:
        """Generate quest using LLM"""

        try:
            # Build personality context
            personality_prompt = ""
            if context.personality:
                personality_prompt = self._build_personality_prompt(context.personality)

            # Build world context
            world_context = ""
            if context.world_state:
                economy = context.world_state.get("economy", {})
                politics = context.world_state.get("politics", {})
                world_context = f"""
World State:
- Economic health: {economy.get('health', 'stable')}
- Political situation: {politics.get('status', 'peaceful')}
- Recent events: {len(context.recent_events)} events
"""

            # Build prompt
            system_message = f"""You are a quest designer creating a {quest_type} quest for a {difficulty} difficulty level.
The quest giver is {context.npc_name}, a {context.current_state.get('npc_class', 'NPC')}.

{personality_prompt}

{world_context}

Generate a quest that:
1. Fits the NPC's personality and role
2. Is appropriate for a level {player_level} {player_class}
3. Considers the current world state
4. Has clear objectives and rewards
5. Tells a compelling micro-story

Return ONLY a JSON object with this structure:
{{
    "title": "Quest Title",
    "description": "Quest description",
    "story_context": "Background story",
    "objectives": [
        {{"description": "Objective 1", "type": "kill/collect/talk/explore", "target": "target name", "count": 5}}
    ],
    "rewards": {{
        "experience": 1000,
        "zeny": 500,
        "items": [{{"item_id": 501, "amount": 10}}],
        "reputation": {{"faction_name": 10}}
    }},
    "time_limit": 3600,
    "success_message": "Success message",
    "failure_message": "Failure message"
}}"""

            user_prompt = f"""Create a {quest_type} quest for a level {player_level} {player_class}.
Quest difficulty: {difficulty}
NPC: {context.npc_name}
Location: {context.current_state.get('location', {}).get('map', 'unknown')}

Make it engaging and contextual!"""

            # Generate with LLM
            response_text = await self._generate_with_llm(
                prompt=user_prompt,
                system_message=system_message,
                temperature=0.8
            )

            # Parse JSON response
            quest_data = self._parse_quest_json(response_text)

            return quest_data

        except Exception as e:
            logger.error(f"LLM quest generation failed: {e}")
            return None

    def _parse_quest_json(self, response_text: str) -> Optional[Dict[str, Any]]:
        """Parse JSON from LLM response"""

        try:
            # Try direct JSON parse
            return json.loads(response_text)
        except json.JSONDecodeError:
            # Try to extract JSON from markdown code block
            if "```json" in response_text:
                start = response_text.find("```json") + 7
                end = response_text.find("```", start)
                json_str = response_text[start:end].strip()
                return json.loads(json_str)
            elif "```" in response_text:
                start = response_text.find("```") + 3
                end = response_text.find("```", start)
                json_str = response_text[start:end].strip()
                return json.loads(json_str)
            else:
                logger.warning("Could not parse JSON from LLM response")
                return None

    def _generate_template_quest(
        self,
        context: AgentContext,
        quest_type: QuestType,
        difficulty: QuestDifficulty,
        player_level: int
    ) -> Dict[str, Any]:
        """Generate a template quest as fallback"""

        templates = {
            QuestType.FETCH: {
                "title": f"Gather Materials for {context.npc_name}",
                "description": f"{context.npc_name} needs materials for their work.",
                "objectives": [{"description": "Collect 10 items", "type": "collect", "target": "Material", "count": 10}],
                "rewards": {"experience": player_level * 100, "zeny": player_level * 50}
            },
            QuestType.KILL: {
                "title": f"Defeat Monsters for {context.npc_name}",
                "description": f"{context.npc_name} needs help dealing with monsters.",
                "objectives": [{"description": "Defeat 5 monsters", "type": "kill", "target": "Monster", "count": 5}],
                "rewards": {"experience": player_level * 150, "zeny": player_level * 75}
            },
            QuestType.DELIVERY: {
                "title": f"Deliver Package for {context.npc_name}",
                "description": f"{context.npc_name} needs a package delivered.",
                "objectives": [{"description": "Deliver package", "type": "deliver", "target": "Recipient", "count": 1}],
                "rewards": {"experience": player_level * 80, "zeny": player_level * 100}
            }
        }

        return templates.get(quest_type, templates[QuestType.FETCH])

    def _build_quest_object(
        self,
        quest_data: Dict[str, Any],
        context: AgentContext,
        quest_type: QuestType,
        difficulty: QuestDifficulty
    ) -> Quest:
        """Build Quest object from generated data"""

        # Generate unique quest ID
        quest_id = f"quest_{context.npc_id}_{int(datetime.now(__import__('datetime').timezone.utc).timestamp())}"

        # Build objectives
        objectives = []
        for idx, obj_data in enumerate(quest_data.get("objectives", [])):
            objectives.append(QuestObjective(
                objective_id=f"{quest_id}_obj_{idx}",
                description=obj_data.get("description", "Complete objective"),
                objective_type=obj_data.get("type", "generic"),
                target=obj_data.get("target", "Unknown"),
                required_count=obj_data.get("count", 1),
                current_count=0,
                completed=False,
                optional=obj_data.get("optional", False)
            ))

        # Build rewards
        reward_data = quest_data.get("rewards", {})
        rewards = QuestReward(
            experience=reward_data.get("experience", 0),
            base_level_exp=reward_data.get("base_level_exp", reward_data.get("experience", 0)),
            zeny=reward_data.get("zeny", 0),
            items=reward_data.get("items", []),
            reputation=reward_data.get("reputation", {})
        )

        # Build requirements
        requirements = QuestRequirements(
            min_level=max(1, context.current_state.get("player_level", 1) - 5),
            max_level=context.current_state.get("player_level", 1) + 10
        )

        # Create Quest
        quest = Quest(
            quest_id=quest_id,
            title=quest_data.get("title", f"Quest from {context.npc_name}"),
            description=quest_data.get("description", "Help the NPC"),
            quest_type=quest_type,
            difficulty=difficulty,
            status=QuestStatus.AVAILABLE,
            giver_npc_id=context.npc_id,
            giver_npc_name=context.npc_name,
            objectives=objectives,
            requirements=requirements,
            rewards=rewards,
            time_limit=quest_data.get("time_limit"),
            story_context=quest_data.get("story_context", ""),
            success_message=quest_data.get("success_message", "Quest completed!"),
            failure_message=quest_data.get("failure_message", "Quest failed."),
            generated_by_ai=True,
            generation_context={
                "world_state": context.world_state,
                "npc_personality": context.personality.dict() if context.personality else {}
            },
            triggers=[],  # Will be added by add_triggers_to_quest
            is_secret_quest=False,
            prerequisite_quests=[],
            next_in_sequence=None,
            relationship_unlock_threshold=None
        )

        return quest

    def add_triggers_to_quest(
        self,
        quest: Quest,
        trigger_config: Dict[str, Any]
    ) -> Quest:
        """
        Add triggers to a quest based on configuration

        Args:
            quest: The quest to add triggers to
            trigger_config: Configuration for triggers

        Returns:
            Quest with triggers added
        """
        triggers = []

        # Relationship trigger
        if trigger_config.get("relationship_threshold"):
            trigger = QuestTrigger(
                trigger_id=f"{quest.quest_id}_relationship",
                trigger_type=TriggerType.RELATIONSHIP,
                relationship=RelationshipTrigger(
                    relationship_level_min=trigger_config["relationship_threshold"]
                ),
                description=f"Requires relationship level {trigger_config['relationship_threshold']}",
                priority=8
            )
            triggers.append(trigger)
            quest.relationship_unlock_threshold = trigger_config["relationship_threshold"]

        # Gift trigger
        if trigger_config.get("gift_required"):
            gift_cfg = trigger_config["gift_required"]
            trigger = QuestTrigger(
                trigger_id=f"{quest.quest_id}_gift",
                trigger_type=TriggerType.GIFT,
                gift=GiftTrigger(
                    total_gifts_min=gift_cfg.get("total_gifts_min"),
                    specific_item_received=gift_cfg.get("specific_item_id"),
                    gift_category_received=gift_cfg.get("category")
                ),
                description="Triggered by gift-giving",
                priority=7
            )
            triggers.append(trigger)

        # Sequence trigger (prerequisite quests)
        if trigger_config.get("prerequisite_quests"):
            trigger = QuestTrigger(
                trigger_id=f"{quest.quest_id}_sequence",
                trigger_type=TriggerType.SEQUENCE,
                sequence=SequenceTrigger(
                    prerequisite_quests=trigger_config["prerequisite_quests"],
                    all_required=trigger_config.get("all_prerequisites_required", True)
                ),
                description="Requires prerequisite quests",
                priority=9
            )
            triggers.append(trigger)
            quest.prerequisite_quests = trigger_config["prerequisite_quests"]

        # Time-based trigger
        if trigger_config.get("real_time"):
            rt_cfg = trigger_config["real_time"]
            trigger = QuestTrigger(
                trigger_id=f"{quest.quest_id}_realtime",
                trigger_type=TriggerType.REAL_TIME,
                real_time=RealTimeTrigger(
                    start_date=rt_cfg.get("start_date"),
                    end_date=rt_cfg.get("end_date"),
                    days_of_week=rt_cfg.get("days_of_week"),
                    hours=rt_cfg.get("hours")
                ),
                description="Time-based trigger",
                priority=5
            )
            triggers.append(trigger)

        # Scheduled (in-game time) trigger
        if trigger_config.get("scheduled"):
            sch_cfg = trigger_config["scheduled"]
            trigger = QuestTrigger(
                trigger_id=f"{quest.quest_id}_scheduled",
                trigger_type=TriggerType.SCHEDULED,
                scheduled=ScheduledTrigger(
                    game_time_start=sch_cfg.get("game_time_start"),
                    game_time_end=sch_cfg.get("game_time_end"),
                    game_day=sch_cfg.get("game_day"),
                    season=sch_cfg.get("season")
                ),
                description="In-game time trigger",
                priority=6
            )
            triggers.append(trigger)

        # Mark as secret quest if it has triggers
        if triggers:
            quest.is_secret_quest = trigger_config.get("is_secret", True)

        quest.triggers = triggers
        return quest

    async def generate_secret_quest(
        self,
        context: AgentContext,
        relationship_threshold: float = 50.0,
        prerequisite_quests: Optional[list] = None
    ) -> Quest:
        """
        Generate a secret quest with relationship and sequence triggers

        Args:
            context: Agent context
            relationship_threshold: Minimum relationship level to unlock
            prerequisite_quests: List of prerequisite quest IDs

        Returns:
            Secret quest with triggers
        """
        # Generate base quest
        response = await self.process(context)
        quest_data = response.data.get("quest")

        if not quest_data:
            raise ValueError("Failed to generate base quest")

        quest = Quest(**quest_data)

        # Add triggers
        trigger_config = {
            "relationship_threshold": relationship_threshold,
            "is_secret": True
        }

        if prerequisite_quests:
            trigger_config["prerequisite_quests"] = prerequisite_quests

        quest = self.add_triggers_to_quest(quest, trigger_config)

        logger.info(f"Generated secret quest: {quest.quest_id} with {len(quest.triggers)} triggers")

        return quest

