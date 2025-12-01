"""
World State Collector for AI-Driven Storyline Generator
Aggregates data from all 21 agents + database for comprehensive world snapshot
"""

import asyncio
import json
import hashlib
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from pathlib import Path
from loguru import logger

from database import postgres_db, db
from config import settings
from models.storyline import WorldStateSnapshot


class WorldStateCollector:
    """
    Collects comprehensive world state for storyline generation.
    
    Data Sources:
    - All 15 procedural agents (Problem, NPC, Event, Faction, Reputation, etc)
    - 6 original agents (Dialogue, Decision, Memory, World, Quest, Economy)
    - PostgreSQL database (via SQL queries)
    - DragonflyDB cache (real-time metrics)
    - Bridge Layer metrics (if available)
    
    Collection Frequency:
    - Real-time: Player actions, events (via cache)
    - Hourly: Map activity, economy snapshots
    - Daily: Complete world state for story generation
    """
    
    def __init__(self):
        """Initialize World State Collector"""
        self.cache_ttl = 3600  # 1 hour cache
        self.collection_history = []
        logger.info("World State Collector initialized")
    
    async def collect_complete_world_state(
        self,
        force_fresh: bool = False
    ) -> WorldStateSnapshot:
        """
        Collect all data needed for storyline generation.
        
        Aggregates from:
        - Map activity (WorldAgent + database queries)
        - Economy metrics (EconomyAgent + MerchantEconomyAgent)
        - Faction state (FactionAgent)
        - Karma state (KarmaAgent)
        - Active problems (ProblemAgent)
        - Player metrics (database)
        - MVP data (database)
        - Previous story context (database)
        
        Args:
            force_fresh: Skip cache and collect fresh data
            
        Returns:
            Complete WorldStateSnapshot
        """
        try:
            logger.info("Collecting complete world state...")
            start_time = datetime.now(UTC)
            
            # Check cache first (unless forced)
            if not force_fresh:
                cached_state = await self._get_cached_world_state()
                if cached_state:
                    logger.info("Using cached world state")
                    return WorldStateSnapshot(**cached_state)
            
            # Collect all data in parallel for efficiency
            results = await asyncio.gather(
                self._collect_map_activity(),
                self._collect_player_metrics(),
                self._collect_mvp_data(),
                self._collect_economy_data(),
                self._collect_faction_data(),
                self._collect_karma_data(),
                self._collect_problem_data(),
                self._collect_story_context(),
                return_exceptions=True
            )
            
            # Unpack results with error handling
            (
                map_activity,
                player_metrics,
                mvp_data,
                economy_data,
                faction_data,
                karma_data,
                problem_data,
                story_context
            ) = [r if not isinstance(r, Exception) else {} for r in results]
            
            # Build comprehensive snapshot
            world_state = WorldStateSnapshot(
                # Map Activity
                most_visited_maps=map_activity.get('most_visited', []),
                least_visited_maps=map_activity.get('least_visited', []),
                map_kill_counts=map_activity.get('kill_counts', {}),
                
                # Player Metrics
                online_player_count=player_metrics.get('online_count', 0),
                player_average_level=player_metrics.get('avg_level', 50),
                player_level_distribution=player_metrics.get('level_distribution', {}),
                
                # MVP Data
                mvp_kill_frequency=mvp_data.get('kill_frequency', {}),
                mvp_respawn_times=mvp_data.get('respawn_times', {}),
                
                # Economy
                zeny_circulation=economy_data.get('zeny_circulation', 0),
                inflation_rate=economy_data.get('inflation_rate', 0.0),
                top_items_in_circulation=economy_data.get('top_items', []),
                
                # Faction State
                dominant_faction=faction_data.get('dominant_faction'),
                faction_scores=faction_data.get('faction_scores', {}),
                faction_conflicts=faction_data.get('recent_conflicts', []),
                
                # Karma & Problems
                global_karma=karma_data.get('karma_score', 0),
                active_problems=problem_data.get('active_problems', []),
                problem_outcomes=problem_data.get('recent_outcomes', []),
                
                # Storyline Context
                previous_arcs=story_context.get('previous_arcs', []),
                current_arc_chapter=story_context.get('current_chapter', 0),
                player_choices_made=story_context.get('player_choices', []),
                
                # Random Seed
                timestamp=int(datetime.now(UTC).timestamp()),
                random_seed=self._generate_random_seed()
            )
            
            # Cache for future requests
            await self._cache_world_state(world_state.model_dump())
            
            # Record collection
            collection_time = (datetime.now(UTC) - start_time).total_seconds()
            self.collection_history.append({
                'timestamp': start_time.isoformat(),
                'collection_time_seconds': collection_time,
                'data_points_collected': self._count_data_points(world_state)
            })
            
            logger.info(f"✓ World state collected in {collection_time:.2f}s")
            
            return world_state
            
        except Exception as e:
            logger.error(f"Failed to collect world state: {e}", exc_info=True)
            # Return minimal valid state
            return WorldStateSnapshot(
                random_seed=self._generate_random_seed(),
                timestamp=int(datetime.now(UTC).timestamp())
            )
    
    async def export_to_json(
        self,
        world_state: WorldStateSnapshot,
        filepath: str
    ) -> bool:
        """
        Export world state to JSON file for LLM consumption or debugging.
        
        Args:
            world_state: WorldStateSnapshot to export
            filepath: Destination file path
            
        Returns:
            Success status
        """
        try:
            output_path = Path(filepath)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            # Convert to dict and format nicely
            state_dict = world_state.model_dump()
            
            # Add metadata
            state_dict['_metadata'] = {
                'generated_at': datetime.now(UTC).isoformat(),
                'version': '1.0',
                'generator': 'WorldStateCollector'
            }
            
            # Write JSON
            with open(output_path, 'w', encoding='utf-8') as f:
                json.dump(state_dict, f, indent=2, ensure_ascii=False)
            
            logger.info(f"✓ Exported world state to {filepath}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to export world state: {e}")
            return False
    
    # ========================================================================
    # PRIVATE COLLECTION METHODS (One per data source)
    # ========================================================================
    
    async def _collect_map_activity(self) -> Dict[str, Any]:
        """Collect map activity from WorldAgent + database"""
        try:
            # Query most/least visited maps (last 24 hours)
            query = """
                SELECT map_name, COUNT(*) as visit_count
                FROM player_location_history
                WHERE timestamp > NOW() - INTERVAL '24 hours'
                GROUP BY map_name
                ORDER BY visit_count DESC
            """
            
            rows = await postgres_db.fetch_all(query)
            
            if rows:
                most_visited = [row['map_name'] for row in rows[:10]]
                least_visited = [row['map_name'] for row in rows[-10:]]
            else:
                most_visited = ['prontera', 'geffen', 'payon']
                least_visited = ['abyss_03', 'gl_church']
            
            # Get kill counts per map
            kill_query = """
                SELECT map_name, SUM(kill_count) as total_kills
                FROM map_monster_kills
                WHERE date > CURRENT_DATE - INTERVAL '7 days'
                GROUP BY map_name
                ORDER BY total_kills DESC
            """
            
            kill_rows = await postgres_db.fetch_all(kill_query)
            kill_counts = {row['map_name']: row['total_kills'] for row in kill_rows}
            
            return {
                'most_visited': most_visited,
                'least_visited': least_visited,
                'kill_counts': kill_counts
            }
            
        except Exception as e:
            logger.warning(f"Map activity collection failed: {e}")
            return {
                'most_visited': ['prontera'],
                'least_visited': [],
                'kill_counts': {}
            }
    
    async def _collect_player_metrics(self) -> Dict[str, Any]:
        """Collect player population and level metrics"""
        try:
            # Try cache first (real-time data)
            cached_count = await db.get("metrics:online_players")
            online_count = int(cached_count) if cached_count else 0
            
            # Get average level from database
            query = """
                SELECT 
                    COUNT(*) as total_players,
                    AVG(base_level) as avg_level
                FROM characters
                WHERE last_login > NOW() - INTERVAL '7 days'
            """
            
            result = await postgres_db.fetch_one(query)
            
            avg_level = int(result['avg_level']) if result and result['avg_level'] else 50
            
            # Get level distribution
            dist_query = """
                SELECT 
                    FLOOR(base_level / 10) * 10 as level_bracket,
                    COUNT(*) as player_count
                FROM characters
                WHERE last_login > NOW() - INTERVAL '7 days'
                GROUP BY level_bracket
                ORDER BY level_bracket
            """
            
            dist_rows = await postgres_db.fetch_all(dist_query)
            level_distribution = {
                int(row['level_bracket']): row['player_count']
                for row in dist_rows
            }
            
            return {
                'online_count': online_count,
                'avg_level': avg_level,
                'level_distribution': level_distribution,
                'total_active_players': result['total_players'] if result else 0
            }
            
        except Exception as e:
            logger.warning(f"Player metrics collection failed: {e}")
            return {
                'online_count': 0,
                'avg_level': 50,
                'level_distribution': {},
                'total_active_players': 0
            }
    
    async def _collect_mvp_data(self) -> Dict[str, Any]:
        """Collect MVP kill frequency and respawn patterns"""
        try:
            # MVP kill frequency (last 7 days)
            query = """
                SELECT 
                    mob_id,
                    COUNT(*) as kill_count,
                    AVG(EXTRACT(EPOCH FROM respawn_time)) / 3600 as avg_respawn_hours
                FROM mvp_kills
                WHERE killed_at > NOW() - INTERVAL '7 days'
                GROUP BY mob_id
                ORDER BY kill_count DESC
            """
            
            rows = await postgres_db.fetch_all(query)
            
            kill_frequency = {
                str(row['mob_id']): row['kill_count']
                for row in rows
            }
            
            respawn_times = {
                str(row['mob_id']): round(row['avg_respawn_hours'], 2)
                for row in rows
                if row['avg_respawn_hours']
            }
            
            return {
                'kill_frequency': kill_frequency,
                'respawn_times': respawn_times
            }
            
        except Exception as e:
            logger.warning(f"MVP data collection failed: {e}")
            return {
                'kill_frequency': {},
                'respawn_times': {}
            }
    
    async def _collect_economy_data(self) -> Dict[str, Any]:
        """Collect economy metrics from EconomyAgent + MerchantEconomyAgent"""
        try:
            # Try cache first
            cached_economy = await db.get("economy:metrics:current")
            if cached_economy:
                return cached_economy
            
            # Calculate zeny circulation
            zeny_query = """
                SELECT SUM(zeny) as total_zeny
                FROM characters
                WHERE last_login > NOW() - INTERVAL '30 days'
            """
            
            zeny_result = await postgres_db.fetch_one(zeny_query)
            total_zeny = zeny_result['total_zeny'] if zeny_result and zeny_result['total_zeny'] else 0
            
            # Get inflation rate (from economic_snapshots table if exists)
            inflation_query = """
                SELECT inflation_rate
                FROM economic_snapshots
                ORDER BY snapshot_time DESC
                LIMIT 1
            """
            
            inflation_result = await postgres_db.fetch_one(inflation_query)
            inflation_rate = inflation_result['inflation_rate'] if inflation_result else 0.0
            
            # Get top traded items (from vending or trade logs)
            items_query = """
                SELECT item_id, item_name, COUNT(*) as trade_count
                FROM trade_log
                WHERE trade_time > NOW() - INTERVAL '24 hours'
                GROUP BY item_id, item_name
                ORDER BY trade_count DESC
                LIMIT 10
            """
            
            items_rows = await postgres_db.fetch_all(items_query)
            top_items = [row['item_name'] for row in items_rows]
            
            economy_data = {
                'zeny_circulation': int(total_zeny),
                'inflation_rate': float(inflation_rate),
                'top_items': top_items
            }
            
            # Cache for 1 hour
            await db.set("economy:metrics:current", economy_data, expire=3600)
            
            return economy_data
            
        except Exception as e:
            logger.warning(f"Economy data collection failed: {e}")
            return {
                'zeny_circulation': 0,
                'inflation_rate': 0.0,
                'top_items': []
            }
    
    async def _collect_faction_data(self) -> Dict[str, Any]:
        """Collect faction alignment from FactionAgent"""
        try:
            # Import and use faction agent
            from agents.progression.faction_agent import faction_agent
            
            alignment_data = await faction_agent.get_world_alignment()
            
            # Extract dominant faction
            dominant_faction = alignment_data.get('dominant_faction')
            
            # Build faction scores
            faction_scores = {}
            for faction in alignment_data.get('factions', []):
                faction_scores[faction['faction_id']] = faction['alignment_score']
            
            # Get recent conflicts
            conflicts_query = """
                SELECT faction_id, action_type, alignment_change, created_at
                FROM faction_actions
                WHERE created_at > NOW() - INTERVAL '7 days'
                  AND ABS(alignment_change) > 100
                ORDER BY created_at DESC
                LIMIT 20
            """
            
            conflict_rows = await postgres_db.fetch_all(conflicts_query)
            recent_conflicts = [
                {
                    'faction_id': row['faction_id'],
                    'action': row['action_type'],
                    'impact': row['alignment_change'],
                    'timestamp': row['created_at'].isoformat()
                }
                for row in conflict_rows
            ]
            
            return {
                'dominant_faction': dominant_faction,
                'faction_scores': faction_scores,
                'recent_conflicts': recent_conflicts
            }
            
        except Exception as e:
            logger.warning(f"Faction data collection failed: {e}")
            return {
                'dominant_faction': None,
                'faction_scores': {},
                'recent_conflicts': []
            }
    
    async def _collect_karma_data(self) -> Dict[str, Any]:
        """Collect global karma from KarmaAgent"""
        try:
            # Import karma agent
            from agents.economy_social.karma_agent import karma_agent
            
            karma_state = await karma_agent.get_global_karma_state()
            
            return {
                'karma_score': karma_state.get('karma_score', 0),
                'alignment': karma_state.get('alignment', 'neutral'),
                'good_actions': karma_state.get('good_actions_count', 0),
                'evil_actions': karma_state.get('evil_actions_count', 0)
            }
            
        except Exception as e:
            logger.warning(f"Karma data collection failed: {e}")
            return {
                'karma_score': 0,
                'alignment': 'neutral',
                'good_actions': 0,
                'evil_actions': 0
            }
    
    async def _collect_problem_data(self) -> Dict[str, Any]:
        """Collect active problems from ProblemAgent"""
        try:
            # Import problem agent
            from agents.procedural.problem_agent import problem_agent
            
            # Get current active problem
            active_problem = await problem_agent.get_current_problem()
            
            active_problems = []
            if active_problem:
                active_problems.append({
                    'problem_id': active_problem.get('problem_id'),
                    'problem_type': active_problem.get('problem_type'),
                    'title': active_problem.get('title'),
                    'difficulty': active_problem.get('difficulty'),
                    'participation': active_problem.get('participation_count', 0)
                })
            
            # Get recent problem outcomes
            outcomes_query = """
                SELECT problem_type, status, participation_count, completed_at
                FROM world_problems
                WHERE completed_at > NOW() - INTERVAL '7 days'
                ORDER BY completed_at DESC
                LIMIT 10
            """
            
            outcome_rows = await postgres_db.fetch_all(outcomes_query)
            recent_outcomes = [
                {
                    'problem_type': row['problem_type'],
                    'status': row['status'],
                    'participation': row['participation_count'],
                    'completed_at': row['completed_at'].isoformat()
                }
                for row in outcome_rows
            ]
            
            return {
                'active_problems': active_problems,
                'recent_outcomes': recent_outcomes
            }
            
        except Exception as e:
            logger.warning(f"Problem data collection failed: {e}")
            return {
                'active_problems': [],
                'recent_outcomes': []
            }
    
    async def _collect_story_context(self) -> Dict[str, Any]:
        """Collect previous story arcs and current arc context"""
        try:
            # Get previous arcs
            arcs_query = """
                SELECT 
                    arc_id,
                    arc_name,
                    arc_summary,
                    total_chapters,
                    theme,
                    completed_at
                FROM story_arc_history
                ORDER BY completed_at DESC
                LIMIT 5
            """
            
            arc_rows = await postgres_db.fetch_all(arcs_query)
            
            previous_arcs = [
                {
                    'arc_id': row['arc_id'],
                    'arc_name': row['arc_name'],
                    'arc_summary': row['arc_summary'],
                    'total_chapters': row['total_chapters'],
                    'theme': row['theme'],
                    'completed_at': row['completed_at'].isoformat()
                }
                for row in arc_rows
            ]
            
            # Get current arc if exists
            current_query = """
                SELECT arc_id, current_chapter
                FROM story_arcs
                WHERE status = 'active'
                LIMIT 1
            """
            
            current_result = await postgres_db.fetch_one(current_query)
            current_chapter = current_result['current_chapter'] if current_result else 0
            
            # Get recent player choices
            choices_query = """
                SELECT player_id, choices_made
                FROM story_participation
                WHERE last_participation > NOW() - INTERVAL '7 days'
                  AND choices_made IS NOT NULL
                ORDER BY last_participation DESC
                LIMIT 50
            """
            
            choice_rows = await postgres_db.fetch_all(choices_query)
            
            player_choices = []
            for row in choice_rows:
                if row['choices_made']:
                    try:
                        choices = json.loads(row['choices_made']) if isinstance(row['choices_made'], str) else row['choices_made']
                        if isinstance(choices, list):
                            player_choices.extend(choices)
                        elif isinstance(choices, dict):
                            player_choices.append(choices)
                    except:
                        continue
            
            return {
                'previous_arcs': previous_arcs,
                'current_chapter': current_chapter,
                'player_choices': player_choices[-50:]  # Last 50 choices
            }
            
        except Exception as e:
            logger.warning(f"Story context collection failed: {e}")
            return {
                'previous_arcs': [],
                'current_chapter': 0,
                'player_choices': []
            }
    
    # ========================================================================
    # CACHE MANAGEMENT
    # ========================================================================
    
    async def _get_cached_world_state(self) -> Optional[Dict[str, Any]]:
        """Get cached world state if fresh enough"""
        try:
            cached = await db.get("storyline:world_state:latest")
            return cached
        except Exception as e:
            logger.debug(f"Cache miss: {e}")
            return None
    
    async def _cache_world_state(self, world_state_dict: Dict[str, Any]):
        """Cache world state for reuse"""
        try:
            await db.set(
                "storyline:world_state:latest",
                world_state_dict,
                expire=self.cache_ttl
            )
            logger.debug("World state cached")
        except Exception as e:
            logger.warning(f"Failed to cache world state: {e}")
    
    def _generate_random_seed(self) -> str:
        """Generate deterministic random seed from timestamp"""
        timestamp = datetime.now(UTC).isoformat()
        return hashlib.md5(timestamp.encode()).hexdigest()
    
    def _count_data_points(self, world_state: WorldStateSnapshot) -> int:
        """Count total data points collected"""
        count = 0
        count += len(world_state.most_visited_maps)
        count += len(world_state.least_visited_maps)
        count += len(world_state.map_kill_counts)
        count += len(world_state.player_level_distribution)
        count += len(world_state.mvp_kill_frequency)
        count += len(world_state.faction_scores)
        count += len(world_state.active_problems)
        count += len(world_state.previous_arcs)
        return count


# Global instance
world_state_collector = WorldStateCollector()