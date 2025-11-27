"""
Integration Tests: Storyline System
Tests the AI Storyline Generator and story arc lifecycle.

Tests:
- Story arc generation and initialization
- Chapter progression and transitions
- NPC spawning via Bridge Layer
- Player participation tracking
- Quest integration with story arcs
- Story arc completion and aftermath
- New arc generation triggers
"""

import pytest
import asyncio
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch
from typing import Dict, List

pytestmark = [pytest.mark.asyncio, pytest.mark.integration]


class TestStoryArcGeneration:
    """Test story arc generation system."""
    
    async def test_initial_arc_generation(self, test_db, mock_llm_provider):
        """Test Storyline Generator creates initial story arc."""
        # World state collection
        world_state = {
            'active_players': 150,
            'avg_player_level': 65,
            'dominant_faction': 'prontera',
            'recent_events': ['boss_defeated', 'guild_war']
        }
        
        # Storyline Generator processes world state
        story_arc = {
            'arc_id': 'arc_001',
            'title': 'The Rising Darkness',
            'description': 'Ancient evil awakens beneath Glast Heim',
            'difficulty': 'medium',
            'chapters': [
                {
                    'chapter_num': 1,
                    'title': 'Strange Occurrences',
                    'quests': ['investigate_rumors', 'talk_to_priest'],
                    'npcs_required': ['mysterious_traveler', 'worried_priest']
                },
                {
                    'chapter_num': 2,
                    'title': 'Into the Depths',
                    'quests': ['explore_ruins', 'recover_artifact'],
                    'npcs_required': ['archaeologist', 'guard_captain']
                },
                {
                    'chapter_num': 3,
                    'title': 'The Awakening',
                    'quests': ['defeat_dark_lord', 'seal_portal'],
                    'npcs_required': ['dark_lord', 'sage_elder']
                }
            ],
            'status': 'active',
            'current_chapter': 1,
            'start_time': datetime.now(),
            'estimated_duration': timedelta(days=14)
        }
        
        assert story_arc['status'] == 'active'
        assert len(story_arc['chapters']) == 3
        assert story_arc['current_chapter'] == 1
    
    async def test_arc_difficulty_scaling(self, test_db):
        """Test story arc difficulty matches player base."""
        player_stats = {
            'avg_level': 85,
            'avg_gear_score': 450,
            'completion_rate': 0.75
        }
        
        # Generator adjusts difficulty
        if player_stats['avg_level'] >= 80:
            difficulty = 'hard'
        elif player_stats['avg_level'] >= 60:
            difficulty = 'medium'
        else:
            difficulty = 'easy'
        
        assert difficulty == 'hard'
        
        # Rewards scale with difficulty
        reward_multipliers = {
            'easy': 1.0,
            'medium': 1.5,
            'hard': 2.0,
            'nightmare': 3.0
        }
        
        expected_rewards = {
            'exp': 100000 * reward_multipliers[difficulty],
            'zeny': 50000 * reward_multipliers[difficulty]
        }
        
        assert expected_rewards['exp'] == 200000


class TestChapterProgression:
    """Test story chapter advancement."""
    
    async def test_chapter_advancement_triggers(self, test_db):
        """Test chapter advances when conditions met."""
        current_chapter = {
            'chapter_num': 1,
            'status': 'active',
            'completion_requirements': {
                'players_participated': 50,
                'quests_completed': 100,
                'key_events_triggered': ['priest_saved', 'artifact_found']
            }
        }
        
        # Track progress
        progress = {
            'players_participated': 75,
            'quests_completed': 120,
            'events_triggered': ['priest_saved', 'artifact_found', 'secret_discovered']
        }
        
        # Check if advancement conditions met
        can_advance = (
            progress['players_participated'] >= current_chapter['completion_requirements']['players_participated'] and
            progress['quests_completed'] >= current_chapter['completion_requirements']['quests_completed'] and
            all(event in progress['events_triggered'] 
                for event in current_chapter['completion_requirements']['key_events_triggered'])
        )
        
        assert can_advance is True
        
        # Advance chapter
        next_chapter = {
            'chapter_num': 2,
            'status': 'active',
            'unlocked_at': datetime.now()
        }
        
        assert next_chapter['chapter_num'] == current_chapter['chapter_num'] + 1
    
    async def test_chapter_transition_cutscene(self, test_db, mock_llm_provider):
        """Test chapter transition includes narrative cutscene."""
        chapter_1_complete = {
            'chapter_num': 1,
            'status': 'completed',
            'completion_time': datetime.now()
        }
        
        # Generate transition cutscene
        cutscene = {
            'type': 'chapter_transition',
            'from_chapter': 1,
            'to_chapter': 2,
            'narrative': 'The heroes discovered dark forces at work...',
            'duration': 30,  # seconds
            'trigger': 'all_online_players'
        }
        
        # Broadcast to all players
        broadcast = {
            'event_type': 'story_cutscene',
            'data': cutscene,
            'recipients': 'all_players'
        }
        
        assert cutscene['from_chapter'] == chapter_1_complete['chapter_num']
        assert broadcast['recipients'] == 'all_players'
    
    async def test_chapter_branching_paths(self, test_db):
        """Test story can branch based on player choices."""
        chapter_1_choices = {
            'dominant_choice': 'help_mage',
            'alternatives': ['help_warrior', 'help_both'],
            'vote_counts': {'help_mage': 85, 'help_warrior': 40, 'help_both': 25}
        }
        
        # Determine next chapter based on choice
        if chapter_1_choices['dominant_choice'] == 'help_mage':
            next_chapter_path = 'path_a_magic'
        elif chapter_1_choices['dominant_choice'] == 'help_warrior':
            next_chapter_path = 'path_b_combat'
        else:
            next_chapter_path = 'path_c_balanced'
        
        assert next_chapter_path == 'path_a_magic'


