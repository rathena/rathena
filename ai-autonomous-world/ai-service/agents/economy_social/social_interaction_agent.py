"""
Social Interaction Agent - Creates Social Content for Community Bonding
Implements 4-tier LLM optimization for engaging social events
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from enum import Enum
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class SocialEventType(Enum):
    """Types of social events"""
    PICNIC = "picnic"                        # Party buffs
    DANCE_CHALLENGE = "dance_challenge"      # Mini-game
    PHOTO_SPOT = "photo_spot"                # Screenshot rewards
    GUILD_TASK = "guild_task"                # Daily guild missions
    COMMUNITY_GATHERING = "community_gathering"  # Social hub
    TRADING_FAIR = "trading_fair"            # Player marketplace boost


class SocialInteractionAgent(BaseAIAgent):
    """
    Spawns social events to boost community engagement and reduce solo play.
    
    Event Types:
    - Picnic NPC: Spawns in scenic areas, gives party-only buffs
    - Dance Challenge: Emote-based mini-game with small rewards
    - Photo Spot: Encourages screenshots, gives titles
    - Guild Task Board: Daily guild missions with rewards
    - Community Gathering: Social hub with chat bonuses
    - Trading Fair: Boosts player-to-player trade success
    
    Objectives:
    - Increase party formation rate
    - Encourage social interaction
    - Boost guild activity
    - Create memorable moments
    - Reduce solo play isolation
    
    4-Tier Optimization:
    - Tier 1: Rule-based event selection (0 LLM calls)
    - Tier 2: Cached event templates (0 LLM calls)
    - Tier 3: Batched event generation (1 call for multiple)
    - Tier 4: Creative event descriptions (optional)
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Social Interaction Agent"""
        super().__init__(
            agent_id="social_interaction_agent",
            agent_type="social",
            config=config
        )
        
        # Event configuration templates (Tier 2: Cached)
        self.event_templates = {
            SocialEventType.PICNIC: {
                'name_templates': [
                    'Afternoon Tea Party',
                    'Sunset Picnic',
                    'Garden Social',
                    'Lakeside Gathering'
                ],
                'requirements': {'min_party_size': 2, 'max_party_size': 12},
                'rewards': {
                    'buff': 'Social Gathering Bonus',
                    'duration_minutes': 60,
                    'stats': {'atk': 5, 'matk': 5, 'def': 5}
                },
                'duration_hours': 4,
                'spawn_locations': ['scenic', 'city']
            },
            SocialEventType.DANCE_CHALLENGE: {
                'name_templates': [
                    'Dance Off Challenge',
                    'Rhythm Festival',
                    'Emote Competition',
                    'Street Performance'
                ],
                'requirements': {'min_participants': 1, 'emote_sequence': True},
                'rewards': {
                    'items': [{'item_id': 607, 'amount': 1}],  # Yggdrasil Berry
                    'title': 'Dancer'
                },
                'duration_hours': 2,
                'spawn_locations': ['city', 'plaza']
            },
            SocialEventType.PHOTO_SPOT: {
                'name_templates': [
                    'Scenic Photo Opportunity',
                    'Landmark Viewpoint',
                    'Memory Lane',
                    'Picture Perfect Spot'
                ],
                'requirements': {'screenshot': True},
                'rewards': {
                    'title': 'Photographer',
                    'items': [{'item_id': 610, 'amount': 2}]  # Yggdrasil Leaf
                },
                'duration_hours': 6,
                'spawn_locations': ['scenic', 'landmark']
            },
            SocialEventType.GUILD_TASK: {
                'name_templates': [
                    'Guild Daily Mission',
                    'Guild Challenge',
                    'Cooperative Task',
                    'Guild Objective'
                ],
                'requirements': {'guild_only': True, 'min_participants': 3},
                'rewards': {
                    'guild_exp': 1000,
                    'items': [{'item_id': 985, 'amount': 5}]  # Elunium
                },
                'duration_hours': 24,
                'spawn_locations': ['guild_hall', 'city']
            },
            SocialEventType.COMMUNITY_GATHERING: {
                'name_templates': [
                    'Town Square Gathering',
                    'Community Social Hour',
                    'Player Meetup',
                    'Festival Square'
                ],
                'requirements': {'min_participants': 5},
                'rewards': {
                    'buff': 'Community Spirit',
                    'duration_minutes': 30,
                    'stats': {'exp_rate': 1.1}
                },
                'duration_hours': 3,
                'spawn_locations': ['city', 'plaza']
            },
            SocialEventType.TRADING_FAIR: {
                'name_templates': [
                    'Merchant Fair',
                    'Trading Bazaar',
                    'Market Festival',
                    'Commerce Day'
                ],
                'requirements': {'trade_activity': True},
                'rewards': {
                    'trade_bonus': 0.1,  # 10% better trades
                    'items': [{'item_id': 503, 'amount': 10}]  # Yellow Potion
                },
                'duration_hours': 5,
                'spawn_locations': ['city', 'market']
            }
        }
        
        # Guild task templates
        self.guild_task_templates = {
            'cooperative_hunting': {
                'description': 'Kill {count} {monster_type} as a guild',
                'objectives': {'monster_kills': True},
                'difficulty_range': (50, 200)
            },
            'resource_gathering': {
                'description': 'Collect {count} {item_name} items',
                'objectives': {'item_collection': True},
                'difficulty_range': (20, 100)
            },
            'guild_vs_guild': {
                'description': 'Win {count} GvG battles',
                'objectives': {'pvp_wins': True},
                'difficulty_range': (1, 5)
            },
            'boss_raid': {
                'description': 'Defeat {boss_name} together',
                'objectives': {'boss_defeat': True},
                'difficulty_range': (1, 3)
            },
            'exploration': {
                'description': 'Visit {count} different maps as guild',
                'objectives': {'map_visits': True},
                'difficulty_range': (5, 15)
            }
        }
        
        # Location types for event spawning
        self.location_categories = {
            'scenic': ['prt_fild08', 'pay_fild08', 'yuno_fild03'],
            'city': ['prontera', 'geffen', 'payon', 'morocc'],
            'plaza': ['prontera', 'geffen'],
            'landmark': ['yuno', 'aldebaran', 'comodo'],
            'market': ['prontera', 'alberta', 'geffen'],
            'guild_hall': ['prt_gld', 'pay_gld', 'gef_fild13']
        }
        
        logger.info("Social Interaction Agent initialized")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for social interaction management"""
        from crewai import Agent
        
        return Agent(
            role="Community Engagement Manager",
            goal="Create engaging social events to boost player interaction",
            backstory="An AI system that fosters community bonds and reduces solo play",
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
    
    async def spawn_daily_social_events(
        self,
        player_distribution: Dict[str, int],  # map -> solo/party players
        count: int = 5
    ) -> List[AgentResponse]:
        """
        Spawn 3-7 social events across the world.
        
        Selection Strategy (Tier 1: Rule-based):
        - Spawn in high-traffic areas (cities, popular maps)
        - Balance event types (don't repeat same type)
        - Favor locations with many solo players
        - Coordinate with other agents (avoid conflicts)
        
        Args:
            player_distribution: Map activity data
            count: Number of events to spawn (3-7)
            
        Returns:
            List of AgentResponse for each event
        """
        try:
            logger.info(f"Spawning {count} daily social events")
            
            # Get high-traffic maps with solo players (Tier 1: Rule-based)
            target_maps = self._select_spawn_locations(player_distribution, count)
            
            # Select diverse event types
            event_types = self._select_diverse_event_types(count)
            
            # Generate events
            responses = []
            for i, (map_name, event_type) in enumerate(zip(target_maps, event_types)):
                event_spec = await self._generate_event_spec(event_type, map_name, i)
                
                # Spawn event
                event_id = await self._spawn_social_event(event_spec)
                
                responses.append(AgentResponse(
                    agent_type=self.agent_type,
                    success=True,
                    data={
                        'event_id': event_id,
                        **event_spec
                    },
                    confidence=0.9,
                    reasoning=f"Spawned {event_type.value} in {map_name}"
                ))
            
            logger.info(f"✓ Spawned {len(responses)} social events")
            
            return responses
            
        except Exception as e:
            logger.error(f"Failed to spawn social events: {e}", exc_info=True)
            return []
    
    async def generate_guild_tasks(
        self,
        guild_count: int,
        average_guild_size: int = 10
    ) -> List[Dict[str, Any]]:
        """
        Generate daily guild tasks for all guilds.
        
        Task Types (Tier 1: Rule-based selection):
        - Cooperative hunting (kill X monsters as guild)
        - Resource gathering (collect X items)
        - Guild vs Guild (PvP challenge)
        - Boss raid (defeat guild boss together)
        - Exploration (visit X maps as guild)
        
        Args:
            guild_count: Number of active guilds
            average_guild_size: Average members per guild
            
        Returns:
            List of guild tasks
        """
        try:
            logger.info(f"Generating guild tasks for {guild_count} guilds")
            
            tasks = []
            
            # Generate 3 tasks per guild
            for guild_id in range(1, guild_count + 1):
                guild_tasks = []
                
                # Select diverse task types
                task_types = random.sample(
                    list(self.guild_task_templates.keys()),
                    min(3, len(self.guild_task_templates))
                )
                
                for task_type in task_types:
                    task_spec = await self._generate_guild_task_spec(
                        guild_id,
                        task_type,
                        average_guild_size
                    )
                    
                    # Store task
                    task_id = await self._store_guild_task(task_spec)
                    task_spec['task_id'] = task_id
                    
                    guild_tasks.append(task_spec)
                
                tasks.extend(guild_tasks)
            
            logger.info(f"✓ Generated {len(tasks)} guild tasks")
            
            return tasks
            
        except Exception as e:
            logger.error(f"Failed to generate guild tasks: {e}", exc_info=True)
            return []
    
    async def handle_social_participation(
        self,
        event_id: int,
        participants: List[int],
        event_type: SocialEventType,
        party_id: Optional[int] = None
    ) -> AgentResponse:
        """
        Process social event participation and distribute rewards.
        
        Actions:
        - Verify participation requirements (party size, etc.)
        - Distribute rewards
        - Award reputation (Event Participant)
        - Record participation
        - Update social stats
        
        Args:
            event_id: Social event ID
            participants: List of participant player IDs
            event_type: Type of social event
            party_id: Optional party ID
            
        Returns:
            AgentResponse with participation results
        """
        try:
            logger.info(f"Processing participation for event {event_id}, {len(participants)} participants")
            
            # Get event requirements
            template = self.event_templates.get(event_type, {})
            requirements = template.get('requirements', {})
            
            # Validate requirements (Tier 1: Rule-based)
            if 'min_party_size' in requirements:
                if not party_id or len(participants) < requirements['min_party_size']:
                    return AgentResponse(
                        agent_type=self.agent_type,
                        success=False,
                        data={'error': 'Party size requirement not met'},
                        confidence=1.0,
                        reasoning=f"Requires party of {requirements['min_party_size']}+"
                    )
            
            if 'min_participants' in requirements:
                if len(participants) < requirements['min_participants']:
                    return AgentResponse(
                        agent_type=self.agent_type,
                        success=False,
                        data={'error': 'Minimum participants not met'},
                        confidence=1.0,
                        reasoning=f"Requires {requirements['min_participants']}+ participants"
                    )
            
            # Distribute rewards
            rewards_distributed = await self._distribute_event_rewards(
                participants,
                template.get('rewards', {}),
                event_type
            )
            
            # Record participation
            await self._record_social_participation(
                event_id,
                participants,
                party_id,
                rewards_distributed
            )
            
            # Update event stats
            await self._update_event_stats(event_id, len(participants))
            
            result_data = {
                'event_id': event_id,
                'participants': len(participants),
                'party_id': party_id,
                'rewards_distributed': rewards_distributed,
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Processed participation for {len(participants)} players")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Participation processed successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to handle participation: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def analyze_social_health(
        self,
        metrics: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Analyze community health and social engagement.
        
        Metrics analyzed:
        - Party formation rate
        - Average party size
        - Guild activity rate
        - Trade frequency
        - Social event participation
        
        Args:
            metrics: Current social metrics
            
        Returns:
            Social health analysis
        """
        try:
            # Extract metrics
            party_rate = metrics.get('party_formation_rate', 0.0)
            avg_party_size = metrics.get('average_party_size', 1.0)
            guild_activity = metrics.get('guild_activity_rate', 0.0)
            trade_freq = metrics.get('trade_frequency', 0)
            event_participation = metrics.get('event_participation_rate', 0.0)
            
            # Calculate social health score (Tier 1: Rule-based)
            health_score = (
                party_rate * 0.3 +
                (avg_party_size / 6.0) * 0.2 +
                guild_activity * 0.2 +
                min(trade_freq / 100.0, 1.0) * 0.15 +
                event_participation * 0.15
            )
            
            # Determine health status
            if health_score >= 0.7:
                status = 'healthy'
                recommendations = ['Maintain current social event frequency']
            elif health_score >= 0.4:
                status = 'fair'
                recommendations = [
                    'Increase party-focused events',
                    'Add guild task variety',
                    'Promote trading fairs'
                ]
            else:
                status = 'poor'
                recommendations = [
                    'Significantly increase social event spawns',
                    'Add party formation incentives',
                    'Create guild recruitment events',
                    'Boost social event rewards'
                ]
            
            analysis = {
                'health_score': round(health_score, 3),
                'status': status,
                'party_formation_rate': party_rate,
                'average_party_size': avg_party_size,
                'guild_activity_rate': guild_activity,
                'trade_frequency': trade_freq,
                'event_participation_rate': event_participation,
                'recommendations': recommendations,
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Store analysis
            await self._store_social_metrics(metrics)
            
            logger.info(f"Social health: {status} (score: {health_score:.2f})")
            
            return analysis
            
        except Exception as e:
            logger.error(f"Failed to analyze social health: {e}")
            return {'health_score': 0.0, 'status': 'unknown'}
    
    async def get_active_social_events(self) -> List[Dict[str, Any]]:
        """Get all currently active social events"""
        try:
            query = """
                SELECT *
                FROM social_events
                WHERE status = 'active' AND despawns_at > NOW()
                ORDER BY spawned_at DESC
            """
            
            events = await postgres_db.fetch_all(query)
            
            event_list = []
            for event in events:
                event_dict = dict(event)
                
                # Parse JSON fields
                for field in ['requirements', 'rewards']:
                    if event_dict.get(field):
                        event_dict[field] = json.loads(event_dict[field])
                
                event_list.append(event_dict)
            
            return event_list
            
        except Exception as e:
            logger.error(f"Failed to get active events: {e}")
            return []
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _select_spawn_locations(
        self,
        player_distribution: Dict[str, int],
        count: int
    ) -> List[str]:
        """Select spawn locations based on player distribution"""
        # Sort maps by player count (prefer high-traffic)
        sorted_maps = sorted(
            player_distribution.items(),
            key=lambda x: x[1],
            reverse=True
        )
        
        # Select top maps
        target_maps = [map_name for map_name, _ in sorted_maps[:count]]
        
        # Fill remaining with popular locations
        while len(target_maps) < count:
            target_maps.append(random.choice(self.location_categories['city']))
        
        return target_maps[:count]
    
    def _select_diverse_event_types(self, count: int) -> List[SocialEventType]:
        """Select diverse event types without repetition"""
        event_types = list(SocialEventType)
        
        if count <= len(event_types):
            return random.sample(event_types, count)
        else:
            # Repeat with shuffling
            result = []
            while len(result) < count:
                result.extend(random.sample(event_types, len(event_types)))
            return result[:count]
    
    async def _generate_event_spec(
        self,
        event_type: SocialEventType,
        map_name: str,
        index: int
    ) -> Dict[str, Any]:
        """Generate event specification from template"""
        template = self.event_templates.get(event_type, {})
        
        # Select random name
        name = random.choice(template.get('name_templates', ['Social Event']))
        
        # Generate spawn coordinates (simplified - would use actual map data)
        spawn_x = random.randint(100, 200)
        spawn_y = random.randint(100, 200)
        
        # Calculate despawn time
        duration_hours = template.get('duration_hours', 4)
        despawns_at = datetime.now(UTC) + timedelta(hours=duration_hours)
        
        return {
            'event_type': event_type.value,
            'event_name': name,
            'spawn_map': map_name,
            'spawn_x': spawn_x,
            'spawn_y': spawn_y,
            'requirements': template.get('requirements', {}),
            'rewards': template.get('rewards', {}),
            'despawns_at': despawns_at.isoformat()
        }
    
    async def _spawn_social_event(self, event_spec: Dict[str, Any]) -> int:
        """Spawn social event in database"""
        try:
            query = """
                INSERT INTO social_events
                (event_type, event_name, spawn_map, spawn_x, spawn_y, 
                 requirements, rewards, despawns_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                RETURNING event_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                event_spec['event_type'],
                event_spec['event_name'],
                event_spec['spawn_map'],
                event_spec['spawn_x'],
                event_spec['spawn_y'],
                json.dumps(event_spec['requirements']),
                json.dumps(event_spec['rewards']),
                event_spec['despawns_at']
            )
            
            return result['event_id']
            
        except Exception as e:
            logger.error(f"Failed to spawn social event: {e}")
            return 0
    
    async def _generate_guild_task_spec(
        self,
        guild_id: int,
        task_type: str,
        guild_size: int
    ) -> Dict[str, Any]:
        """Generate guild task specification"""
        template = self.guild_task_templates.get(task_type, {})
        
        # Scale difficulty by guild size
        min_diff, max_diff = template.get('difficulty_range', (10, 50))
        threshold = int((min_diff + max_diff) / 2 * (guild_size / 10))
        
        # Generate task description
        description = template.get('description', 'Complete guild task')
        description = description.format(
            count=threshold,
            monster_type='monsters',
            item_name='items',
            boss_name='Boss Monster'
        )
        
        # Calculate rewards
        rewards = {
            'guild_exp': threshold * 10,
            'items': [
                {'item_id': 985, 'amount': max(1, threshold // 10)}  # Elunium
            ]
        }
        
        return {
            'guild_id': guild_id,
            'task_type': task_type,
            'task_description': description,
            'objectives': template.get('objectives', {}),
            'completion_threshold': threshold,
            'rewards': rewards,
            'expires_at': (datetime.now(UTC) + timedelta(hours=24)).isoformat()
        }
    
    async def _store_guild_task(self, task_spec: Dict[str, Any]) -> int:
        """Store guild task in database"""
        try:
            query = """
                INSERT INTO guild_tasks
                (guild_id, task_type, task_description, objectives, 
                 rewards, completion_threshold, expires_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING task_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                task_spec['guild_id'],
                task_spec['task_type'],
                task_spec['task_description'],
                json.dumps(task_spec['objectives']),
                json.dumps(task_spec['rewards']),
                task_spec['completion_threshold'],
                task_spec['expires_at']
            )
            
            return result['task_id']
            
        except Exception as e:
            logger.error(f"Failed to store guild task: {e}")
            return 0
    
    async def _distribute_event_rewards(
        self,
        participants: List[int],
        rewards: Dict[str, Any],
        event_type: SocialEventType
    ) -> Dict[str, Any]:
        """Distribute rewards to participants"""
        distributed = {
            'participants': len(participants),
            'buffs': [],
            'items': [],
            'titles': []
        }
        
        # Distribute buffs
        if 'buff' in rewards:
            distributed['buffs'].append({
                'name': rewards['buff'],
                'duration': rewards.get('duration_minutes', 60)
            })
        
        # Distribute items
        if 'items' in rewards:
            distributed['items'] = rewards['items']
        
        # Award titles
        if 'title' in rewards:
            distributed['titles'].append(rewards['title'])
        
        return distributed
    
    async def _record_social_participation(
        self,
        event_id: int,
        participants: List[int],
        party_id: Optional[int],
        rewards: Dict
    ):
        """Record social event participation"""
        try:
            query = """
                INSERT INTO social_participation
                (event_id, player_ids, party_id, rewards_distributed)
                VALUES ($1, $2, $3, $4)
            """
            
            await postgres_db.execute(
                query,
                event_id,
                json.dumps(participants),
                party_id,
                json.dumps(rewards)
            )
            
        except Exception as e:
            logger.error(f"Failed to record participation: {e}")
    
    async def _update_event_stats(self, event_id: int, participant_count: int):
        """Update event participation stats"""
        try:
            query = """
                UPDATE social_events
                SET participation_count = participation_count + $1
                WHERE event_id = $2
            """
            
            await postgres_db.execute(query, participant_count, event_id)
            
        except Exception as e:
            logger.error(f"Failed to update event stats: {e}")
    
    async def _store_social_metrics(self, metrics: Dict[str, Any]):
        """Store social metrics snapshot"""
        try:
            query = """
                INSERT INTO social_metrics
                (party_formation_rate, average_party_size, guild_activity_rate,
                 trade_frequency, social_event_participation_rate)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                metrics.get('party_formation_rate', 0.0),
                metrics.get('average_party_size', 1.0),
                metrics.get('guild_activity_rate', 0.0),
                metrics.get('trade_frequency', 0),
                metrics.get('event_participation_rate', 0.0)
            )
            
        except Exception as e:
            logger.error(f"Failed to store social metrics: {e}")


# Global instance
social_interaction_agent = SocialInteractionAgent()