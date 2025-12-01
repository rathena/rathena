"""
World Agent - Analyzes and responds to world state changes
Processes global events and their impact on NPCs
"""

from typing import Dict, Any, List
from loguru import logger
import json

from crewai import Agent
try:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from models.npc_relationship import NPCRelationship, NPCInteraction, SharedInformation

class WorldAgent(BaseAIAgent):
    """
    Specialized agent for world state analysis

    Responsibilities:
    - Analyze world state changes
    - Determine impact on NPCs
    - Generate world events
    - Track economic trends
    - Monitor political changes
    - Assess environmental conditions
    - Integrate with full NPC consciousness model (personality, goals, emotion, memory, reflection)
    - Integrate social fabric: relationship, gossip, persuasion, social learning, reputation
    """
    def __init__(self, agent_id: str, llm_provider: Any, config: Dict[str, Any]):
        """Initialize World Agent"""
        super().__init__(
            agent_id=agent_id,
            agent_type="world",
            llm_provider=llm_provider,
            config=config
        )
        from agents.moral_alignment import MoralAlignment
        self.moral_alignment = MoralAlignment()
        logger.info(f"World Agent {agent_id} initialized")

    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for world analysis"""
        from crewai import LLM
        import os
        try:
            from config import settings
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version
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
            role="World State Analyst",
            goal="Analyze world conditions and their impact on NPCs to create a living, reactive world",
            backstory="""You are an expert in systems thinking and world simulation. You understand how
            economic, political, and environmental factors interact and affect individuals. You excel at
            identifying patterns, predicting trends, and determining how global changes impact local
            behavior. You help create a world that feels alive and responsive. You also understand social fabric, reputation, and emergent behavior.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
        )

    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Analyze world state and determine impact

        Args:
            context: AgentContext with world state information

        Returns:
            AgentResponse with analysis results
        """
        try:
            operation = context.current_state.get("operation", "analyze")

            # --- Moral Alignment Integration ---
            # Update alignment based on world event/operation
            alignment_action = {
                "type": f"world_{operation}",
                "npc_id": context.npc_id if hasattr(context, "npc_id") else None,
                "operation": operation
            }
            self.moral_alignment.update_from_action(alignment_action)

            if operation == "analyze":
                result = await self._analyze_world_state(context)
            elif operation == "impact":
                result = await self._assess_impact(context)
            elif operation == "generate_event":
                result = await self._generate_world_event(context)
            else:
                raise ValueError(f"Unknown world operation: {operation}")
            logger.info(f"World operation '{operation}' completed")
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={**result, "alignment": self.moral_alignment.to_dict()},
                confidence=0.75,
                reasoning=f"World {operation} operation completed and alignment updated",
                metadata={"alignment": self.moral_alignment.to_dict()}
            )
        except Exception as e:
            logger.error(f"World operation failed: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error during world operation: {e}"
            )

    async def _analyze_world_state(self, context: AgentContext) -> Dict[str, Any]:
        """Analyze current world state (now with consciousness and social context)"""
        world_state = context.world_state
        # Optionally, use personality, goal, emotion, memory for world-level analysis
        analysis = {
            "economy": self._analyze_economy(world_state.get("economy", {})),
            "politics": self._analyze_politics(world_state.get("politics", {})),
            "environment": self._analyze_environment(world_state.get("environment", {})),
            "overall_stability": 0.0,
            "trends": [],
            "npc_consciousness": {
                "personality": self._build_personality_prompt(context.personality),
                "goal_state": self._build_goal_prompt(getattr(context, "goal_state", None)),
                "emotion_state": self._build_emotion_prompt(getattr(context, "emotion_state", None)),
                "memory_state": self._build_memory_prompt(getattr(context, "memory_state", None))
            },
            "social_fabric": self._build_social_fabric_context(context)
        }
        stability_scores = [
            analysis["economy"].get("stability", 0.5),
            analysis["politics"].get("stability", 0.5),
            analysis["environment"].get("stability", 0.5)
        ]
        analysis["overall_stability"] = sum(stability_scores) / len(stability_scores)
        analysis["trends"] = self._identify_trends(analysis)
        logger.info(f"World state analyzed: stability={analysis['overall_stability']:.2f}")
        return analysis

    def _analyze_economy(self, economy_state: Dict[str, Any]) -> Dict[str, Any]:
        """Analyze economic conditions"""
        inflation_rate = economy_state.get("inflation_rate", 0.0)
        trade_volume = economy_state.get("trade_volume", 0.0)
        if inflation_rate < 0.05 and trade_volume > 500000:
            health = "thriving"
            stability = 0.9
        elif inflation_rate < 0.10 and trade_volume > 100000:
            health = "stable"
            stability = 0.7
        elif inflation_rate < 0.20:
            health = "struggling"
            stability = 0.4
        else:
            health = "crisis"
            stability = 0.2
        return {
            "health": health,
            "stability": stability,
            "inflation_rate": inflation_rate,
            "trade_volume": trade_volume,
            "description": f"Economy is {health} with {inflation_rate*100:.1f}% inflation"
        }

    def _analyze_politics(self, politics_state: Dict[str, Any]) -> Dict[str, Any]:
        """Analyze political conditions"""
        faction_relations = politics_state.get("faction_relations", {})
        conflict_level = politics_state.get("conflict_level", 0)
        logger.debug(f"Analyzing politics with {len(faction_relations)} faction relations")
        if conflict_level < 0.2:
            status = "peaceful"
            stability = 0.9
        elif conflict_level < 0.5:
            status = "tense"
            stability = 0.6
        elif conflict_level < 0.8:
            status = "conflict"
            stability = 0.3
        else:
            status = "war"
            stability = 0.1
        return {
            "status": status,
            "stability": stability,
            "conflict_level": conflict_level,
            "description": f"Political situation is {status}"
        }

    def _analyze_environment(self, environment_state: Dict[str, Any]) -> Dict[str, Any]:
        """Analyze environmental conditions"""
        weather = environment_state.get("weather", "clear")
        season = environment_state.get("season", "spring")
        disasters = environment_state.get("recent_disasters", [])
        if not disasters and weather in ["clear", "sunny"]:
            condition = "favorable"
            stability = 0.9
        elif len(disasters) == 0:
            condition = "normal"
            stability = 0.7
        elif len(disasters) < 3:
            condition = "challenging"
            stability = 0.4
        else:
            condition = "hazardous"
            stability = 0.2
        return {
            "condition": condition,
            "stability": stability,
            "weather": weather,
            "season": season,
            "description": f"Environment is {condition} ({weather}, {season})"
        }

    def _identify_trends(self, analysis: Dict[str, Any]) -> List[str]:
        """Identify notable trends from analysis"""
        trends = []
        economy = analysis["economy"]
        if economy["stability"] < 0.5:
            trends.append("economic_decline")
        elif economy["stability"] > 0.8:
            trends.append("economic_boom")
        politics = analysis["politics"]
        if politics["stability"] < 0.5:
            trends.append("political_instability")
        environment = analysis["environment"]
        if environment["stability"] < 0.5:
            trends.append("environmental_crisis")
        return trends

    def _build_social_fabric_context(self, context: AgentContext) -> str:
        """
        Build social fabric context: relationships, gossip, persuasion, social learning, reputation
        """
        relationship_data = context.current_state.get("npc_relationships", [])
        gossip_data = context.current_state.get("gossip", [])
        reputation = context.current_state.get("reputation", 0)
        social_parts = []
        if relationship_data:
            for rel in relationship_data:
                try:
                    rel_obj = NPCRelationship(**rel)
                    social_parts.append(
                        f"Relationship with {rel_obj.npc_id_2}: {rel_obj.relationship_type} ({rel_obj.relationship_value:.1f}), trust: {rel_obj.trust_level:.1f}"
                    )
                except Exception:
                    continue
        if gossip_data:
            for g in gossip_data:
                try:
                    g_obj = SharedInformation(**g)
                    social_parts.append(
                        f"Gossip: {g_obj.content} (from {g_obj.source_npc_id}, reliability: {g_obj.reliability:.2f})"
                    )
                except Exception:
                    continue
        social_parts.append(f"Reputation: {reputation}")
        return "\n".join(social_parts)

    async def _assess_impact(self, context: AgentContext) -> Dict[str, Any]:
        """Assess impact of world state on specific NPC (with consciousness and social context)"""
        world_analysis = await self._analyze_world_state(context)
        npc_class = context.current_state.get("npc_class", "generic")
        impact = {
            "affected": False,
            "impact_level": 0.0,
            "suggested_behaviors": [],
            "mood_modifier": 0.0,
            "npc_consciousness": {
                "personality": self._build_personality_prompt(context.personality),
                "goal_state": self._build_goal_prompt(getattr(context, "goal_state", None)),
                "emotion_state": self._build_emotion_prompt(getattr(context, "emotion_state", None)),
                "memory_state": self._build_memory_prompt(getattr(context, "memory_state", None))
            },
            "social_fabric": self._build_social_fabric_context(context)
        }
        if npc_class == "merchant":
            economy_stability = world_analysis["economy"]["stability"]
            impact["affected"] = True
            impact["impact_level"] = 1.0 - economy_stability
            if economy_stability < 0.5:
                impact["suggested_behaviors"].append("complain_about_economy")
                impact["mood_modifier"] = -0.3
            elif economy_stability > 0.8:
                impact["suggested_behaviors"].append("celebrate_prosperity")
                impact["mood_modifier"] = 0.3
        elif npc_class == "guard":
            politics_stability = world_analysis["politics"]["stability"]
            impact["affected"] = True
            impact["impact_level"] = 1.0 - politics_stability
            if politics_stability < 0.5:
                impact["suggested_behaviors"].append("increase_vigilance")
                impact["mood_modifier"] = -0.2
        return impact

    async def _generate_world_event(self, context: AgentContext) -> Dict[str, Any]:
        """Generate a world event based on current state (with consciousness and social context)"""
        world_state = context.world_state
        personality = self._build_personality_prompt(context.personality)
        goal_state = self._build_goal_prompt(getattr(context, "goal_state", None))
        emotion_state = self._build_emotion_prompt(getattr(context, "emotion_state", None))
        memory_state = self._build_memory_prompt(getattr(context, "memory_state", None))
        social_context = self._build_social_fabric_context(context)
        event = await self._generate_event_with_llm(world_state, personality, goal_state, emotion_state, memory_state, social_context)
        return {
            "event_type": event.get("type", "general"),
            "description": event.get("description", "A notable event occurred"),
            "severity": event.get("severity", 5),
            "affected_areas": event.get("affected_areas", []),
            "duration": event.get("duration", "short")
        }

    async def _generate_event_with_llm(
        self,
        world_state: Dict[str, Any],
        personality: str,
        goal_state: str,
        emotion_state: str,
        memory_state: str,
        social_context: str
    ) -> Dict[str, Any]:
        """Generate world event using LLM (with consciousness and social context)"""
        system_message = """You are a world event generator for a medieval fantasy MMORPG.
