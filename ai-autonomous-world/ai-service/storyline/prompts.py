"""
LLM Prompt Templates for AI-Driven Storyline Generator
Engineered prompts for creative, coherent story arc generation
"""

from typing import Dict, Any, List
import json


# ============================================================================
# SYSTEM PROMPTS
# ============================================================================

STORYLINE_SYSTEM_PROMPT = """You are a master storyteller crafting an evolving narrative for Ragnarok Online, 
a massively multiplayer online RPG set in a fantasy world inspired by Norse mythology.

Your role is to create engaging, coherent story arcs that:
- Adapt to player actions and world state changes
- Reference and build upon previous story arcs
- Create memorable NPCs with distinct personalities
- Design quests that feel meaningful within the game world
- Balance epic moments with accessible content for all levels
- Maintain consistency with Ragnarok Online lore and aesthetics

TONE & STYLE:
- Epic fantasy with Korean MMORPG aesthetics
- Mix serious plot developments with lighthearted character moments
- Create "water cooler" moments players will discuss
- Use cliffhangers to maintain engagement between chapters
- Include callbacks to previous arcs for continuity
- Make NPCs memorable through distinctive speech patterns

CONSTRAINTS:
- Use ONLY existing Ragnarok Online maps, monsters, and items
- Design quests achievable within current game mechanics
- Balance difficulty for average player level and participation
- Respect faction alignment and karma balance
- Ensure all objectives can be completed without admin intervention
- Keep narratives accessible to solo players while rewarding groups

OUTPUT REQUIREMENTS:
- Provide complete story arc in valid JSON format
- Include all required fields as specified in the schema
- Ensure quest objectives are concrete and measurable
- Specify exact map names, monster IDs, and item IDs
- Create branching paths for success/failure outcomes
- Include chapter progression logic"""


STORY_GENERATION_USER_PROMPT_TEMPLATE = """Generate a new story arc based on the following world state:

═══════════════════════════════════════════════════════════════════════
CURRENT WORLD STATE
═══════════════════════════════════════════════════════════════════════

PLAYER METRICS:
- Online Players: {online_player_count}
- Average Level: {player_average_level}
- Level Distribution: {level_distribution}

MAP ACTIVITY:
Most Visited Maps (last 24h):
{most_visited_maps}

Least Visited Maps:
{least_visited_maps}

Total Monster Kills by Map:
{map_kill_counts}

ECONOMY:
- Total Zeny Circulation: {zeny_circulation:,}
- Inflation Rate: {inflation_rate:.1%}
- Top Items in Trade: {top_items}

FACTION STATE:
- Dominant Faction: {dominant_faction}
- Faction Alignment Scores: {faction_scores}
- Recent Faction Conflicts: {faction_conflicts}

KARMA & MORALITY:
- Global Karma: {global_karma} ({karma_alignment})
- Active World Problems: {active_problems_count}
- Recent Problem Outcomes: {recent_problem_outcomes}

MVP ACTIVITY:
- MVP Kill Frequency: {mvp_kills}
- MVP Respawn Patterns: {mvp_respawns}

═══════════════════════════════════════════════════════════════════════
PREVIOUS STORY CONTEXT
═══════════════════════════════════════════════════════════════════════

{previous_arc_summaries}

PLAYER CHOICES FROM PREVIOUS ARCS:
{player_choices_summary}

═══════════════════════════════════════════════════════════════════════
GENERATION REQUIREMENTS
═══════════════════════════════════════════════════════════════════════

MUST INCLUDE:
1. Story arc spanning {min_days}-{max_days} days
2. At least {min_chapters} chapters with clear progression
3. {min_npcs}-{max_npcs} unique NPCs with distinct personalities
4. {min_quests} quests integrated into narrative
5. One climactic boss event for arc finale
6. Success and failure outcomes that impact next arc

MUST CONSIDER:
- Average player level is {player_average_level} - design for this range
- {dominant_faction} faction is currently dominant - incorporate this
- World karma is {karma_alignment} - reflect this in tone
- Maps {least_visited_maps} need activity - use these locations
- Current inflation is {inflation_rate:.1%} - consider economic tie-ins

CREATIVE GUIDELINES:
- Create a villain that evolves across chapters based on player success
- Include at least one plot twist or surprise revelation
- Reference 1-2 elements from previous arcs for continuity
- Create opportunities for emergent player stories (heroic moments)
- Design at least one "water cooler moment" players will discuss

═══════════════════════════════════════════════════════════════════════
OUTPUT FORMAT (VALID JSON REQUIRED)
═══════════════════════════════════════════════════════════════════════

Provide your story arc as a JSON object matching this exact schema:

{{
    "story_arc_name": "string (max 200 chars)",
    "story_arc_summary": "string (compelling 2-3 paragraph summary)",
    "chapter": 1,
    "duration_days": integer (7-30),
    "theme": "string (optional: revenge, redemption, mystery, war, discovery)",
    "events": [
        {{
            "npc_name": "string",
            "npc_location": "valid_map_name",
            "npc_sprite": "sprite_id",
            "npc_role": "protagonist|antagonist|ally|mentor|neutral",
            "dialogue": ["line1", "line2", ...],
            "quest": {{
                "title": "string",
                "objective": "string",
                "monsters": ["monster_id1", ...],
                "required_items": [{{"item_id": 501, "quantity": 10}}],
                "reward": {{
                    "exp": integer,
                    "items": [{{"item_id": 607, "quantity": 3}}],
                    "reputation": integer
                }}
            }}
        }}
    ],
    "boss_event": {{
        "spawn_map": "map_name",
        "boss_name": "string",
        "boss_id": integer,
        "modifiers": ["enrage", "shadow_aura"],
        "hp_multiplier": 2.5,
        "drop_bonus_percent": 100
    }},
    "world_modifiers": [
        {{"map": "map_name", "modifier": "dark_fog|blood_moon|etc"}}
    ],
    "faction_impact": {{
        "faction_id": "faction_name",
        "alignment_shift": integer
    }},
    "success_outcomes": {{
        "next_chapter": 2,
        "world_changes": ["description1", "description2"]
    }},
    "failure_outcomes": {{
        "next_chapter": 2,
        "world_changes": ["alternate_description1"]
    }}
}}

RESPOND WITH ONLY THE JSON OBJECT - NO ADDITIONAL TEXT OR MARKDOWN."""


