"""
Problem Agent - Generates Daily World Problems
Implements 4-tier LLM optimization for cost-effective problem generation
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings
from models.procedural import (
    ProblemType, ProblemStatus, ProblemSpawnData, ProblemRewardData
)


class ProblemAgent(BaseAIAgent):
    """
    Generates daily world problems based on server state using 4-tier optimization.
    
    Problem Types:
    - Monster Surge: Monsters multiply in a region
    - MVP Rampage: MVP spawns in unusual map
    - Good vs Evil: Faction warfare event
    - Resource Shortage: Encourage farming specific items
    - Pollution/Weather: Environmental hazards
    
    4-Tier Optimization:
    - Tier 1: Rule-based (0 LLM calls) - Simple threshold checks
    - Tier 2: Cached (0 LLM calls) - Reuse similar past decisions
    - Tier 3: Batched LLM (1 call for multiple) - Batch similar requests
    - Tier 4: Full LLM - Complex creative generation
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Problem Agent"""
        super().__init__(
            agent_id="problem_agent",
            agent_type="problem",
            config=config
        )
        
        # Problem type configurations
        self.problem_types = {
            ProblemType.MONSTER_SURGE: {
                "min_level": 30,
                "difficulty_range": (2, 8),
                "duration_hours": (12, 24),
                "description_template": "A surge of {monster_type} has invaded {map_name}! Help eliminate the threat!"
            },
            ProblemType.MVP_RAMPAGE: {
                "min_level": 50,
                "difficulty_range": (5, 10),
                "duration_hours": (8, 16),
                "description_template": "The MVP {mvp_name} has gone berserk in {map_name}!"
            },
            ProblemType.GOOD_VS_EVIL: {
                "min_level": 40,
                "difficulty_range": (4, 9),
                "duration_hours": (12, 24),
                "description_template": "A battle between good and evil forces rages in {map_name}!"
            },
            ProblemType.RESOURCE_SHORTAGE: {
                "min_level": 20,
                "difficulty_range": (1, 5),
                "duration_hours": (16, 24),
                "description_template": "A shortage of {resource} has struck! Gather supplies to help!"
            },
            ProblemType.POLLUTION: {
                "min_level": 25,
                "difficulty_range": (2, 6),
                "duration_hours": (12, 20),
                "description_template": "Pollution has corrupted {map_name}! Cleanse the area!"
            },
            ProblemType.WEATHER_HAZARD: {
                "min_level": 15,
                "difficulty_range": (1, 4),
                "duration_hours": (6, 12),
                "description_template": "Dangerous weather threatens {map_name}!"
            }
        }
        
        # Difficulty scaling for rewards
        self.difficulty_scaling = {
            1: {"exp_mult": 1.0, "zeny_mult": 1.0},
            2: {"exp_mult": 1.2, "zeny_mult": 1.2},
            3: {"exp_mult": 1.5, "zeny_mult": 1.3},
            4: {"exp_mult": 1.8, "zeny_mult": 1.5},
            5: {"exp_mult": 2.2, "zeny_mult": 1.8},
            6: {"exp_mult": 2.8, "zeny_mult": 2.2},
            7: {"exp_mult": 3.5, "zeny_mult": 2.8},
            8: {"exp_mult": 4.5, "zeny_mult": 3.5},
            9: {"exp_mult": 6.0, "zeny_mult": 4.5},
            10: {"exp_mult": 8.0, "zeny_mult": 6.0}
        }
        
        logger.info(f"Problem Agent initialized with {len(self.problem_types)} problem types")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for problem generation"""
        from crewai import Agent
        
        return Agent(
            role="World Problem Generator",
            goal="Generate engaging daily problems based on server metrics",
            backstory="An AI system that monitors the world and creates dynamic challenges",
            verbose=settings.crewai_verbose,
            allow_delegation=False,
            llm=self.llm_provider if hasattr(self, 'llm_provider') else None
        )
    
    async def _process(self, context: AgentContext) -> AgentResponse:
        """Process method required by BaseAIAgent (not used directly)"""
        return AgentResponse(
            agent_type=self.agent_type,
            success=True,
            data={},
            confidence=1.0
        )
    
    # ========================================================================
    # PUBLIC API METHODS
    # ========================================================================
    
    async def generate_daily_problem(
        self, 
        world_state: Dict[str, Any],
        force_type: Optional[str] = None
    ) -> AgentResponse:
        """
        Generate a single problem for the day based on world metrics.
        
        Args:
            world_state: Current world metrics
            force_type: Optional forced problem type (admin override)
            
        Returns:
            AgentResponse with problem details
        """
        try:
            logger.info("Generating daily problem...")
            
            # Check if a problem already exists today
            existing_problem = await self._get_active_problem()
            if existing_problem:
                logger.info(f"Active problem already exists: {existing_problem['title']}")
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=True,
                    data=existing_problem,
                    confidence=1.0,
                    reasoning="Active problem already exists for today"
                )
            
            # Step 1: Decide problem type using 4-tier system
            problem_type = await self.decide_problem_type(world_state, force_type)
            
            # Step 2: Generate problem details
            problem_data = await self._generate_problem_details(problem_type, world_state)
            
            # Step 3: Calculate rewards
            rewards = await self.calculate_rewards(
                problem_type,
                problem_data['difficulty'],
                world_state
            )
            
            # Step 4: Store in database
            problem_id = await self._store_problem(problem_data, rewards)
            
            # Step 5: Cache in DragonflyDB
            await self._cache_problem(problem_id, problem_data, rewards)
            
            result_data = {
                "problem_id": problem_id,
                **problem_data,
                "reward_data": rewards
            }
            
            logger.info(f"✓ Generated problem #{problem_id}: {problem_data['title']}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.9,
                reasoning=f"Generated {problem_type} problem"
            )
            
        except Exception as e:
            logger.error(f"Failed to generate daily problem: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def decide_problem_type(
        self,
        world_state: Dict[str, Any],
        force_type: Optional[str] = None
    ) -> str:
        """
        Use 4-tier decision system to select problem type.
        
        Tier 1: Rule-based (0 LLM calls)
        Tier 2: Cached decisions (0 LLM calls)
        Tier 3: Batched LLM (1 call for multiple)
        Tier 4: Full LLM call
        
        Args:
            world_state: Current world metrics
            force_type: Optional forced problem type
            
        Returns:
            Selected problem type
        """
        if force_type:
            return force_type
        
        # TIER 1: Rule-based decisions
        avg_level = world_state.get('avg_player_level', 50)
        online_players = world_state.get('online_players', 0)
        
        # Filter problems by average level
        available_problems = [
            ptype for ptype, config in self.problem_types.items()
            if avg_level >= config['min_level']
        ]
        
        if not available_problems:
            # Default to easiest problem if level too low
            available_problems = [ProblemType.WEATHER_HAZARD]
        
        # Rule: Low player count = easier problems
        if online_players < 10:
            easy_problems = [
                pt for pt in available_problems 
                if self.problem_types[pt]['difficulty_range'][0] <= 3
            ]
            if easy_problems:
                selected = random.choice(easy_problems)
                logger.info(f"[Tier 1] Selected {selected} (low player count rule)")
                return selected
        
        # TIER 2: Check cached decision
        cache_key = self._generate_cache_key(world_state)
        cached_decision = await self._get_cached_decision(cache_key)
        
        if cached_decision:
            logger.info(f"[Tier 2] Using cached decision: {cached_decision}")
            return cached_decision
        
        # TIER 3 & 4: LLM-based decision (skip for now, use weighted random)
        # In production, this would call LLM with world state context
        
        # Weighted selection based on recent problems
        recent_problems = await self._get_recent_problem_types(days=7)
        weights = self._calculate_problem_weights(available_problems, recent_problems)
        
        selected = random.choices(available_problems, weights=weights, k=1)[0]
        
        # Cache decision for 24 hours
        await self._cache_decision(cache_key, selected, ttl=86400)
        
        logger.info(f"[Tier 1+] Selected {selected} (weighted random)")
        return selected
    
    async def calculate_rewards(
        self,
        problem_type: str,
        difficulty: int,
        world_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Calculate adaptive rewards based on problem and world state.
        
        Args:
            problem_type: Type of problem
            difficulty: Problem difficulty (1-10)
            world_state: Current world metrics
            
        Returns:
            Reward structure
        """
        avg_level = world_state.get('avg_player_level', 50)
        online_players = world_state.get('online_players', 10)
        
        # Base rewards scale with level
        base_exp = int(avg_level * 1000 * (1 + difficulty * 0.2))
        base_zeny = int(avg_level * 500 * (1 + difficulty * 0.15))
        
        # Apply difficulty scaling
        scaling = self.difficulty_scaling.get(difficulty, self.difficulty_scaling[5])
        exp_reward = int(base_exp * scaling['exp_mult'])
        zeny_reward = int(base_zeny * scaling['zeny_mult'])
        
        # Participation bonus (more players = higher rewards)
        participation_mult = min(1.5, 1.0 + (online_players / 100))
        exp_reward = int(exp_reward * participation_mult)
        zeny_reward = int(zeny_reward * participation_mult)
        
        # Item rewards based on difficulty
        items = []
        if difficulty >= 3:
            items.append({"item_id": 607, "amount": difficulty, "name": "Yggdrasil Berry"})
        if difficulty >= 5:
            items.append({"item_id": 608, "amount": difficulty // 2, "name": "Yggdrasil Seed"})
        if difficulty >= 7:
            items.append({"item_id": 617, "amount": 1, "name": "Old Violet Box"})
        if difficulty >= 9:
            items.append({"item_id": 616, "amount": 1, "name": "Old Card Album"})
        
        return {
            "exp": exp_reward,
            "zeny": zeny_reward,
            "items": items,
            "special": {
                "participation_bonus": participation_mult > 1.0,
                "difficulty_tier": "high" if difficulty >= 7 else "medium" if difficulty >= 4 else "low"
            }
        }
    
    async def record_participation(
        self,
        problem_id: int,
        player_id: int,
        action_type: str,
        action_data: Optional[Dict[str, Any]] = None,
        contribution_score: int = 0
    ) -> bool:
        """
        Record player participation in problem.
        
        Args:
            problem_id: Problem ID
            player_id: Player ID
            action_type: Type of action performed
            action_data: Additional action data
            contribution_score: Player's contribution score
            
        Returns:
            Success status
        """
        try:
            query = """
                INSERT INTO problem_history 
                (problem_id, player_id, action_type, action_data, contribution_score)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                problem_id,
                player_id,
                action_type,
                json.dumps(action_data) if action_data else None,
                contribution_score
            )
            
            # Update participation count
            await postgres_db.execute(
                "UPDATE world_problems SET participation_count = participation_count + 1 WHERE problem_id = $1",
                problem_id
            )
            
            logger.debug(f"Recorded participation: player {player_id} in problem {problem_id}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to record participation: {e}")
            return False
    
    async def get_current_problem(self) -> Optional[Dict[str, Any]]:
        """Get currently active problem"""
        return await self._get_active_problem()
    
    async def complete_problem(self, problem_id: int) -> bool:
        """
        Mark problem as completed.
        
        Args:
            problem_id: Problem ID
            
        Returns:
            Success status
        """
        try:
            query = """
                UPDATE world_problems 
                SET status = $1, completed_at = NOW()
                WHERE problem_id = $2 AND status = 'active'
            """
            
            rows_affected = await postgres_db.execute(query, ProblemStatus.COMPLETED.value, problem_id)
            
            if rows_affected > 0:
                # Clear cache
                await db.delete(f"problem:active")
                logger.info(f"✓ Problem #{problem_id} marked as completed")
                return True
            
            return False
            
        except Exception as e:
            logger.error(f"Failed to complete problem: {e}")
            return False
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _generate_problem_details(
        self,
        problem_type: str,
        world_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Generate problem details including title, description, spawn data"""
        config = self.problem_types[problem_type]
        avg_level = world_state.get('avg_player_level', 50)
        
        # Calculate difficulty based on average level
        min_diff, max_diff = config['difficulty_range']
        difficulty = min(max_diff, max(min_diff, avg_level // 15))
        
        # Select target map based on activity
        map_activity = world_state.get('map_activity', {})
        target_map = self._select_target_map(map_activity, difficulty)
        
        # Generate duration
        min_hours, max_hours = config['duration_hours']
        duration_hours = random.randint(min_hours, max_hours)
        
        # Use LLM for creative title/description (Tier 4) or template (Tier 1)
        if settings.rule_based_decisions_enabled:
            # Tier 1: Template-based
            title = f"{problem_type.replace('_', ' ').title()} - Day {datetime.now(UTC).day}"
            description = config['description_template'].format(
                monster_type="monsters",
                map_name=target_map,
                mvp_name="Boss Monster",
                resource="materials"
            )
        else:
            # Tier 4: LLM-generated (simplified for now)
            title, description = await self._generate_creative_problem(problem_type, target_map)
        
        # Generate spawn data
        spawn_data = None
        if problem_type in [ProblemType.MONSTER_SURGE, ProblemType.MVP_RAMPAGE]:
            spawn_data = {
                "map_name": target_map,
                "spawn_count": difficulty * 10,
                "mob_id": 1002 + difficulty,  # Simplified mob ID
                "spawn_coords": []  # Will be generated by Bridge Layer
            }
        
        return {
            "problem_type": problem_type,
            "title": title,
            "description": description,
            "target_map": target_map,
            "spawn_data": spawn_data,
            "difficulty": difficulty,
            "status": ProblemStatus.ACTIVE.value,
            "expires_at": datetime.now(UTC) + timedelta(hours=duration_hours)
        }
    
    async def _generate_creative_problem(
        self,
        problem_type: str,
        target_map: str
    ) -> tuple[str, str]:
        """Generate creative problem using LLM (Tier 4)"""
        # Simplified - in production this would call LLM
        problem_names = {
            ProblemType.MONSTER_SURGE: "Monster Invasion",
            ProblemType.MVP_RAMPAGE: "Boss Rampage",
            ProblemType.GOOD_VS_EVIL: "Faction War",
            ProblemType.RESOURCE_SHORTAGE: "Resource Crisis",
            ProblemType.POLLUTION: "Environmental Hazard",
            ProblemType.WEATHER_HAZARD: "Severe Weather"
        }
        
        title = f"{problem_names.get(problem_type, 'World Problem')} in {target_map}"
        description = f"A critical situation has emerged in {target_map}. Adventurers needed!"
        
        return title, description
    
    def _select_target_map(
        self,
        map_activity: Dict[str, int],
        difficulty: int
    ) -> str:
        """Select target map based on activity and difficulty"""
        if not map_activity:
            return "prontera"
        
        # Prefer maps with moderate activity
        sorted_maps = sorted(map_activity.items(), key=lambda x: x[1], reverse=True)
        
        # High difficulty = high activity maps, low difficulty = low activity maps
        if difficulty >= 7:
            return sorted_maps[0][0] if sorted_maps else "prontera"
        elif difficulty >= 4:
            mid = len(sorted_maps) // 2
            return sorted_maps[mid][0] if sorted_maps else "geffen"
        else:
            return sorted_maps[-1][0] if sorted_maps else "payon"
    
    def _generate_cache_key(self, world_state: Dict[str, Any]) -> str:
        """Generate cache key from world state"""
        # Create deterministic key based on relevant state
        key_data = {
            "avg_level": world_state.get('avg_player_level', 50) // 10 * 10,  # Round to nearest 10
            "online": world_state.get('online_players', 0) // 5 * 5,  # Round to nearest 5
            "date": datetime.now(UTC).strftime("%Y-%m-%d")
        }
        
        key_str = json.dumps(key_data, sort_keys=True)
        return f"problem:decision:{hashlib.md5(key_str.encode()).hexdigest()}"
    
    async def _get_cached_decision(self, cache_key: str) -> Optional[str]:
        """Get cached problem type decision"""
        try:
            cached = await db.get(cache_key)
            return cached if cached else None
        except Exception as e:
            logger.debug(f"Cache miss for {cache_key}: {e}")
            return None
    
    async def _cache_decision(self, cache_key: str, problem_type: str, ttl: int = 86400):
        """Cache problem type decision"""
        try:
            await db.set(cache_key, problem_type, expire=ttl)
        except Exception as e:
            logger.warning(f"Failed to cache decision: {e}")
    
    async def _get_recent_problem_types(self, days: int = 7) -> List[str]:
        """Get problem types from recent history"""
        try:
            query = """
                SELECT problem_type 
                FROM world_problems 
                WHERE created_at > NOW() - INTERVAL '%s days'
                ORDER BY created_at DESC
            """
            
            rows = await postgres_db.fetch_all(query, days)
            return [row['problem_type'] for row in rows]
            
        except Exception as e:
            logger.debug(f"Failed to get recent problems: {e}")
            return []
    
    def _calculate_problem_weights(
        self,
        available_problems: List[str],
        recent_problems: List[str]
    ) -> List[float]:
        """Calculate weights to avoid repetition"""
        weights = []
        
        for problem in available_problems:
            # Count occurrences in recent history
            recent_count = recent_problems.count(problem)
            
            # Weight inversely proportional to recent usage
            weight = max(0.1, 1.0 - (recent_count * 0.2))
            weights.append(weight)
        
        return weights
    
    async def _store_problem(
        self,
        problem_data: Dict[str, Any],
        rewards: Dict[str, Any]
    ) -> int:
        """Store problem in PostgreSQL"""
        query = """
            INSERT INTO world_problems 
            (problem_type, title, description, target_map, spawn_data, reward_data, difficulty, status, expires_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
            RETURNING problem_id
        """
        
        result = await postgres_db.fetch_one(
            query,
            problem_data['problem_type'],
            problem_data['title'],
            problem_data['description'],
            problem_data['target_map'],
            json.dumps(problem_data['spawn_data']) if problem_data['spawn_data'] else None,
            json.dumps(rewards),
            problem_data['difficulty'],
            problem_data['status'],
            problem_data['expires_at']
        )
        
        return result['problem_id']
    
    async def _cache_problem(
        self,
        problem_id: int,
        problem_data: Dict[str, Any],
        rewards: Dict[str, Any]
    ):
        """Cache problem in DragonflyDB"""
        try:
            cache_data = {
                "problem_id": problem_id,
                **problem_data,
                "rewards": rewards
            }
            
            # Cache active problem
            await db.set("problem:active", cache_data, expire=86400)  # 24 hours
            
            # Cache specific problem
            await db.set(f"problem:{problem_id}", cache_data, expire=86400)
            
        except Exception as e:
            logger.warning(f"Failed to cache problem: {e}")
    
    async def _get_active_problem(self) -> Optional[Dict[str, Any]]:
        """Get currently active problem from cache or database"""
        try:
            # Try cache first
            cached = await db.get("problem:active")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT * FROM world_problems
                WHERE status = 'active' AND expires_at > NOW()
                ORDER BY created_at DESC
                LIMIT 1
            """
            
            problem = await postgres_db.fetch_one(query)
            
            if problem:
                # Parse JSON fields
                problem_dict = dict(problem)
                if problem_dict.get('spawn_data'):
                    problem_dict['spawn_data'] = json.loads(problem_dict['spawn_data'])
                if problem_dict.get('reward_data'):
                    problem_dict['reward_data'] = json.loads(problem_dict['reward_data'])
                
                # Cache for future requests
                await db.set("problem:active", problem_dict, expire=3600)
                
                return problem_dict
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to get active problem: {e}")
            return None


# Global instance
problem_agent = ProblemAgent()