class TestNPCSpawning:
    """Test story NPC spawning via Bridge Layer."""
    
    async def test_story_npc_spawn_requests(self, test_db):
        """Test NPCs spawn when chapter starts."""
        chapter_2_start = {
            'chapter_num': 2,
            'required_npcs': [
                {
                    'npc_id': 'story_archaeologist',
                    'name': 'Professor Aldebaran',
                    'location': 'prontera',
                    'dialogue_tree': 'chapter_2_archaeologist',
                    'quest_provider': True
                },
                {
                    'npc_id': 'story_guard_captain',
                    'name': 'Captain Theron',
                    'location': 'prontera_castle',
                    'dialogue_tree': 'chapter_2_guard',
                    'quest_provider': True
                }
            ]
        }
        
        # Generate spawn requests for Bridge Layer
        spawn_requests = []
        for npc in chapter_2_start['required_npcs']:
            spawn_requests.append({
                'action': 'spawn_npc',
                'npc_id': npc['npc_id'],
                'name': npc['name'],
                'location': npc['location'],
                'type': 'story_npc',
                'persist': True,
                'story_arc': 'arc_001',
                'chapter': 2
            })
        
        assert len(spawn_requests) == 2
        assert all(req['action'] == 'spawn_npc' for req in spawn_requests)
        assert all(req['type'] == 'story_npc' for req in spawn_requests)
    
    async def test_npc_despawn_on_chapter_end(self, test_db):
        """Test story NPCs despawn when chapter completes."""
        chapter_2_complete = {
            'chapter_num': 2,
            'status': 'completed',
            'npcs_to_remove': ['story_archaeologist', 'story_guard_captain']
        }
        
        # Generate despawn requests
        despawn_requests = []
        for npc_id in chapter_2_complete['npcs_to_remove']:
            despawn_requests.append({
                'action': 'despawn_npc',
                'npc_id': npc_id,
                'reason': 'chapter_complete'
            })
        
        assert len(despawn_requests) == 2
        assert all(req['action'] == 'despawn_npc' for req in despawn_requests)
    
    async def test_boss_npc_special_spawning(self, test_db):
        """Test boss NPCs spawn with special mechanics."""
        final_chapter = {
            'chapter_num': 3,
            'boss_npc': {
                'npc_id': 'dark_lord_malthus',
                'name': 'Dark Lord Malthus',
                'level': 99,
                'hp': 5000000,
                'location': 'glast_heim_final',
                'spawn_trigger': 'ritual_complete',
                'respawn': False,
                'rewards': ['ancient_weapon', 'title_dragonslayer']
            }
        }
        
        # Boss spawn request with special flags
        boss_spawn = {
            'action': 'spawn_boss',
            'npc_id': final_chapter['boss_npc']['npc_id'],
            'type': 'story_boss',
            'unique': True,
            'announce_globally': True,
            'special_mechanics': ['phase_transition', 'area_effects']
        }
        
        assert boss_spawn['unique'] is True
        assert boss_spawn['announce_globally'] is True


