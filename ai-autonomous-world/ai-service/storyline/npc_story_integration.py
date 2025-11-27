"""
NPC Story Integration
Integrates story NPCs with existing Dialogue Agent and game systems
"""

import asyncio
import json
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from database import postgres_db, db
from config import settings
from models.storyline import (
    StoryEventNPC, StoryNPCPersonality, StoryNPCResponse,
    NPCRole, RecurringCharacterUpdate, HeroSelectionResponse
)
from storyline.prompts import (
    NPC_PERSONALITY_PROMPT_TEMPLATE,
    HERO_RECOGNITION_PROMPT_TEMPLATE
)


class NPCStoryIntegration:
    """
    Integrates story NPCs with existing Dialogue Agent.
    
    Features:
    - Story NPC personality generation
    - Context-aware dialogue
    - Quest delivery
    - Recurring character system
    - Hero of the Week recognition
    - Integration with existing DialogueAgent
    """
    
    def __init__(self):
        """Initialize NPC Story Integration"""
        self.recurring_enabled = getattr(
            settings, 'RECURRING_CHARACTERS_ENABLED', True
        )
        self.hero_recognition_enabled = getattr(
            settings, 'HERO_RECOGNITION_ENABLED', True
        )
        
        logger.info("NPC Story Integration initialized")
    
    async def create_story_npc(
        self,
        npc_spec: StoryEventNPC,
        arc_id: int,
        chapter_id: int,
        arc_context: Dict[str, Any]
    ) -> StoryNPCResponse:
        """
        Create NPC from story arc specification.
        
        Process:
        1. Generate personality from arc context
        2. Create dialogue tree
        3. Assign quest if applicable
        4. Store in database
        5. Register with DialogueAgent
        6. Queue spawn via Bridge Layer
        
        Args:
            npc_spec: NPC specification from story arc
            arc_id: Parent story arc ID
            chapter_id: Parent chapter ID
            arc_context: Full arc context for personality generation
            
        Returns:
            StoryNPCResponse with NPC ID and details
        """
        try:
            logger.info(f"Creating story NPC: {npc_spec.npc_name}")
            
            # Generate rich personality
            personality = await self._generate_npc_personality(
                npc_spec=npc_spec,
                arc_context=arc_context
            )
            
            # Build dialogue tree from spec
            dialogue_tree = await self._create_dialogue_tree(
                npc_spec=npc_spec,
                personality=personality
            )
            
            # Determine despawn time (end of chapter or arc)
            despawn_at = datetime.now(UTC) + timedelta(days=arc_context.get('chapter_duration', 3))
            
            # Store NPC in database
            npc_query = """
                INSERT INTO story_npcs
                (arc_id, chapter_id, npc_name, npc_role, npc_sprite, npc_location,
                 personality_data, dialogue_tree, quest_data, is_recurring)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
                RETURNING npc_id, created_at
            """
            
            quest_data = npc_spec.quest.model_dump() if npc_spec.quest else None
            
            result = await postgres_db.fetch_one(
                npc_query,
                arc_id,
                chapter_id,
                npc_spec.npc_name,
                npc_spec.npc_role.value,
                npc_spec.npc_sprite,
                npc_spec.npc_location,
                json.dumps(personality.model_dump()),
                json.dumps(dialogue_tree),
                json.dumps(quest_data) if quest_data else None,
                False  # Not recurring by default
            )
            
            npc_id = result['npc_id']
            created_at = result['created_at']
            
            # Register with Dialogue Agent (store in cache for pickup)
            await self._register_with_dialogue_agent(npc_id, npc_spec, dialogue_tree)
            
            # Queue spawn via Bridge Layer
            await self._queue_npc_spawn(npc_id, npc_spec)
            
            # Build response
            response = StoryNPCResponse(
                npc_id=npc_id,
                arc_id=arc_id,
                npc_name=npc_spec.npc_name,
                npc_role=npc_spec.npc_role,
                npc_sprite=npc_spec.npc_sprite,
                npc_location=npc_spec.npc_location,
                personality_data=personality,
                is_recurring=False,
                appearances_count=1,
                quest_id=None  # Will be set by StoryQuestBridge
            )
            
            logger.info(f"✓ Story NPC #{npc_id} created: {npc_spec.npc_name}")
            
            return response
            
        except Exception as e:
            logger.error(f"Failed to create story NPC: {e}", exc_info=True)
            raise
    
    async def make_recurring_character(
        self,
        npc_id: int,
        arc_id: int
    ) -> bool:
        """
        Mark NPC as recurring across arcs.
        
        Recurring NPCs:
        - Remember player interactions
        - Evolve personality over time
        - Reference past events
        - Build relationships with players
        - Appear in future arcs with continuity
        
        Args:
            npc_id: NPC ID to make recurring
            arc_id: Current arc ID
            
        Returns:
            Success status
        """
        try:
            if not self.recurring_enabled:
                logger.info("Recurring characters disabled in settings")
                return False
            
            # Update NPC record
            query = """
                UPDATE story_npcs
                SET is_recurring = TRUE,
                    appearances_count = 1
                WHERE npc_id = $1
            """
            
            await postgres_db.execute(query, npc_id)
            
            # Store in recurring registry for future arc references
            await db.set(
                f"storyline:recurring_npc:{npc_id}",
                {
                    'npc_id': npc_id,
                    'first_arc_id': arc_id,
                    'appearances': 1,
                    'marked_at': datetime.now(UTC).isoformat()
                },
                expire=86400 * 365  # 1 year
            )
            
            logger.info(f"✓ NPC #{npc_id} marked as recurring character")
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to make NPC recurring: {e}")
            return False
    
    async def generate_hero_recognition(
        self,
        arc_id: int,
        top_participants: List[Dict[str, Any]]
    ) -> HeroSelectionResponse:
        """
        Create "Hero of the Week" recognition.
        
        Features:
        - Top 3 contributors named in story
        - Special dialogue mentioning their deeds
        - Unique title rewards
        - NPC references to their actions
        - Server-wide announcement
        
        Args:
            arc_id: Story arc ID
            top_participants: Top contributors list
            
        Returns:
            HeroSelectionResponse with recognition details
        """
        try:
            if not self.hero_recognition_enabled or not top_participants:
                logger.info("Hero recognition disabled or no participants")
                return HeroSelectionResponse(
                    arc_id=arc_id,
                    heroes=[],
                    selection_criteria="No heroes selected",
                    rewards={}
                )
            
            logger.info(f"Generating hero recognition for arc #{arc_id}")
            
            # Build hero stats for LLM
            hero_stats = "\n".join([
                f"{i+1}. Player {h['player_id']}: "
                f"{h['contribution_score']} contribution, "
                f"{h['chapters_completed']} chapters, "
                f"{h['quests_completed']} quests"
                for i, h in enumerate(top_participants[:self.hero_selection_top_n])
            ])
            
            # Get arc details
            arc = await self._get_arc_details(arc_id)
            
            # TODO: Call LLM for creative hero recognition (optional)
            # For now, use template-based approach
            
            heroes = []
            for i, participant in enumerate(top_participants[:self.hero_selection_top_n], 1):
                hero = {
                    'player_id': participant['player_id'],
                    'rank': i,
                    'contribution_score': participant['contribution_score'],
                    'title_reward': f"{arc['arc_name']} Champion" if i == 1 else f"{arc['arc_name']} Hero",
                    'special_dialogue': f"Your heroic deeds in the {arc['arc_name']} will be remembered!",
                    'achievements': participant.get('notable_choices', [])
                }
                heroes.append(hero)
            
            # Define rewards
            rewards = {
                'rank_1': {
                    'title': f"{arc['arc_name']} Champion",
                    'items': [{'item_id': 617, 'name': 'Old Violet Box', 'quantity': 5}],
                    'special_buff': 'Hero Aura (7 days)'
                },
                'rank_2': {
                    'title': f"{arc['arc_name']} Hero",
                    'items': [{'item_id': 617, 'name': 'Old Violet Box', 'quantity': 3}]
                },
                'rank_3': {
                    'title': f"{arc['arc_name']} Defender",
                    'items': [{'item_id': 607, 'name': 'Yggdrasil Berry', 'quantity': 10}]
                }
            }
            
            response = HeroSelectionResponse(
                arc_id=arc_id,
                heroes=heroes,
                selection_criteria="Contribution score, chapters completed, quests finished",
                rewards=rewards
            )
            
            # Store hero recognition in database
            await self._store_hero_recognition(arc_id, heroes, rewards)
            
            # Queue broadcast
            await self._broadcast_hero_announcement(arc_id, heroes, arc['arc_name'])
            
            logger.info(f"✓ Selected {len(heroes)} heroes for arc #{arc_id}")
            
            return response
            
        except Exception as e:
            logger.error(f"Failed to generate hero recognition: {e}")
            return HeroSelectionResponse(
                arc_id=arc_id,
                heroes=[],
                selection_criteria="Error occurred",
                rewards={}
            )
    
    async def evolve_recurring_character(
        self,
        npc_id: int,
        new_arc_id: int,
        previous_interactions: List[Dict[str, Any]]
    ) -> RecurringCharacterUpdate:
        """
        Evolve recurring character for new arc appearance.
        
        Evolution based on:
        - Previous player interactions
        - Past arc outcomes
        - Time elapsed
        - New arc theme
        
        Args:
            npc_id: Recurring NPC ID
            new_arc_id: New arc to appear in
            previous_interactions: History of interactions
            
        Returns:
            RecurringCharacterUpdate with evolved character
        """
        try:
            logger.info(f"Evolving recurring NPC #{npc_id} for arc #{new_arc_id}")
            
            # Get current NPC data
            npc = await self._get_npc_details(npc_id)
            if not npc or not npc['is_recurring']:
                raise ValueError(f"NPC #{npc_id} is not recurring")
            
            appearances = npc['appearances_count'] + 1
            
            # TODO: Use LLM to evolve personality based on interactions
            # For now, use rule-based evolution
            
            personality_evolution = {
                'experience_gained': len(previous_interactions),
                'reputation_change': sum(i.get('impact', 0) for i in previous_interactions),
                'new_quirks': []
            }
            
            new_dialogue = [
                f"Ah, we meet again! It's been quite a journey since {npc['npc_location']}.",
                "I've learned much since our last encounter.",
                "What brings you back to me, old friend?"
            ]
            
            # Update NPC record
            await postgres_db.execute(
                """
                UPDATE story_npcs
                SET appearances_count = $1,
                    personality_data = personality_data || $2::jsonb
                WHERE npc_id = $3
                """,
                appearances,
                json.dumps({'evolution': personality_evolution}),
                npc_id
            )
            
            update = RecurringCharacterUpdate(
                npc_id=npc_id,
                new_appearance=appearances,
                personality_evolution=personality_evolution,
                new_dialogue=new_dialogue,
                relationship_changes={}
            )
            
            logger.info(f"✓ Recurring NPC #{npc_id} evolved (appearance #{appearances})")
            
            return update
            
        except Exception as e:
            logger.error(f"Failed to evolve recurring character: {e}")
            raise
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _generate_npc_personality(
        self,
        npc_spec: StoryEventNPC,
        arc_context: Dict[str, Any]
    ) -> StoryNPCPersonality:
        """
        Generate rich NPC personality using LLM or templates.
        
        Args:
            npc_spec: NPC specification from arc
            arc_context: Full arc context
            
        Returns:
            StoryNPCPersonality
        """
        try:
            # TODO: Call LLM for creative personality generation
            # For now, use template-based approach
            
            # Map role to personality type
            role_personalities = {
                NPCRole.PROTAGONIST: 'hero',
                NPCRole.ANTAGONIST: 'villain',
                NPCRole.ALLY: 'companion',
                NPCRole.MENTOR: 'sage',
                NPCRole.NEUTRAL: 'observer'
            }
            
            personality_type = role_personalities.get(npc_spec.npc_role, 'neutral')
            
            # Generate traits based on role
            role_traits = {
                'hero': ['brave', 'determined', 'compassionate'],
                'villain': ['cunning', 'ruthless', 'ambitious'],
                'companion': ['loyal', 'supportive', 'cheerful'],
                'sage': ['wise', 'patient', 'mysterious'],
                'observer': ['neutral', 'observant', 'cautious']
            }
            
            traits = role_traits.get(personality_type, ['neutral'])
            
            # Generate backstory
            faction = arc_context.get('dominant_faction', 'none')
            theme = arc_context.get('theme', 'adventure')
            
            backstory = (
                f"A {personality_type} connected to the {theme} unfolding across the realm. "
                f"{'Aligned with ' + faction if faction != 'none' else 'Independent'}, "
                f"they seek to influence the outcome."
            )
            
            # Speech patterns based on role
            speech_patterns = {
                'hero': ['speaks confidently', 'uses inspiring language'],
                'villain': ['speaks menacingly', 'uses dramatic pauses'],
                'companion': ['speaks warmly', 'uses friendly terms'],
                'sage': ['speaks cryptically', 'references ancient lore'],
                'observer': ['speaks neutrally', 'asks questions']
            }
            
            personality = StoryNPCPersonality(
                personality_type=personality_type,
                traits=traits,
                motivations=[f"Fulfill their role in the {theme}"],
                backstory=backstory,
                speech_patterns=speech_patterns.get(personality_type, ['speaks normally']),
                relationships={}
            )
            
            return personality
            
        except Exception as e:
            logger.error(f"Failed to generate NPC personality: {e}")
            # Return minimal personality
            return StoryNPCPersonality(
                personality_type='neutral',
                traits=['mysterious'],
                motivations=['unknown'],
                backstory='A mysterious figure',
                speech_patterns=['speaks cryptically']
            )
    
    async def _create_dialogue_tree(
        self,
        npc_spec: StoryEventNPC,
        personality: StoryNPCPersonality
    ) -> Dict[str, Any]:
        """
        Create branching dialogue tree for NPC.
        
        Args:
            npc_spec: NPC specification
            personality: Generated personality
            
        Returns:
            Dialogue tree structure
        """
        try:
            # Build dialogue tree with quest delivery if applicable
            dialogue_tree = {
                'root': {
                    'id': 'greeting',
                    'text': npc_spec.dialogue[0] if npc_spec.dialogue else "Greetings, adventurer.",
                    'options': []
                },
                'nodes': {}
            }
            
            # Add quest delivery branch if quest exists
            if npc_spec.quest:
                dialogue_tree['root']['options'].append({
                    'text': "Tell me about the quest",
                    'next_node': 'quest_intro'
                })
                
                dialogue_tree['nodes']['quest_intro'] = {
                    'id': 'quest_intro',
                    'text': npc_spec.quest.objective,
                    'quest_id': None,  # Will be set by StoryQuestBridge
                    'options': [
                        {'text': 'I accept this quest', 'action': 'accept_quest'},
                        {'text': 'Tell me more', 'next_node': 'quest_details'},
                        {'text': 'Not now', 'action': 'decline'}
                    ]
                }
                
                dialogue_tree['nodes']['quest_details'] = {
                    'id': 'quest_details',
                    'text': f"Quest: {npc_spec.quest.title}",
                    'options': [
                        {'text': 'I accept', 'action': 'accept_quest'},
                        {'text': 'I need to prepare', 'action': 'decline'}
                    ]
                }
            
            # Add dialogue lines from spec
            for i, line in enumerate(npc_spec.dialogue[1:], 1):
                node_id = f'dialogue_{i}'
                dialogue_tree['nodes'][node_id] = {
                    'id': node_id,
                    'text': line,
                    'options': [
                        {'text': 'Continue', 'next_node': f'dialogue_{i+1}' if i < len(npc_spec.dialogue)-1 else None}
                    ]
                }
                
                # Link from root or previous
                if i == 1 and 'Tell me more' not in [o['text'] for o in dialogue_tree['root']['options']]:
                    dialogue_tree['root']['options'].append({
                        'text': 'Tell me more',
                        'next_node': node_id
                    })
            
            # Add farewell option
            dialogue_tree['root']['options'].append({
                'text': 'Farewell',
                'action': 'end_conversation'
            })
            
            return dialogue_tree
            
        except Exception as e:
            logger.error(f"Failed to create dialogue tree: {e}")
            return {
                'root': {
                    'id': 'greeting',
                    'text': "Hello",
                    'options': [{'text': 'Goodbye', 'action': 'end_conversation'}]
                },
                'nodes': {}
            }
    
    async def _register_with_dialogue_agent(
        self,
        npc_id: int,
        npc_spec: StoryEventNPC,
        dialogue_tree: Dict[str, Any]
    ):
        """Register NPC with Dialogue Agent for conversation handling"""
        try:
            # Store in cache for Dialogue Agent pickup
            dialogue_data = {
                'npc_id': npc_id,
                'npc_name': npc_spec.npc_name,
                'dialogue_tree': dialogue_tree,
                'is_story_npc': True,
                'registered_at': datetime.now(UTC).isoformat()
            }
            
            await db.set(
                f"dialogue:story_npc:{npc_id}",
                dialogue_data,
                expire=86400 * 7  # 7 days
            )
            
            logger.debug(f"Registered NPC #{npc_id} with Dialogue Agent")
            
        except Exception as e:
            logger.warning(f"Failed to register with Dialogue Agent: {e}")
    
    async def _queue_npc_spawn(
        self,
        npc_id: int,
        npc_spec: StoryEventNPC
    ):
        """Queue NPC spawn via Bridge Layer"""
        try:
            spawn_command = {
                'command': 'spawn_story_npc',
                'npc_id': npc_id,
                'npc_name': npc_spec.npc_name,
                'sprite': npc_spec.npc_sprite,
                'map': npc_spec.npc_location,
                'x': 150,  # TODO: Get proper coordinates
                'y': 150,
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Queue in Bridge Layer pickup location
            await db.set(
                f"bridge:spawn_queue:npc:{npc_id}",
                spawn_command,
                expire=300  # 5 minutes for Bridge Layer to pickup
            )
            
            logger.debug(f"Queued spawn for NPC #{npc_id}")
            
        except Exception as e:
            logger.warning(f"Failed to queue NPC spawn: {e}")
    
    async def _get_arc_details(self, arc_id: int) -> Dict[str, Any]:
        """Get arc details from database"""
        try:
            query = """
                SELECT arc_name, arc_summary, theme
                FROM story_arcs
                WHERE arc_id = $1
            """
            return await postgres_db.fetch_one(query, arc_id)
        except Exception as e:
            logger.error(f"Failed to get arc details: {e}")
            return {'arc_name': 'Unknown Arc', 'arc_summary': '', 'theme': None}
    
    async def _get_npc_details(self, npc_id: int) -> Optional[Dict[str, Any]]:
        """Get NPC details from database"""
        try:
            query = """
                SELECT * FROM story_npcs WHERE npc_id = $1
            """
            return await postgres_db.fetch_one(query, npc_id)
        except Exception as e:
            logger.error(f"Failed to get NPC details: {e}")
            return None
    
    async def _store_hero_recognition(
        self,
        arc_id: int,
        heroes: List[Dict[str, Any]],
        rewards: Dict[str, Any]
    ):
        """Store hero recognition in database"""
        try:
            # Store in arc metadata
            await postgres_db.execute(
                """
                UPDATE story_arcs
                SET metadata = COALESCE(metadata, '{}'::jsonb) || $1::jsonb
                WHERE arc_id = $2
                """,
                json.dumps({
                    'heroes': heroes,
                    'hero_rewards': rewards,
                    'hero_selection_date': datetime.now(UTC).isoformat()
                }),
                arc_id
            )
            
            logger.debug(f"Stored hero recognition for arc #{arc_id}")
            
        except Exception as e:
            logger.warning(f"Failed to store hero recognition: {e}")
    
    async def _broadcast_hero_announcement(
        self,
        arc_id: int,
        heroes: List[Dict[str, Any]],
        arc_name: str
    ):
        """Broadcast hero announcement"""
        try:
            announcement = {
                'type': 'hero_recognition',
                'arc_id': arc_id,
                'arc_name': arc_name,
                'heroes': heroes,
                'message': f"Heroes of {arc_name}: {', '.join([f'Player {h['player_id']}' for h in heroes[:3]])}!",
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Queue for Bridge Layer
            await db.set(
                f"broadcast:hero_recognition:{arc_id}",
                announcement,
                expire=3600
            )
            
            logger.info(f"Hero announcement queued for arc #{arc_id}")
            
        except Exception as e:
            logger.warning(f"Failed to queue hero announcement: {e}")


# Global instance
npc_story_integration = NPCStoryIntegration()