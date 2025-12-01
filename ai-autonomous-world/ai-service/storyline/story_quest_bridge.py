"""
Story-Quest Bridge
Converts LLM-generated story quests to rAthena-compatible format
"""

import asyncio
import json
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from database import postgres_db, db
from config import settings
from models.storyline import StoryQuest, StoryQuestSpec, StoryQuestProgress


class StoryQuestBridge:
    """
    Converts story arc quests to rAthena-compatible format.
    
    Handles:
    - Quest ID allocation
    - Objective translation (narrative → game mechanics)
    - Reward calculation
    - Progress tracking
    - Completion detection
    - Integration with QuestAgent
    
    Quest ID Range: 8000-8999 (reserved for story quests)
    """
    
    def __init__(self):
        """Initialize Story-Quest Bridge"""
        self.quest_id_range = (8000, 8999)
        self.next_quest_id = self.quest_id_range[0]
        
        logger.info(
            f"Story-Quest Bridge initialized "
            f"(Quest ID range: {self.quest_id_range[0]}-{self.quest_id_range[1]})"
        )
    
    async def create_story_quest(
        self,
        quest_spec: StoryQuest,
        arc_id: int,
        chapter_number: int,
        giver_npc_id: int
    ) -> int:
        """
        Create quest from story specification.
        
        Translation:
        - LLM narrative → rAthena quest format
        - Monster names → Monster IDs
        - Item names → Item IDs
        - Map names → Map IDs (if applicable)
        
        Process:
        1. Allocate quest ID
        2. Parse objectives
        3. Validate rewards
        4. Convert to rAthena format
        5. Store in database
        6. Register with QuestAgent
        
        Args:
            quest_spec: Quest from story arc
            arc_id: Parent arc ID
            chapter_number: Chapter number
            giver_npc_id: NPC who gives the quest
            
        Returns:
            Quest ID
        """
        try:
            logger.info(f"Creating story quest: {quest_spec.title}")
            
            # Allocate quest ID
            quest_id = await self._allocate_quest_id()
            
            # Parse and validate objectives
            objectives_data = await self._parse_objectives(quest_spec)
            
            # Validate rewards
            rewards_data = await self._validate_rewards(quest_spec.reward)
            
            # Build rAthena-compatible quest structure
            rathena_quest = {
                'quest_id': quest_id,
                'quest_name': quest_spec.title,
                'quest_desc': quest_spec.objective,
                'min_level': 1,  # TODO: Extract from story context
                'max_level': 999,
                'objectives': objectives_data,
                'rewards': rewards_data,
                'time_limit': 0,  # No time limit by default
                'repeatable': False,
                'quest_giver_npc': giver_npc_id,
                'quest_type': 'story'
            }
            
            # Store in database
            query = """
                INSERT INTO story_quests
                (quest_id, arc_id, chapter_number, giver_npc_id, quest_data, 
                 status, created_at)
                VALUES ($1, $2, $3, $4, $5, $6, NOW())
                RETURNING quest_id
            """
            
            await postgres_db.execute(
                query,
                quest_id,
                arc_id,
                chapter_number,
                giver_npc_id,
                json.dumps(rathena_quest),
                'active'
            )
            
            # Register with QuestAgent (store in cache)
            await self._register_with_quest_agent(quest_id, rathena_quest)
            
            # Update NPC's quest_data
            await postgres_db.execute(
                """
                UPDATE story_npcs
                SET quest_data = $1
                WHERE npc_id = $2
                """,
                json.dumps({'quest_id': quest_id, 'quest_title': quest_spec.title}),
                giver_npc_id
            )
            
            logger.info(f"✓ Story quest #{quest_id} created: {quest_spec.title}")
            
            return quest_id
            
        except Exception as e:
            logger.error(f"Failed to create story quest: {e}", exc_info=True)
            raise
    
    async def track_quest_progress(
        self,
        quest_id: int,
        player_id: int,
        progress_data: Dict[str, Any]
    ) -> StoryQuestProgress:
        """
        Track player progress on story quest.
        
        Used for:
        - Chapter advancement decisions
        - Success/failure outcomes
        - Reward distribution
        - Contribution score calculation
        
        Args:
            quest_id: Quest ID
            player_id: Player ID
            progress_data: Progress update data
            
        Returns:
            Updated quest progress
        """
        try:
            # Get quest details
            quest = await self._get_quest_by_id(quest_id)
            if not quest:
                raise ValueError(f"Quest #{quest_id} not found")
            
            quest_data = json.loads(quest['quest_data']) if isinstance(quest['quest_data'], str) else quest['quest_data']
            total_objectives = len(quest_data.get('objectives', []))
            
            # Calculate progress
            objectives_completed = progress_data.get('objectives_completed', 0)
            progress_percent = (objectives_completed / total_objectives * 100) if total_objectives > 0 else 0.0
            is_completed = objectives_completed >= total_objectives
            
            # Upsert progress record
            progress_query = """
                INSERT INTO quest_progress
                (quest_id, player_id, objectives_completed, progress_percent, is_completed, updated_at)
                VALUES ($1, $2, $3, $4, $5, NOW())
                ON CONFLICT (quest_id, player_id)
                DO UPDATE SET
                    objectives_completed = $3,
                    progress_percent = $4,
                    is_completed = $5,
                    updated_at = NOW()
                RETURNING objectives_completed, progress_percent, is_completed
            """
            
            result = await postgres_db.execute(
                progress_query,
                quest_id,
                player_id,
                objectives_completed,
                progress_percent,
                is_completed
            )
            
            # Update story participation if quest completed
            if is_completed:
                await self._update_story_participation(quest_id, player_id)
            
            progress = StoryQuestProgress(
                quest_id=quest_id,
                player_id=player_id,
                objectives_completed=objectives_completed,
                total_objectives=total_objectives,
                progress_percent=progress_percent,
                is_completed=is_completed
            )
            
            logger.debug(
                f"Quest #{quest_id} progress for player {player_id}: "
                f"{objectives_completed}/{total_objectives} ({progress_percent:.1f}%)"
            )
            
            return progress
            
        except Exception as e:
            logger.error(f"Failed to track quest progress: {e}")
            raise
    
    async def complete_story_quest(
        self,
        quest_id: int,
        player_id: int
    ) -> bool:
        """
        Mark story quest as completed and award rewards.
        
        Args:
            quest_id: Quest ID
            player_id: Player ID
            
        Returns:
            Success status
        """
        try:
            logger.info(f"Completing story quest #{quest_id} for player {player_id}")
            
            # Get quest data
            quest = await self._get_quest_by_id(quest_id)
            if not quest:
                raise ValueError(f"Quest #{quest_id} not found")
            
            quest_data = json.loads(quest['quest_data']) if isinstance(quest['quest_data'], str) else quest['quest_data']
            
            # Mark as completed
            await postgres_db.execute(
                """
                UPDATE quest_progress
                SET is_completed = TRUE,
                    completed_at = NOW()
                WHERE quest_id = $1 AND player_id = $2
                """,
                quest_id,
                player_id
            )
            
            # Award rewards (queue for Bridge Layer)
            await self._queue_quest_rewards(player_id, quest_data.get('rewards', {}))
            
            # Update story participation
            await self._update_story_participation(quest_id, player_id, completed=True)
            
            logger.info(f"✓ Quest #{quest_id} completed by player {player_id}")
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to complete story quest: {e}")
            return False
    
    async def get_player_story_quests(
        self,
        player_id: int,
        arc_id: Optional[int] = None
    ) -> List[Dict[str, Any]]:
        """
        Get all story quests for a player.
        
        Args:
            player_id: Player ID
            arc_id: Optional arc ID filter
            
        Returns:
            List of quest data with progress
        """
        try:
            if arc_id:
                query = """
                    SELECT sq.quest_id, sq.arc_id, sq.chapter_number, sq.quest_data,
                           qp.objectives_completed, qp.progress_percent, qp.is_completed
                    FROM story_quests sq
                    LEFT JOIN quest_progress qp 
                        ON sq.quest_id = qp.quest_id AND qp.player_id = $1
                    WHERE sq.arc_id = $2 AND sq.status = 'active'
                    ORDER BY sq.chapter_number, sq.quest_id
                """
                rows = await postgres_db.fetch_all(query, player_id, arc_id)
            else:
                query = """
                    SELECT sq.quest_id, sq.arc_id, sq.chapter_number, sq.quest_data,
                           qp.objectives_completed, qp.progress_percent, qp.is_completed
                    FROM story_quests sq
                    LEFT JOIN quest_progress qp 
                        ON sq.quest_id = qp.quest_id AND qp.player_id = $1
                    WHERE sq.status = 'active'
                    ORDER BY sq.created_at DESC
                    LIMIT 20
                """
                rows = await postgres_db.fetch_all(query, player_id)
            
            quests = []
            for row in rows:
                quest_data = json.loads(row['quest_data']) if isinstance(row['quest_data'], str) else row['quest_data']
                
                quests.append({
                    'quest_id': row['quest_id'],
                    'arc_id': row['arc_id'],
                    'chapter': row['chapter_number'],
                    'title': quest_data.get('quest_name'),
                    'description': quest_data.get('quest_desc'),
                    'objectives_completed': row['objectives_completed'] or 0,
                    'progress_percent': row['progress_percent'] or 0.0,
                    'is_completed': row['is_completed'] or False
                })
            
            return quests
            
        except Exception as e:
            logger.error(f"Failed to get player story quests: {e}")
            return []
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _allocate_quest_id(self) -> int:
        """Allocate next available quest ID in story range"""
        try:
            # Get max quest ID from database
            query = """
                SELECT COALESCE(MAX(quest_id), $1) as max_id
                FROM story_quests
                WHERE quest_id BETWEEN $1 AND $2
            """
            
            result = await postgres_db.fetch_one(
                query,
                self.quest_id_range[0],
                self.quest_id_range[1]
            )
            
            max_id = result['max_id'] if result else self.quest_id_range[0]
            
            # Allocate next ID
            next_id = max_id + 1 if max_id >= self.quest_id_range[0] else self.quest_id_range[0]
            
            # Check if range exhausted
            if next_id > self.quest_id_range[1]:
                logger.error("Story quest ID range exhausted!")
                # Wrap around or expand range
                next_id = self.quest_id_range[0]
            
            return next_id
            
        except Exception as e:
            logger.error(f"Failed to allocate quest ID: {e}")
            # Return fallback ID
            return self.quest_id_range[0]
    
    async def _parse_objectives(self, quest_spec: StoryQuest) -> List[Dict[str, Any]]:
        """
        Parse quest objectives into rAthena format.
        
        Objective Types:
        - Kill monsters: monster_id, quantity
        - Collect items: item_id, quantity
        - Talk to NPC: npc_id
        - Explore map: map_name
        
        Args:
            quest_spec: Quest specification
            
        Returns:
            List of objective dicts
        """
        objectives = []
        
        try:
            # Parse from objectives list if available
            if quest_spec.objectives:
                for obj in quest_spec.objectives:
                    objective = {
                        'type': obj.objective_type,
                        'description': obj.description,
                        'quantity': obj.quantity
                    }
                    
                    if obj.monsters:
                        objective['monsters'] = await self._resolve_monster_ids(obj.monsters)
                    
                    if obj.required_items:
                        objective['items'] = await self._resolve_item_ids(obj.required_items)
                    
                    if obj.target_npc:
                        objective['target_npc'] = obj.target_npc
                    
                    if obj.target_map:
                        objective['target_map'] = obj.target_map
                    
                    objectives.append(objective)
            
            # Fallback to legacy format
            if not objectives:
                # Kill objectives
                if quest_spec.monsters:
                    monster_ids = await self._resolve_monster_ids(quest_spec.monsters)
                    objectives.append({
                        'type': 'kill',
                        'description': f"Defeat {len(monster_ids)} types of monsters",
                        'monsters': monster_ids,
                        'quantity': 10  # Default
                    })
                
                # Collect objectives
                if quest_spec.required_items:
                    item_ids = await self._resolve_item_ids(quest_spec.required_items)
                    objectives.append({
                        'type': 'collect',
                        'description': f"Gather {len(item_ids)} types of items",
                        'items': item_ids,
                        'quantity': sum(i.get('quantity', 1) for i in quest_spec.required_items)
                    })
            
            # Default objective if none parsed
            if not objectives:
                objectives.append({
                    'type': 'generic',
                    'description': quest_spec.objective,
                    'quantity': 1
                })
            
            return objectives
            
        except Exception as e:
            logger.error(f"Failed to parse objectives: {e}")
            return [{
                'type': 'generic',
                'description': quest_spec.objective,
                'quantity': 1
            }]
    
    async def _validate_rewards(self, reward_spec) -> Dict[str, Any]:
        """
        Validate and format quest rewards.
        
        Args:
            reward_spec: Reward specification from quest
            
        Returns:
            Validated reward structure
        """
        try:
            rewards = {
                'exp': max(0, reward_spec.exp),
                'items': [],
                'reputation': reward_spec.reputation if hasattr(reward_spec, 'reputation') else 0,
                'special': reward_spec.special if hasattr(reward_spec, 'special') else None
            }
            
            # Validate item rewards
            if reward_spec.items:
                for item in reward_spec.items:
                    # Ensure item has required fields
                    item_id = item.get('item_id', 0)
                    quantity = item.get('quantity', 1)
                    
                    if item_id > 0:
                        rewards['items'].append({
                            'item_id': item_id,
                            'quantity': max(1, quantity)
                        })
            
            return rewards
            
        except Exception as e:
            logger.error(f"Failed to validate rewards: {e}")
            return {
                'exp': 10000,
                'items': [],
                'reputation': 0
            }
    
    async def _resolve_monster_ids(self, monster_names: List[str]) -> List[int]:
        """
        Resolve monster names/IDs to valid monster IDs.
        
        Args:
            monster_names: List of monster names or IDs
            
        Returns:
            List of valid monster IDs
        """
        try:
            monster_ids = []
            
            for monster in monster_names:
                # Check if already an ID
                if isinstance(monster, int):
                    monster_ids.append(monster)
                    continue
                
                # Try to parse as int
                try:
                    monster_id = int(monster)
                    monster_ids.append(monster_id)
                    continue
                except ValueError:
                    pass
                
                # TODO: Query monster database for name lookup
                # For now, use default monster IDs
                default_monsters = {
                    'poring': 1002,
                    'drops': 1113,
                    'lunatic': 1063,
                    'fabre': 1007,
                    'zombie': 1015,
                    'skeleton': 1076,
                    'orc': 1023
                }
                
                monster_lower = monster.lower()
                for name, mob_id in default_monsters.items():
                    if name in monster_lower:
                        monster_ids.append(mob_id)
                        break
            
            # Return unique IDs
            return list(set(monster_ids)) if monster_ids else [1002]  # Default to Poring
            
        except Exception as e:
            logger.error(f"Failed to resolve monster IDs: {e}")
            return [1002]  # Default to Poring
    
    async def _resolve_item_ids(self, item_specs: List[Dict[str, Any]]) -> List[Dict[str, int]]:
        """
        Resolve item names to valid item IDs.
        
        Args:
            item_specs: List of item specifications
            
        Returns:
            List of item dicts with validated IDs
        """
        try:
            items = []
            
            for item_spec in item_specs:
                item_id = item_spec.get('item_id', 0)
                quantity = item_spec.get('quantity', 1)
                
                # Validate item ID is in reasonable range
                if 500 <= item_id <= 20000:
                    items.append({
                        'item_id': item_id,
                        'quantity': max(1, quantity)
                    })
                else:
                    logger.warning(f"Invalid item ID: {item_id}")
            
            return items if items else [{'item_id': 501, 'quantity': 1}]  # Default to Red Potion
            
        except Exception as e:
            logger.error(f"Failed to resolve item IDs: {e}")
            return [{'item_id': 501, 'quantity': 1}]
    
    async def _register_with_quest_agent(
        self,
        quest_id: int,
        quest_data: Dict[str, Any]
    ):
        """Register quest with QuestAgent for tracking"""
        try:
            # Store quest in cache for QuestAgent
            await db.set(
                f"quest:story:{quest_id}",
                quest_data,
                expire=86400 * 30  # 30 days
            )
            
            # Add to active quests index
            await db.client.sadd("quests:story:active", quest_id)
            
            logger.debug(f"Registered quest #{quest_id} with QuestAgent")
            
        except Exception as e:
            logger.warning(f"Failed to register with QuestAgent: {e}")
    
    async def _queue_quest_rewards(
        self,
        player_id: int,
        rewards: Dict[str, Any]
    ):
        """Queue quest reward distribution via Bridge Layer"""
        try:
            reward_command = {
                'command': 'give_rewards',
                'player_id': player_id,
                'rewards': rewards,
                'reason': 'story_quest_completion',
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Queue for Bridge Layer
            await db.set(
                f"bridge:rewards:{player_id}:{int(datetime.now(UTC).timestamp())}",
                reward_command,
                expire=300  # 5 minutes
            )
            
            logger.debug(f"Queued rewards for player {player_id}")
            
        except Exception as e:
            logger.warning(f"Failed to queue rewards: {e}")
    
    async def _update_story_participation(
        self,
        quest_id: int,
        player_id: int,
        completed: bool = False
    ):
        """Update player's story participation record"""
        try:
            # Get arc ID from quest
            quest = await self._get_quest_by_id(quest_id)
            if not quest:
                return
            
            arc_id = quest['arc_id']
            
            # Calculate contribution score (100 per quest)
            contribution = 100 if completed else 10
            
            # Update participation
            query = """
                INSERT INTO story_participation
                (arc_id, player_id, quests_completed, contribution_score, last_participation)
                VALUES ($1, $2, $3, $4, NOW())
                ON CONFLICT (arc_id, player_id)
                DO UPDATE SET
                    quests_completed = story_participation.quests_completed + $3,
                    contribution_score = story_participation.contribution_score + $4,
                    last_participation = NOW()
            """
            
            await postgres_db.execute(
                query,
                arc_id,
                player_id,
                1 if completed else 0,
                contribution
            )
            
            logger.debug(
                f"Updated story participation for player {player_id} "
                f"(+{contribution} contribution)"
            )
            
        except Exception as e:
            logger.warning(f"Failed to update story participation: {e}")
    
    async def _get_quest_by_id(self, quest_id: int) -> Optional[Dict[str, Any]]:
        """Get quest from database"""
        try:
            query = """
                SELECT * FROM story_quests WHERE quest_id = $1
            """
            return await postgres_db.fetch_one(query, quest_id)
        except Exception as e:
            logger.error(f"Failed to get quest #{quest_id}: {e}")
            return None


# Global instance
story_quest_bridge = StoryQuestBridge()