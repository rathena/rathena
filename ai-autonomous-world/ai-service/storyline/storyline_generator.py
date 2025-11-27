"""
Storyline Generator Service
LLM-powered narrative engine for dynamic story arc generation
"""

import asyncio
import json
import hashlib
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger
from pydantic import ValidationError

from llm.factory import get_llm_provider_with_fallback
from database import db, postgres_db
from config import settings
from models.storyline import (
    StoryArcSpec, WorldStateSnapshot, VillainEvolutionSpec,
    LLMValidationResult, StoryEventNPC, StoryQuest
)
from storyline.prompts import (
    STORYLINE_SYSTEM_PROMPT,
    format_story_generation_prompt,
    VILLAIN_EVOLUTION_PROMPT_TEMPLATE,
    VALIDATION_ERROR_CORRECTION_PROMPT,
    TEMPLATE_FALLBACK_STORY,
    build_json_schema_prompt
)


class StorylineGenerator:
    """
    LLM-powered narrative engine for dynamic storytelling.
    
    Features:
    - Remembers past arcs (context window)
    - Player choice consequences
    - Villain evolution
    - Multi-chapter arcs (7-30 days)
    - Branching narratives
    - Geography-aware (uses visited/unvisited maps)
    - Economy-aware (inflation creates merchant crisis)
    - Faction-aware (dominant faction influences plot)
    - Karma-aware (morality affects story tone)
    
    LLM Strategy:
    - Use function calling for structured output
    - Retry on JSON parse errors (max 3)
    - Validate output against Pydantic schema
    - Cache similar generations (7-day TTL)
    - Fallback to templates if LLM fails
    """
    
    def __init__(
        self,
        llm_provider: Optional[str] = None,
        model: Optional[str] = None
    ):
        """
        Initialize Storyline Generator.
        
        Args:
            llm_provider: LLM provider name (defaults to settings)
            model: Model name (defaults to settings)
        """
        # Use storyline-specific config or fallback to general settings
        self.llm_provider_name = llm_provider or getattr(
            settings, 'STORYLINE_LLM_PROVIDER', 'azure_openai'
        )
        self.model = model or getattr(
            settings, 'STORYLINE_LLM_MODEL', 'gpt-4-turbo'
        )
        
        # Initialize LLM with fallback chain
        self.llm = get_llm_provider_with_fallback(
            provider_chain=getattr(settings, 'llm_fallback_chain', None)
        )
        
        # Generation parameters
        self.max_context_tokens = getattr(
            settings, 'STORYLINE_MAX_TOKENS', 8000
        )
        self.temperature = getattr(
            settings, 'STORYLINE_TEMPERATURE', 0.8
        )
        self.max_retries = 3
        self.cache_ttl_days = 7
        
        # Cost tracking
        self.monthly_budget = getattr(
            settings, 'STORYLINE_MONTHLY_BUDGET', 500
        )
        self.fallback_to_templates = getattr(
            settings, 'STORYLINE_FALLBACK_TO_TEMPLATES', True
        )
        
        # Generation config
        self.generation_config = {
            'min_duration_days': getattr(settings, 'ARC_DURATION_DAYS_MIN', 7),
            'max_duration_days': getattr(settings, 'ARC_DURATION_DAYS_MAX', 30),
            'min_chapters': 3,
            'min_npcs': 3,
            'max_npcs': 10,
            'min_quests': 5
        }
        
        logger.info(
            f"Storyline Generator initialized: "
            f"provider={self.llm_provider_name}, "
            f"model={self.model}, "
            f"temperature={self.temperature}"
        )
    
    async def generate_story_arc(
        self,
        world_state: WorldStateSnapshot,
        previous_arcs: List[Dict[str, Any]],
        force_theme: Optional[str] = None
    ) -> Optional[StoryArcSpec]:
        """
        Generate complete story arc using LLM.
        
        Process:
        1. Format comprehensive prompt with world state
        2. Call LLM with structured output request
        3. Parse and validate JSON response
        4. Retry on errors (max 3 attempts)
        5. Fallback to template if all retries fail
        
        Args:
            world_state: Current world state snapshot
            previous_arcs: List of previous story arcs for continuity
            force_theme: Optional forced theme (admin override)
            
        Returns:
            Validated StoryArcSpec or None if generation failed
        """
        try:
            logger.info("Generating story arc with LLM...")
            
            # Check cache first
            cache_key = self._generate_cache_key(world_state, previous_arcs)
            cached_arc = await self._get_cached_arc(cache_key)
            if cached_arc:
                logger.info("Using cached story arc")
                return cached_arc
            
            # Format prompt
            user_prompt = format_story_generation_prompt(
                world_state=world_state.model_dump(),
                previous_arcs=previous_arcs,
                config=self.generation_config
            )
            
            # Add theme override if specified
            if force_theme:
                user_prompt += f"\n\nREQUIRED THEME: {force_theme}"
            
            # Attempt LLM generation with retries
            for attempt in range(1, self.max_retries + 1):
                try:
                    logger.info(f"LLM generation attempt {attempt}/{self.max_retries}")
                    
                    # Call LLM
                    response = await self.llm.generate(
                        prompt=user_prompt,
                        system_prompt=STORYLINE_SYSTEM_PROMPT,
                        temperature=self.temperature,
                        max_tokens=self.max_context_tokens
                    )
                    
                    # Track cost
                    await self._track_generation_cost(response)
                    
                    # Parse response
                    arc_data = self._parse_llm_response(response.content)
                    
                    if not arc_data:
                        logger.warning(f"Failed to parse LLM response on attempt {attempt}")
                        continue
                    
                    # Validate and sanitize
                    validation_result = await self.validate_story_output(arc_data)
                    
                    if validation_result.is_valid and validation_result.sanitized_output:
                        # Success! Cache and return
                        await self._cache_arc(cache_key, validation_result.sanitized_output)
                        
                        logger.info(
                            f"✓ Story arc generated successfully: "
                            f"{validation_result.sanitized_output.story_arc_name}"
                        )
                        
                        return validation_result.sanitized_output
                    
                    # Validation failed - try correction
                    if attempt < self.max_retries:
                        logger.info(f"Validation errors: {validation_result.errors}. Attempting correction...")
                        arc_data = await self._attempt_error_correction(arc_data, validation_result.errors)
                        if arc_data:
                            validation_result = await self.validate_story_output(arc_data)
                            if validation_result.is_valid and validation_result.sanitized_output:
                                await self._cache_arc(cache_key, validation_result.sanitized_output)
                                return validation_result.sanitized_output
                
                except Exception as e:
                    logger.error(f"LLM generation attempt {attempt} failed: {e}")
                    if attempt == self.max_retries:
                        break
                    await asyncio.sleep(2 ** attempt)  # Exponential backoff
            
            # All retries failed - use template fallback if enabled
            if self.fallback_to_templates:
                logger.warning("All LLM attempts failed. Using template fallback.")
                return await self._generate_template_fallback(world_state)
            
            logger.error("Story arc generation failed and template fallback disabled")
            return None
            
        except Exception as e:
            logger.error(f"Story arc generation error: {e}", exc_info=True)
            
            if self.fallback_to_templates:
                return await self._generate_template_fallback(world_state)
            
            return None
    
    async def evolve_villain(
        self,
        villain_name: str,
        previous_defeats: int,
        player_strength: int,
        success_rate: float
    ) -> Optional[VillainEvolutionSpec]:
        """
        Evolve villain based on player progression.
        
        Evolution Rules:
        - More defeats → villain gets smarter/stronger
        - High player strength → villain recruits allies
        - Faction alignment → villain changes tactics
        - Success rate → determines evolution tier
        
        Args:
            villain_name: Current villain name
            previous_defeats: Number of times defeated
            player_strength: Average player level
            success_rate: Player success rate vs villain (0.0-1.0)
            
        Returns:
            VillainEvolutionSpec or None if evolution failed
        """
        try:
            logger.info(f"Evolving villain: {villain_name} (defeats: {previous_defeats})")
            
            # Calculate evolution tier (1-5)
            if previous_defeats == 0:
                evolution_tier = 1
            elif previous_defeats <= 2:
                evolution_tier = 2
            elif previous_defeats <= 4:
                evolution_tier = 3
            elif previous_defeats <= 7:
                evolution_tier = 4
            else:
                evolution_tier = 5
            
            # Format prompt
            prompt = VILLAIN_EVOLUTION_PROMPT_TEMPLATE.format(
                villain_name=villain_name,
                previous_defeats=previous_defeats,
                last_encounter_summary=f"Defeated {previous_defeats} times",
                player_level=player_strength,
                success_rate=success_rate,
                participant_count=int(success_rate * 100)  # Estimate
            )
            
            # Call LLM
            response = await self.llm.generate(
                prompt=prompt,
                system_prompt="You are a villain evolution designer for an MMORPG. Create compelling villain progression.",
                temperature=0.7,
                max_tokens=1000
            )
            
            # Parse response
            villain_data = self._parse_llm_response(response.content)
            
            if villain_data:
                # Validate and create spec
                evolution = VillainEvolutionSpec(
                    villain_name=villain_data.get('villain_name', villain_name),
                    previous_defeats=previous_defeats,
                    player_strength_level=player_strength,
                    evolution_tier=evolution_tier,
                    new_abilities=villain_data.get('new_abilities', []),
                    recruited_allies=villain_data.get('recruited_allies', []),
                    strategy_changes=villain_data.get('strategy_changes', [])
                )
                
                logger.info(f"✓ Villain evolved to tier {evolution_tier}")
                return evolution
            
            return None
            
        except Exception as e:
            logger.error(f"Villain evolution failed: {e}")
            return None
    
    async def validate_story_output(
        self,
        llm_output: Dict[str, Any]
    ) -> LLMValidationResult:
        """
        Validate and sanitize LLM output.
        
        Validation Steps:
        1. Parse as StoryArcSpec (Pydantic validation)
        2. Check all required fields present
        3. Validate map names exist (TODO: add map validation)
        4. Validate monster IDs exist (TODO: add monster validation)
        5. Validate item IDs exist (TODO: add item validation)
        6. Ensure quest objectives are achievable
        
        Args:
            llm_output: Raw LLM output as dict
            
        Returns:
            LLMValidationResult with validation status
        """
        errors = []
        warnings = []
        
        try:
            # Attempt Pydantic validation
            story_arc = StoryArcSpec(**llm_output)
            
            # Additional validations
            
            # 1. Check events exist
            if not story_arc.events:
                errors.append("No events defined in story arc")
            
            # 2. Validate NPC count
            if len(story_arc.events) < 2:
                warnings.append("Only 1 NPC defined - consider adding more for richer story")
            
            # 3. Validate quest completability
            for event in story_arc.events:
                if event.quest:
                    if not event.quest.objectives and not event.quest.monsters and not event.quest.required_items:
                        errors.append(f"Quest '{event.quest.title}' has no objectives")
            
            # 4. Check duration
            if not (7 <= story_arc.duration_days <= 30):
                errors.append(f"Duration {story_arc.duration_days} days outside valid range (7-30)")
            
            # 5. Validate outcomes
            if not story_arc.success_outcomes.world_changes:
                warnings.append("No world changes defined for success outcome")
            
            if not story_arc.failure_outcomes.world_changes:
                warnings.append("No world changes defined for failure outcome")
            
            # TODO: Add map/monster/item ID validation against game database
            
            if errors:
                logger.warning(f"Story arc validation failed: {len(errors)} errors")
                return LLMValidationResult(
                    is_valid=False,
                    errors=errors,
                    warnings=warnings,
                    sanitized_output=None
                )
            
            if warnings:
                logger.info(f"Story arc validated with {len(warnings)} warnings")
            else:
                logger.info("Story arc validated successfully")
            
            return LLMValidationResult(
                is_valid=True,
                errors=[],
                warnings=warnings,
                sanitized_output=story_arc
            )
            
        except ValidationError as e:
            logger.error(f"Pydantic validation failed: {e}")
            errors.append(f"Schema validation failed: {str(e)}")
            
            return LLMValidationResult(
                is_valid=False,
                errors=errors,
                warnings=warnings,
                sanitized_output=None
            )
        
        except Exception as e:
            logger.error(f"Validation error: {e}")
            errors.append(f"Validation exception: {str(e)}")
            
            return LLMValidationResult(
                is_valid=False,
                errors=errors,
                warnings=warnings,
                sanitized_output=None
            )
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _parse_llm_response(self, response_text: str) -> Optional[Dict[str, Any]]:
        """
        Parse JSON from LLM response with multiple strategies.
        
        Strategies:
        1. Direct JSON parse
        2. Extract from ```json``` code blocks
        3. Extract from ``` code blocks
        4. Find first { and last }
        
        Args:
            response_text: Raw LLM response
            
        Returns:
            Parsed dict or None
        """
        try:
            # Strategy 1: Direct parse
            return json.loads(response_text)
        except json.JSONDecodeError:
            pass
        
        try:
            # Strategy 2: Extract from ```json``` blocks
            if "```json" in response_text:
                start = response_text.find("```json") + 7
                end = response_text.find("```", start)
                if end > start:
                    json_str = response_text[start:end].strip()
                    return json.loads(json_str)
        except json.JSONDecodeError:
            pass
        
        try:
            # Strategy 3: Extract from ``` blocks
            if "```" in response_text:
                start = response_text.find("```") + 3
                end = response_text.find("```", start)
                if end > start:
                    json_str = response_text[start:end].strip()
                    return json.loads(json_str)
        except json.JSONDecodeError:
            pass
        
        try:
            # Strategy 4: Find JSON object boundaries
            start = response_text.find("{")
            end = response_text.rfind("}") + 1
            if start >= 0 and end > start:
                json_str = response_text[start:end]
                return json.loads(json_str)
        except json.JSONDecodeError:
            pass
        
        logger.error("Failed to parse LLM response with all strategies")
        return None
    
    async def _attempt_error_correction(
        self,
        failed_output: Dict[str, Any],
        validation_errors: List[str]
    ) -> Optional[Dict[str, Any]]:
        """
        Attempt to correct validation errors using LLM.
        
        Args:
            failed_output: Output that failed validation
            validation_errors: List of validation error messages
            
        Returns:
            Corrected output or None
        """
        try:
            logger.info("Attempting LLM-based error correction")
            
            correction_prompt = VALIDATION_ERROR_CORRECTION_PROMPT.format(
                validation_errors='\n'.join(f"- {err}" for err in validation_errors),
                original_output=json.dumps(failed_output, indent=2)
            )
            
            response = await self.llm.generate(
                prompt=correction_prompt,
                system_prompt="You are a JSON validation and correction expert. Fix the errors while preserving the creative content.",
                temperature=0.3,  # Lower temperature for correction
                max_tokens=4000
            )
            
            corrected = self._parse_llm_response(response.content)
            
            if corrected:
                logger.info("Error correction successful")
                return corrected
            
            return None
            
        except Exception as e:
            logger.error(f"Error correction failed: {e}")
            return None
    
    async def _generate_template_fallback(
        self,
        world_state: WorldStateSnapshot
    ) -> StoryArcSpec:
        """
        Generate story arc using template (no LLM).
        
        Used when:
        - LLM budget exceeded
        - All LLM retries failed
        - LLM temporarily unavailable
        
        Args:
            world_state: Current world state (for basic customization)
            
        Returns:
            Template-based StoryArcSpec
        """
        try:
            logger.info("Using template fallback for story generation")
            
            # Clone template
            arc_data = json.loads(json.dumps(TEMPLATE_FALLBACK_STORY))
            
            # Basic customization based on world state
            if world_state.dominant_faction:
                arc_data['faction_impact']['faction_id'] = world_state.dominant_faction
            
            if world_state.global_karma > 3000:
                arc_data['theme'] = 'redemption'
            elif world_state.global_karma < -3000:
                arc_data['theme'] = 'corruption'
            
            # Use least visited map for variety
            if world_state.least_visited_maps:
                arc_data['events'][0]['npc_location'] = world_state.least_visited_maps[0]
            
            # Adjust difficulty to player level
            for event in arc_data.get('events', []):
                if event.get('quest'):
                    base_exp = world_state.player_average_level * 1000
                    event['quest']['reward']['exp'] = int(base_exp)
            
            # Validate template
            story_arc = StoryArcSpec(**arc_data)
            
            logger.info(f"✓ Template fallback generated: {story_arc.story_arc_name}")
            
            return story_arc
            
        except Exception as e:
            logger.error(f"Template fallback failed: {e}")
            raise
    
    async def _track_generation_cost(self, response: Any):
        """Track LLM generation cost"""
        try:
            # Get cost manager
            from services.cost_manager import get_cost_manager
            cost_manager = get_cost_manager()
            
            # Extract cost metrics
            provider = getattr(response, 'provider', self.llm_provider_name)
            model = getattr(response, 'model', self.model)
            tokens_input = getattr(response, 'tokens_input', 0)
            tokens_output = getattr(response, 'tokens_output', 0)
            cost_usd = getattr(response, 'cost_usd', 0.0)
            
            # Record cost
            cost_manager.record_cost(
                provider=provider,
                model=model,
                tokens_input=tokens_input,
                tokens_output=tokens_output,
                cost_usd=cost_usd,
                request_type='storyline_generation'
            )
            
            logger.info(
                f"Story generation cost: ${cost_usd:.4f} "
                f"({tokens_input + tokens_output} tokens)"
            )
            
        except Exception as e:
            logger.warning(f"Cost tracking failed: {e}")
    
    def _generate_cache_key(
        self,
        world_state: WorldStateSnapshot,
        previous_arcs: List[Dict[str, Any]]
    ) -> str:
        """Generate deterministic cache key from world state"""
        # Create key from significant state attributes
        key_data = {
            'avg_level': world_state.player_average_level // 10 * 10,  # Round to nearest 10
            'dominant_faction': world_state.dominant_faction,
            'karma_alignment': 'good' if world_state.global_karma > 3000 else 'evil' if world_state.global_karma < -3000 else 'neutral',
            'top_maps': sorted(world_state.most_visited_maps[:3]),
            'arc_count': len(previous_arcs),
            'week': datetime.now(UTC).strftime("%Y-W%W")  # Weekly rotation
        }
        
        key_str = json.dumps(key_data, sort_keys=True)
        return f"storyline:arc:cache:{hashlib.md5(key_str.encode()).hexdigest()}"
    
    async def _get_cached_arc(self, cache_key: str) -> Optional[StoryArcSpec]:
        """Get cached story arc"""
        try:
            cached = await db.get(cache_key)
            if cached:
                return StoryArcSpec(**cached)
            return None
        except Exception as e:
            logger.debug(f"Cache miss: {e}")
            return None
    
    async def _cache_arc(self, cache_key: str, story_arc: StoryArcSpec):
        """Cache generated story arc"""
        try:
            ttl_seconds = self.cache_ttl_days * 24 * 3600
            await db.set(cache_key, story_arc.model_dump(), expire=ttl_seconds)
            logger.debug(f"Cached story arc for {self.cache_ttl_days} days")
        except Exception as e:
            logger.warning(f"Failed to cache arc: {e}")
    
    def _count_data_points(self, world_state: WorldStateSnapshot) -> int:
        """Count total data points in world state"""
        count = 0
        count += len(world_state.most_visited_maps)
        count += len(world_state.least_visited_maps)
        count += len(world_state.map_kill_counts)
        count += len(world_state.player_level_distribution)
        count += len(world_state.mvp_kill_frequency)
        count += len(world_state.faction_scores)
        count += len(world_state.active_problems)
        count += len(world_state.previous_arcs)
        count += len(world_state.player_choices_made)
        return count


# Global instance
storyline_generator = StorylineGenerator()