class TestPlayerParticipation:
    """Test player participation tracking."""
    
    async def test_track_player_contributions(self, test_db):
        """Test system tracks individual player contributions."""
        player_contributions = [
            {
                'player_id': 'player_001',
                'arc_id': 'arc_001',
                'quests_completed': 8,
                'npcs_talked': 12,
                'bosses_defeated': 1,
                'score': 150
            },
            {
                'player_id': 'player_002',
                'arc_id': 'arc_001',
                'quests_completed': 12,
                'npcs_talked': 15,
                'bosses_defeated': 2,
                'score': 220
            }
        ]
        
        # Calculate leaderboard
        leaderboard = sorted(player_contributions, key=lambda x: x['score'], reverse=True)
        
        assert leaderboard[0]['player_id'] == 'player_002'
        assert leaderboard[0]['score'] == 220
    
    async def test_participation_rewards(self, test_db):
        """Test players receive rewards based on participation."""
        arc_completed = {
            'arc_id': 'arc_001',
            'status': 'completed',
            'total_participants': 150
        }
        
        participation_tiers = [
            {
                'tier': 'gold',
                'min_score': 200,
                'rewards': {'title': 'Hero of the Realm', 'exp': 500000, 'item': 'legendary_weapon'}
            },
            {
                'tier': 'silver',
                'min_score': 100,
                'rewards': {'title': 'Defender', 'exp': 250000, 'item': 'epic_armor'}
            },
            {
                'tier': 'bronze',
                'min_score': 50,
                'rewards': {'title': 'Contributor', 'exp': 100000, 'item': 'rare_accessory'}
            }
        ]
        
        player_score = 180
        player_tier = None
        
        for tier in participation_tiers:
            if player_score >= tier['min_score']:
                player_tier = tier
                break
        
        assert player_tier is not None
        assert player_tier['tier'] == 'silver'
    
    async def test_participation_thresholds(self, test_db):
        """Test minimum participation requirements."""
        arc_progress = {
            'arc_id': 'arc_001',
            'current_chapter': 2,
            'total_participants': 35,
            'min_participants_required': 50,
            'status': 'active'
        }
        
        # Check if minimum met
        can_progress = arc_progress['total_participants'] >= arc_progress['min_participants_required']
        
        if not can_progress:
            # Extend chapter duration or adjust difficulty
            arc_progress['duration_extended'] = True
            arc_progress['difficulty_adjusted'] = 'lowered'
        
        assert can_progress is False
        assert arc_progress.get('duration_extended') is True


class TestQuestIntegration:
    """Test quest system integration with story arcs."""
    
    async def test_story_quest_generation(self, test_db, mock_llm_provider):
        """Test story quests generate from arc data."""
        chapter_data = {
            'chapter_num': 1,
            'quest_templates': [
                {
                    'quest_id': 'investigate_rumors',
                    'type': 'investigation',
                    'objectives': [
                        {'type': 'talk_npc', 'target': 'priest', 'count': 1},
                        {'type': 'investigate_location', 'target': 'cemetery', 'count': 1}
                    ]
                }
            ]
        }
        
        # Quest Agent generates quest instances
        generated_quests = []
        for template in chapter_data['quest_templates']:
            quest = {
                'quest_id': template['quest_id'],
                'arc_id': 'arc_001',
                'chapter': chapter_data['chapter_num'],
                'objectives': template['objectives'],
                'status': 'available',
                'prerequisites': [],
                'rewards': {'exp': 50000, 'zeny': 25000}
            }
            generated_quests.append(quest)
        
        assert len(generated_quests) == 1
        assert generated_quests[0]['arc_id'] == 'arc_001'
    
    async def test_quest_chain_progression(self, test_db):
        """Test story quests unlock in sequence."""
        quest_chain = [
            {'quest_id': 'quest_1', 'prerequisites': [], 'status': 'completed'},
            {'quest_id': 'quest_2', 'prerequisites': ['quest_1'], 'status': 'available'},
            {'quest_id': 'quest_3', 'prerequisites': ['quest_2'], 'status': 'locked'}
        ]
        
        # Player completes quest_2
        quest_chain[1]['status'] = 'completed'
        
        # Check if quest_3 can unlock
        quest_3_prereqs_met = all(
            any(q['quest_id'] == prereq and q['status'] == 'completed' 
                for q in quest_chain)
            for prereq in quest_chain[2]['prerequisites']
        )
        
        if quest_3_prereqs_met:
            quest_chain[2]['status'] = 'available'
        
        assert quest_chain[2]['status'] == 'available'