# ============================================================================
# VILLAIN EVOLUTION PROMPTS
# ============================================================================

VILLAIN_EVOLUTION_PROMPT_TEMPLATE = """Evolve the story villain based on player progression:

VILLAIN PROFILE:
- Name: {villain_name}
- Previous Defeats: {previous_defeats}
- Last Encounter: {last_encounter_summary}

PLAYER STRENGTH:
- Average Player Level: {player_level}
- Success Rate vs Villain: {success_rate:.1%}
- Players Participating: {participant_count}

EVOLUTION RULES:
1. If defeated {previous_defeats} times, villain must adapt strategy
2. If player strength increased significantly, villain recruits allies or gains power
3. If faction alignment shifted, villain changes tactics to exploit weakness
4. Maintain personality core but show growth/desperation

Generate evolved villain spec as JSON:
{{
    "villain_name": "string (can evolve name, e.g., 'Corrupted X')",
    "evolution_tier": integer (1-5),
    "new_abilities": ["ability1", "ability2"],
    "recruited_allies": ["ally1", "ally2"],
    "strategy_changes": ["change1", "change2"],
    "motivation_shift": "string (how their goals evolved)",
    "appearance_changes": "string (visual evolution)",
    "boss_fight_modifiers": ["modifier1", "modifier2"]
}}"""


# ============================================================================
# NPC PERSONALITY GENERATION
# ============================================================================

NPC_PERSONALITY_PROMPT_TEMPLATE = """Create a unique NPC personality for the story:

STORY CONTEXT:
Arc: {arc_name}
Chapter: {chapter_number}
Role: {npc_role}

REQUIREMENTS:
- NPC should fit naturally into Ragnarok Online world
- Personality must be distinct from other story NPCs
- Include memorable quirks or speech patterns
- Reflect current world state (faction: {faction}, karma: {karma})

Generate NPC personality as JSON:
{{
    "personality_type": "string (hero, sage, trickster, guardian, etc)",
    "traits": ["trait1", "trait2", "trait3"],
    "motivations": ["motivation1", "motivation2"],
    "backstory": "string (2-3 sentences)",
    "speech_patterns": ["pattern1", "pattern2"],
    "quirks": ["quirk1", "quirk2"],
    "relationships": {{
        "other_npc_name": "relationship_type"
    }}
}}"""