Generate interesting, lore-appropriate world events based on the current world state, the consciousness of key NPCs, and the social fabric of the world.
Events should be engaging and have potential gameplay impact.

Respond with JSON containing:
- type: event type (economic, political, environmental, social, magical)
- description: brief description of the event
- severity: 1-10 scale
- affected_areas: list of affected locations
- duration: short, medium, or long"""

        user_prompt = f"""Current world state:
Economy: {world_state.get('economy', {})}
Politics: {world_state.get('politics', {})}
Environment: {world_state.get('environment', {})}

Key NPC consciousness:
{personality}
{goal_state}
{emotion_state}
{memory_state}

Social fabric:
{social_context}

Generate an appropriate world event. Respond with JSON only."""

        response_text = await self._generate_with_llm(
            prompt=user_prompt,
            system_message=system_message,
            temperature=0.8
        )

        try:
            if "```json" in response_text:
                json_start = response_text.find("```json") + 7
                json_end = response_text.find("```", json_start)
                response_text = response_text[json_start:json_end].strip()
            event = json.loads(response_text)
            return event
        except json.JSONDecodeError:
            logger.warning("Failed to parse event generation response")
            return {
                "type": "general",
                "description": "A mysterious event occurred in the world",
                "severity": 5,
                "affected_areas": ["unknown"],
                "duration": "short"
            }

    async def get_map_info(self, map_name: str) -> Dict[str, Any]:
        """
        Get map information for pathfinding and movement decisions

        Args:
            map_name: Name of the map

        Returns:
            Map information including boundaries and walkable areas
        """
        logger.debug(f"Getting map info for: {map_name}")
        map_boundaries = {
            "prontera": {"width": 300, "height": 300},
            "geffen": {"width": 200, "height": 200},
            "morocc": {"width": 300, "height": 300},
            "payon": {"width": 200, "height": 200},
            "alberta": {"width": 200, "height": 200},
            "izlude": {"width": 200, "height": 200},
        }
        boundaries = map_boundaries.get(map_name, {"width": 200, "height": 200})
        return {
            "map_name": map_name,
            "width": boundaries["width"],
            "height": boundaries["height"],
            "walkable": True,
            "safe_zone": True
        }

    async def validate_movement_target(
        self,
        current_map: str,
        current_x: int,
        current_y: int,
        target_map: str,
        target_x: int,
        target_y: int
    ) -> Dict[str, Any]:
        """
        Validate if a movement target is reachable

        Args:
            current_map: Current map name
            current_x: Current X coordinate
            current_y: Current Y coordinate
            target_map: Target map name
            target_x: Target X coordinate
            target_y: Target Y coordinate

        Returns:
            Validation result with reachable flag and adjusted coordinates
        """
        logger.debug(f"Validating movement from {current_map}({current_x},{current_y}) to {target_map}({target_x},{target_y})")
        if current_map != target_map:
            logger.warning(f"Cross-map movement not supported: {current_map} -> {target_map}")
            return {
                "reachable": False,
                "reason": "Cross-map movement not supported",
                "adjusted_target": None
            }
        map_info = await self.get_map_info(target_map)
        adjusted_x = max(0, min(target_x, map_info["width"] - 1))
        adjusted_y = max(0, min(target_y, map_info["height"] - 1))
        if adjusted_x != target_x or adjusted_y != target_y:
            logger.info(f"Target coordinates adjusted: ({target_x},{target_y}) -> ({adjusted_x},{adjusted_y})")
        return {
            "reachable": True,
            "reason": "Target is reachable",
            "adjusted_target": {
                "map": target_map,
                "x": adjusted_x,
                "y": adjusted_y
            },
            "distance": abs(target_x - current_x) + abs(target_y - current_y)
        }