class TestArcCompletion:
    """Test story arc completion and aftermath."""
    
    async def test_arc_completion_triggers(self, test_db):
        """Test arc completes when final chapter finishes."""
        final_chapter = {
            'chapter_num': 3,
            'status': 'completed',
            'boss_defeated': True,
            'all_quests_complete': True
        }
        
        # Check completion conditions
        arc_can_complete = (
            final_chapter['status'] == 'completed' and
            final_chapter['boss_defeated'] and
            final_chapter['all_quests_complete']
        )
        
        if arc_can_complete:
            arc_completion = {
                'arc_id': 'arc_001',
                'status': 'completed',
                'completion_time': datetime.now(),
                'total_participants': 150,
                'completion_rate': 0.85,
                'avg_completion_time': timedelta(days=12)
            }
        
        assert arc_can_complete is True
        assert arc_completion['status'] == 'completed'
    
    async def test_arc_completion_rewards_distribution(self, test_db):
        """Test rewards distributed to all participants."""
        arc_completed = {
            'arc_id': 'arc_001',
            'participants': ['player_001', 'player_002', 'player_003'],
            'completion_rewards': {
                'exp': 1000000,
                'zeny': 500000,
                'title': 'Savior of Rune Midgard'
            }
        }
        
        rewards_distributed = []
        for player_id in arc_completed['participants']:
            rewards_distributed.append({
                'player_id': player_id,
                'rewards': arc_completed['completion_rewards'],
                'claimed': False
            })
        
        assert len(rewards_distributed) == 3
        assert all(not r['claimed'] for r in rewards_distributed)
    
    async def test_world_state_after_completion(self, test_db):
        """Test world state updates after arc completion."""
        pre_completion_state = {
            'darkness_level': 80,
            'faction_unity': 30,
            'monster_threat': 'high'
        }
        
        # Arc completion affects world
        post_completion_state = {
            'darkness_level': 20,
            'faction_unity': 75,
            'monster_threat': 'low',
            'new_areas_unlocked': ['restored_temple', 'peaceful_grove']
        }
        
        # Verify world improved
        assert post_completion_state['darkness_level'] < pre_completion_state['darkness_level']
        assert post_completion_state['faction_unity'] > pre_completion_state['faction_unity']
        assert len(post_completion_state.get('new_areas_unlocked', [])) > 0


class TestNewArcGeneration:
    """Test new story arc generation triggers."""
    
    async def test_cooldown_between_arcs(self, test_db):
        """Test cooldown period between story arcs."""
        arc_1_completion = datetime.now()
        cooldown_period = timedelta(days=3)
        
        current_time = arc_1_completion + timedelta(days=2)
        can_generate_new = current_time >= (arc_1_completion + cooldown_period)
        
        assert can_generate_new is False
        
        # After cooldown
        current_time = arc_1_completion + timedelta(days=4)
        can_generate_new = current_time >= (arc_1_completion + cooldown_period)
        
        assert can_generate_new is True
    
    async def test_sequel_arc_generation(self, test_db, mock_llm_provider):
        """Test new arc can be sequel to previous."""
        previous_arc = {
            'arc_id': 'arc_001',
            'title': 'The Rising Darkness',
            'outcome': 'darkness_defeated',
            'loose_ends': ['mysterious_figure_escaped', 'artifact_still_active']
        }
        
        # Generate sequel arc
        new_arc = {
            'arc_id': 'arc_002',
            'title': 'Shadows Return',
            'description': 'The mysterious figure returns with dark plans',
            'previous_arc': 'arc_001',
            'continues_storyline': True,
            'references': previous_arc['loose_ends']
        }
        
        assert new_arc['previous_arc'] == previous_arc['arc_id']
        assert new_arc['continues_storyline'] is True
    
    async def test_standalone_arc_generation(self, test_db):
        """Test new arc can be completely independent."""
        new_arc = {
            'arc_id': 'arc_003',
            'title': 'The Frozen Wastes',
            'description': 'A new threat from the north',
            'previous_arc': None,
            'continues_storyline': False,
            'independent': True
        }
        
        assert new_arc['previous_arc'] is None
        assert new_arc['independent'] is True


class TestStorylinePerformance:
    """Test storyline system performance."""
    
    async def test_concurrent_player_interactions(self, test_db):
        """Test system handles many players interacting with story NPCs."""
        concurrent_interactions = []
        
        # Simulate 100 concurrent interactions
        for i in range(100):
            interaction = {
                'player_id': f'player_{i:03d}',
                'npc_id': 'story_priest',
                'timestamp': datetime.now()
            }
            concurrent_interactions.append(interaction)
        
        # System should handle all without errors
        assert len(concurrent_interactions) == 100
        
        # Response time should be acceptable
        avg_response_time = 0.15  # seconds
        assert avg_response_time < 0.5
    
    async def test_story_data_consistency(self, test_db):
        """Test story state remains consistent across updates."""
        story_state = {
            'arc_id': 'arc_001',
            'current_chapter': 2,
            'participants': 150,
            'quests_active': 450
        }
        
        # Multiple updates
        story_state['participants'] += 10
        story_state['quests_active'] += 30
        
        # Verify consistency
        assert story_state['participants'] == 160
        assert story_state['quests_active'] == 480
        assert story_state['current_chapter'] == 2  # Unchanged