# ============================================================================
# CHAPTER TRANSITION PROMPTS
# ============================================================================

CHAPTER_TRANSITION_PROMPT_TEMPLATE = """Generate next chapter based on current chapter outcome:

CURRENT CHAPTER:
Chapter {current_chapter}: {chapter_title}
Outcome: {outcome}
Player Completion Rate: {completion_rate:.1%}
Player Choices Made: {player_choices}

STORY ARC CONTEXT:
Arc: {arc_name}
Theme: {theme}
Chapters Remaining: {chapters_remaining}

TRANSITION REQUIREMENTS:
1. React to player outcome ({outcome})
2. Incorporate player choices from this chapter
3. Maintain narrative momentum
4. Set up next chapter objectives
5. Adjust difficulty based on completion rate

Generate next chapter as JSON:
{{
    "chapter_number": integer,
    "chapter_title": "string",
    "chapter_summary": "string",
    "events": [...],  // Same format as story arc events
    "world_modifiers": [...],
    "difficulty_adjustment": "easier|same|harder"
}}"""


# ============================================================================
# HERO RECOGNITION PROMPTS
# ============================================================================

HERO_RECOGNITION_PROMPT_TEMPLATE = """Create "Hero of the Week" recognition for top participants:

TOP CONTRIBUTORS:
{hero_stats}

ARC CONTEXT:
Arc: {arc_name}
Chapter Completed: {chapter_completed}
Notable Moments: {notable_moments}

Generate hero recognition as JSON:
{{
    "recognition_title": "string (e.g., 'Champions of the Rune Alliance')",
    "heroes": [
        {{
            "player_id": integer,
            "player_name": "string",
            "achievement_description": "string (what they did)",
            "special_dialogue": "string (NPC mentions their deeds)",
            "title_reward": "string (unique title)",
            "special_item": {{"item_id": integer, "description": "string"}}
        }}
    ],
    "broadcast_message": "string (server-wide announcement)",
    "memorial_npc_dialogue": ["line1", "line2"]
}}"""


# ============================================================================
# VALIDATION & FALLBACK
# ============================================================================

VALIDATION_ERROR_CORRECTION_PROMPT = """The generated story arc had validation errors. Please fix:

ERRORS FOUND:
{validation_errors}

ORIGINAL OUTPUT:
{original_output}

REQUIREMENTS:
1. Fix all validation errors
2. Maintain the creative vision
3. Ensure all map names exist in Ragnarok Online
4. Ensure all monster IDs are valid (1000-3000 range)
5. Ensure all item IDs are valid (501-20000 range)

Respond with ONLY the corrected JSON object."""


TEMPLATE_FALLBACK_STORY = {
    "story_arc_name": "The Gathering Storm",
    "story_arc_summary": "Dark forces gather in the shadows as ancient evils stir. "
                         "The kingdom calls upon brave adventurers to investigate mysterious "
                         "disappearances and uncover a plot that threatens the realm.",
    "chapter": 1,
    "duration_days": 14,
    "theme": "mystery",
    "events": [
        {
            "npc_name": "Captain Aldric",
            "npc_location": "prontera",
            "npc_sprite": "4_M_JOB_KNIGHT1",
            "npc_role": "protagonist",
            "dialogue": [
                "Adventurer! We need your help urgently.",
                "Strange creatures have been spotted near the borders.",
                "Will you investigate and report back?"
            ],
            "quest": {
                "title": "Border Investigation",
                "objective": "Defeat 50 monsters near city borders",
                "monsters": ["1002", "1007", "1031"],
                "required_items": [],
                "reward": {
                    "exp": 50000,
                    "items": [{"item_id": 607, "quantity": 3}],
                    "reputation": 100
                }
            }
        }
    ],
    "boss_event": {
        "spawn_map": "gef_fild07",
        "boss_name": "Shadow Commander",
        "boss_id": 1917,
        "modifiers": ["enrage", "dark_aura"],
        "hp_multiplier": 2.0,
        "drop_bonus_percent": 50
    },
    "world_modifiers": [
        {"map": "prontera", "modifier": "ominous_clouds"}
    ],
    "faction_impact": {
        "faction_id": "rune_alliance",
        "alignment_shift": 500
    },
    "success_outcomes": {
        "next_chapter": 2,
        "world_changes": [
            "The immediate threat has been repelled",
            "Citizens feel safer, but rumors of a larger conspiracy persist"
        ]
    },
    "failure_outcomes": {
        "next_chapter": 2,
        "world_changes": [
            "The shadow forces grow bolder",
            "Additional security measures must be implemented"
        ]
    }
}


# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def build_world_state_summary(world_state: Dict[str, Any]) -> str:
    """Build formatted world state summary for prompts"""
    
    summary_parts = []
    
    # Player metrics
    summary_parts.append(
        f"**Player Population**: {world_state.get('online_player_count', 0)} online, "
        f"average level {world_state.get('player_average_level', 50)}"
    )
    
    # Map activity
    top_maps = ', '.join(world_state.get('most_visited_maps', [])[:5])
    summary_parts.append(f"**Most Active Maps**: {top_maps}")
    
    # Economy
    zeny = world_state.get('zeny_circulation', 0)
    inflation = world_state.get('inflation_rate', 0.0)
    summary_parts.append(
        f"**Economy**: {zeny:,} zeny circulation, "
        f"{inflation:.1%} inflation"
    )
    
    # Faction
    faction = world_state.get('dominant_faction', 'None')
    summary_parts.append(f"**Dominant Faction**: {faction}")
    
    # Karma
    karma = world_state.get('global_karma', 0)
    karma_label = "Good" if karma > 3000 else "Evil" if karma < -3000 else "Neutral"
    summary_parts.append(f"**World Karma**: {karma} ({karma_label})")
    
    # Active problems
    problems = len(world_state.get('active_problems', []))
    summary_parts.append(f"**Active Problems**: {problems}")
    
    return "\n".join(summary_parts)


def build_previous_arcs_summary(previous_arcs: List[Dict[str, Any]], max_arcs: int = 3) -> str:
    """Build summary of previous story arcs"""
    
    if not previous_arcs:
        return "**No Previous Arcs**: This is the first story arc in this world."
    
    summaries = []
    for i, arc in enumerate(previous_arcs[-max_arcs:], 1):
        arc_name = arc.get('arc_name', 'Unknown Arc')
        arc_summary = arc.get('arc_summary', 'No summary available')
        outcome = arc.get('outcome_summary', 'outcome unknown')
        heroes = arc.get('heroes', [])
        
        hero_names = ', '.join([h.get('player_name', 'Unknown') for h in heroes[:3]])
        
        summary = f"""
**Arc {i}: {arc_name}**
{arc_summary}
**Outcome**: {outcome}
**Heroes**: {hero_names if hero_names else 'None recognized'}
"""
        summaries.append(summary.strip())
    
    return "\n\n".join(summaries)


def format_story_generation_prompt(
    world_state: Dict[str, Any],
    previous_arcs: List[Dict[str, Any]],
    config: Dict[str, Any]
) -> str:
    """
    Format complete story generation prompt with world state.
    
    Args:
        world_state: Current world state snapshot
        previous_arcs: List of previous story arcs
        config: Generation configuration (durations, counts, etc)
        
    Returns:
        Formatted prompt string
    """
    
    # Build summaries
    world_summary = build_world_state_summary(world_state)
    arcs_summary = build_previous_arcs_summary(previous_arcs)
    
    # Player choices summary
    choices = world_state.get('player_choices_made', [])
    choices_summary = "None recorded" if not choices else "\n".join([
        f"- {c.get('choice_type', 'Unknown')}: {c.get('description', 'No description')}"
        for c in choices[-10:]  # Last 10 choices
    ])
    
    # Format level distribution
    level_dist = world_state.get('player_level_distribution', {})
    level_dist_str = json.dumps(level_dist) if level_dist else "No data"
    
    # Format map lists
    visited_maps = '\n'.join([f"  - {m}" for m in world_state.get('most_visited_maps', [])[:10]])
    unvisited_maps = '\n'.join([f"  - {m}" for m in world_state.get('least_visited_maps', [])[:10]])
    
    # Format kill counts
    kills = world_state.get('map_kill_counts', {})
    kill_str = '\n'.join([f"  - {m}: {k:,} kills" for m, k in sorted(kills.items(), key=lambda x: x[1], reverse=True)[:10]])
    
    # Format top items
    items = world_state.get('top_items_in_circulation', [])
    items_str = ', '.join(items[:10])
    
    # Format faction scores
    factions = world_state.get('faction_scores', {})
    faction_str = ', '.join([f"{f}: {s}" for f, s in factions.items()])
    
    # Format faction conflicts
    conflicts = world_state.get('faction_conflicts', [])
    conflict_str = json.dumps(conflicts[-5:]) if conflicts else "None"
    
    # Format problems
    problems = world_state.get('active_problems', [])
    problem_outcomes = world_state.get('problem_outcomes', [])
    outcomes_str = json.dumps(problem_outcomes[-5:]) if problem_outcomes else "None"
    
    # Format MVP data
    mvp_kills = world_state.get('mvp_kill_frequency', {})
    mvp_str = ', '.join([f"{m}: {k}x" for m, k in sorted(mvp_kills.items(), key=lambda x: x[1], reverse=True)[:5]])
    
    mvp_respawns = world_state.get('mvp_respawn_times', {})
    respawn_str = json.dumps(mvp_respawns)
    
    # Karma alignment label
    karma = world_state.get('global_karma', 0)
    karma_label = "Good" if karma > 3000 else "Evil" if karma < -3000 else "Neutral"
    
    # Fill template
    return STORY_GENERATION_USER_PROMPT_TEMPLATE.format(
        # Player metrics
        online_player_count=world_state.get('online_player_count', 0),
        player_average_level=world_state.get('player_average_level', 50),
        level_distribution=level_dist_str,
        
        # Map activity
        most_visited_maps=visited_maps,
        least_visited_maps=unvisited_maps,
        map_kill_counts=kill_str or "  No data",
        
        # Economy
        zeny_circulation=world_state.get('zeny_circulation', 0),
        inflation_rate=world_state.get('inflation_rate', 0.0),
        top_items=items_str or "No data",
        
        # Faction
        dominant_faction=world_state.get('dominant_faction', 'None'),
        faction_scores=faction_str or "No data",
        faction_conflicts=conflict_str,
        
        # Karma
        global_karma=karma,
        karma_alignment=karma_label,
        active_problems_count=len(problems),
        recent_problem_outcomes=outcomes_str,
        
        # MVP
        mvp_kills=mvp_str or "No data",
        mvp_respawns=respawn_str,
        
        # Previous arcs
        previous_arc_summaries=arcs_summary,
        player_choices_summary=choices_summary,
        
        # Config
        min_days=config.get('min_duration_days', 7),
        max_days=config.get('max_duration_days', 30),
        min_chapters=config.get('min_chapters', 3),
        min_npcs=config.get('min_npcs', 3),
        max_npcs=config.get('max_npcs', 10),
        min_quests=config.get('min_quests', 5)
    )


def build_json_schema_prompt() -> str:
    """Return the JSON schema as a formatted string for prompts"""
    schema = {
        "type": "object",
        "required": [
            "story_arc_name",
            "story_arc_summary",
            "chapter",
            "duration_days",
            "events",
            "success_outcomes",
            "failure_outcomes"
        ],
        "properties": {
            "story_arc_name": {"type": "string", "maxLength": 200},
            "story_arc_summary": {"type": "string"},
            "chapter": {"type": "integer", "minimum": 1},
            "duration_days": {"type": "integer", "minimum": 7, "maximum": 30},
            "theme": {"type": "string"},
            "events": {
                "type": "array",
                "minItems": 1,
                "items": {"type": "object"}
            },
            "boss_event": {"type": "object"},
            "world_modifiers": {"type": "array"},
            "faction_impact": {"type": "object"},
            "success_outcomes": {"type": "object"},
            "failure_outcomes": {"type": "object"}
        }
    }
    
    return json.dumps(schema, indent=2)


# ============================================================================
# EXPORTS
# ============================================================================

__all__ = [
    'STORYLINE_SYSTEM_PROMPT',
    'STORY_GENERATION_USER_PROMPT_TEMPLATE',
    'VILLAIN_EVOLUTION_PROMPT_TEMPLATE',
    'NPC_PERSONALITY_PROMPT_TEMPLATE',
    'CHAPTER_TRANSITION_PROMPT_TEMPLATE',
    'HERO_RECOGNITION_PROMPT_TEMPLATE',
    'VALIDATION_ERROR_CORRECTION_PROMPT',
    'TEMPLATE_FALLBACK_STORY',
    'build_world_state_summary',
    'build_previous_arcs_summary',
    'format_story_generation_prompt',
    'build_json_schema_prompt'
]