"""
ML Monster AI - Archetype-Specific Reward Shaping
Enhanced ML Monster AI System v2.0

Implements reward functions for each of the 6 archetypes:
1. Aggressive - High damage, risk-taking
2. Defensive - Survival, damage mitigation
3. Support - Buffing, healing allies
4. Mage - Spell casting, mana management
5. Tank - Aggro control, damage absorption
6. Ranged - Kiting, distance maintenance
"""

import numpy as np
from typing import Dict, Any, Optional
import logging

logger = logging.getLogger(__name__)


class ArchetypeRewards:
    """
    Archetype-specific reward functions
    
    Each archetype has different reward priorities and weights
    """
    
    @staticmethod
    def aggressive_reward(
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Aggressive archetype reward
        
        Priorities:
        - High damage dealt (+++)
        - Kills (+++++)
        - Low damage taken (-)
        - Deaths (-----)
        - Distance to enemy (closer is better)
        
        Playstyle: Risk-taking, high damage output, pursue relentlessly
        
        Args:
            state: Current state info
            action: Action taken
            next_state: Resulting state info
            done: Whether episode ended
            info: Additional information
        
        Returns:
            Shaped reward
        """
        reward = 0.0
        
        # Damage dealt (high weight)
        damage_dealt = next_state.get('damage_dealt', 0.0)
        reward += damage_dealt * 2.0  # 2x weight on damage
        
        # Kills (very high reward)
        kills = next_state.get('kills', 0) - state.get('kills', 0)
        reward += kills * 100.0  # Big bonus for kills
        
        # Damage taken (moderate penalty)
        damage_taken = next_state.get('damage_taken', 0.0)
        reward -= damage_taken * 0.5  # Don't care too much about taking damage
        
        # Death penalty
        if done and next_state.get('died', False):
            reward -= 50.0
        
        # Proximity bonus (aggressive monsters want to be close to enemies)
        enemy_distance = next_state.get('enemy_distance', 100.0)
        if enemy_distance < 5.0:
            reward += 5.0  # Close range bonus
        elif enemy_distance > 10.0:
            reward -= 2.0  # Penalty for being far
        
        # Combat engagement reward
        if next_state.get('in_combat', False):
            reward += 1.0  # Small bonus for being in combat
        
        # Retreat penalty (aggressive archetype shouldn't retreat)
        if action == 2:  # Assuming action 2 is RETREAT
            reward -= 10.0
        
        return float(reward)
    
    @staticmethod
    def defensive_reward(
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Defensive archetype reward
        
        Priorities:
        - Survival time (+++)
        - Damage blocked/mitigated (++++)
        - Allies protected (++++)
        - HP maintenance (++)
        - Low damage taken (+)
        
        Playstyle: Tank, protect allies, absorb damage
        """
        reward = 0.0
        
        # Survival time (longer is better)
        survival_time = next_state.get('survival_time', 0.0)
        reward += survival_time * 1.0  # Linear scaling with time
        
        # Damage blocked/mitigated
        damage_blocked = next_state.get('damage_blocked', 0.0)
        reward += damage_blocked * 1.5  # High value on damage mitigation
        
        # Allies protected (took damage meant for allies)
        allies_protected = next_state.get('allies_protected', 0) - state.get('allies_protected', 0)
        reward += allies_protected * 20.0  # Big reward for protecting
        
        # HP maintenance (staying alive is key)
        hp_ratio = next_state.get('hp_ratio', 1.0)
        if hp_ratio > 0.7:
            reward += 5.0  # Bonus for high HP
        elif hp_ratio < 0.3:
            reward -= 5.0  # Penalty for low HP (should have retreated/healed)
        
        # Survival bonus (if episode ended with monster alive)
        if done and not next_state.get('died', False):
            reward += 100.0  # Big survival bonus
        
        # Death penalty (defensive monsters should avoid death)
        if done and next_state.get('died', False):
            reward -= 80.0  # Heavy death penalty
        
        # Positioning (defensive monsters should position between enemies and allies)
        if next_state.get('protecting_ally', False):
            reward += 3.0
        
        # Taking damage for team (good for tanks)
        damage_taken = next_state.get('damage_taken', 0.0)
        if next_state.get('allies_nearby', 0) > 0:
            # Taking damage while protecting is good
            reward += damage_taken * 0.3
        else:
            # Taking damage alone is bad
            reward -= damage_taken * 0.5
        
        return float(reward)
    
    @staticmethod
    def support_reward(
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Support archetype reward
        
        Priorities:
        - Healing done (+++++)
        - Buffs applied (++++)
        - Ally survival (+++)
        - Low personal combat (-)
        - Positioning near allies (+++)
        
        Playstyle: Heal, buff, keep allies alive
        """
        reward = 0.0
        
        # Healing done
        healing_done = next_state.get('healing_done', 0.0)
        reward += healing_done * 2.0  # Very high value on healing
        
        # Effective healing (heal to low HP allies)
        effective_healing = next_state.get('effective_healing', 0.0)  # Healing to <30% HP allies
        reward += effective_healing * 4.0  # Even higher for critical heals
        
        # Buffs applied
        buffs_applied = next_state.get('buffs_applied', 0) - state.get('buffs_applied', 0)
        reward += buffs_applied * 15.0  # High reward for buffing
        
        # Debuffs on enemies
        debuffs_applied = next_state.get('debuffs_applied', 0) - state.get('debuffs_applied', 0)
        reward += debuffs_applied * 10.0
        
        # Ally survival (did any allies die?)
        allies_died = next_state.get('allies_died', 0)
        reward -= allies_died * 50.0  # Heavy penalty if allies die
        
        # Ally count nearby (support should stay near allies)
        allies_nearby = next_state.get('allies_nearby', 0)
        if allies_nearby >= 2:
            reward += 5.0  # Bonus for being with team
        elif allies_nearby == 0:
            reward -= 5.0  # Penalty for being alone
        
        # Personal combat (support shouldn't be fighting directly)
        damage_dealt = next_state.get('damage_dealt', 0.0)
        if damage_dealt > 0:
            reward -= 2.0  # Small penalty for attacking (should be supporting)
        
        # Self-preservation
        hp_ratio = next_state.get('hp_ratio', 1.0)
        if hp_ratio < 0.3:
            reward -= 10.0  # Support dying is very bad for team
        
        # Survival bonus
        if done and not next_state.get('died', False):
            reward += 80.0
        
        # Death penalty
        if done and next_state.get('died', False):
            reward -= 100.0  # Very bad if support dies
        
        # Mana management (should conserve mana for important heals)
        sp_ratio = next_state.get('sp_ratio', 1.0)
        if sp_ratio < 0.2 and allies_nearby > 0:
            reward -= 5.0  # Penalty for running out of mana when allies need support
        
        return float(reward)
    
    @staticmethod
    def mage_reward(
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Mage archetype reward
        
        Priorities:
        - Spell damage (+++++)
        - Mana efficiency (++++)
        - Kills with spells (+++)
        - Safe distance from enemies (++++)
        - Combo execution (+++)
        
        Playstyle: Burst damage from distance, kite, mana management
        """
        reward = 0.0
        
        # Spell damage (very high priority)
        spell_damage = next_state.get('spell_damage', 0.0)
        reward += spell_damage * 3.0  # 3x weight on spell damage
        
        # Kills (especially spell kills)
        kills = next_state.get('kills', 0) - state.get('kills', 0)
        spell_kills = next_state.get('spell_kills', 0) - state.get('spell_kills', 0)
        reward += kills * 80.0
        reward += spell_kills * 40.0  # Bonus for spell kills
        
        # Distance from enemies (mages should kite)
        enemy_distance = next_state.get('enemy_distance', 100.0)
        if 5.0 <= enemy_distance <= 10.0:
            reward += 10.0  # Optimal kiting distance
        elif enemy_distance < 3.0:
            reward -= 15.0  # Too close, danger!
        elif enemy_distance > 15.0:
            reward -= 5.0  # Too far, can't hit
        
        # Mana efficiency (damage per mana spent)
        mana_spent = state.get('sp_ratio', 1.0) - next_state.get('sp_ratio', 1.0)
        if mana_spent > 0:
            mana_efficiency = spell_damage / (mana_spent + 0.01)
            reward += mana_efficiency * 0.5
        
        # Mana management (don't run out)
        sp_ratio = next_state.get('sp_ratio', 1.0)
        if sp_ratio < 0.1:
            reward -= 20.0  # Heavy penalty for running out of mana
        elif sp_ratio > 0.5:
            reward += 2.0  # Small bonus for good mana management
        
        # Combo execution (casting spells in sequence)
        combo_count = next_state.get('combo_count', 0) - state.get('combo_count', 0)
        reward += combo_count * 30.0  # Big reward for combos
        
        # Survival (mages are fragile)
        if done and not next_state.get('died', False):
            reward += 120.0  # Big survival bonus
        
        if done and next_state.get('died', False):
            reward -= 60.0  # Death penalty
        
        # Damage taken (should minimize)
        damage_taken = next_state.get('damage_taken', 0.0)
        reward -= damage_taken * 1.0  # Mages can't afford to take damage
        
        # Safe positioning
        if next_state.get('behind_allies', False):
            reward += 3.0  # Bonus for staying behind frontline
        
        return float(reward)
    
    @staticmethod
    def tank_reward(
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Tank archetype reward
        
        Priorities:
        - Aggro control (maintaining enemy attention) (+++++)
        - Damage absorbed (++++)
        - Ally survival (+++)
        - Frontline positioning (++++)
        - Survival (+++)
        
        Playstyle: Front-line anchor, crowd control, protect team
        """
        reward = 0.0
        
        # Aggro count (how many enemies targeting this tank)
        aggro_count = next_state.get('aggro_count', 0)
        reward += aggro_count * 15.0  # High reward for holding aggro
        
        # Damage absorbed (total damage taken while allies are nearby)
        damage_absorbed = next_state.get('damage_absorbed_for_team', 0.0)
        reward += damage_absorbed * 2.0  # Reward for tanking
        
        # Ally survival
        allies_alive = next_state.get('allies_alive', 0)
        allies_died = state.get('allies_alive', 0) - allies_alive
        reward -= allies_died * 80.0  # Heavy penalty if allies die while tanking
        
        # Frontline positioning (should be in front)
        if next_state.get('frontline_position', False):
            reward += 10.0
        else:
            reward -= 5.0  # Tanks should be in front
        
        # HP management (tanks need to stay alive while taking damage)
        hp_ratio = next_state.get('hp_ratio', 1.0)
        if hp_ratio > 0.5:
            reward += 5.0  # Good HP management
        elif hp_ratio < 0.2:
            reward -= 15.0  # Critical HP, should have mitigated/healed
        
        # Crowd control effects applied
        cc_applied = next_state.get('cc_applied', 0) - state.get('cc_applied', 0)
        reward += cc_applied * 25.0  # High value on crowd control
        
        # Taunt/provoke usage
        taunts_used = next_state.get('taunts_used', 0) - state.get('taunts_used', 0)
        reward += taunts_used * 20.0  # Reward for aggro management
        
        # Damage mitigation skills used
        mitigation_skills = next_state.get('mitigation_skills_used', 0) - state.get('mitigation_skills_used', 0)
        reward += mitigation_skills * 15.0
        
        # Survival (tanks must survive to protect)
        if done and not next_state.get('died', False):
            reward += 150.0  # Very high survival bonus
        
        if done and next_state.get('died', False):
            # Death is acceptable if team survived
            if allies_alive > 0:
                reward -= 30.0  # Lesser penalty if protected team
            else:
                reward -= 100.0  # Heavy penalty if team also died
        
        # Enemies engaged (tanks should engage multiple enemies)
        enemies_engaged = next_state.get('enemies_engaged', 0)
        reward += enemies_engaged * 5.0
        
        return float(reward)
    
    @staticmethod
    def ranged_reward(
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Ranged archetype reward
        
        Priorities:
        - Sustained damage over time (++++)
        - Distance maintenance (+++++)
        - Kiting effectiveness (++++)
        - Positioning (++++)
        - Minimal damage taken (+++)
        
        Playstyle: Sustained DPS from safety, mobility, kiting
        """
        reward = 0.0
        
        # Sustained damage (consistent damage output)
        damage_dealt = next_state.get('damage_dealt', 0.0)
        reward += damage_dealt * 1.5  # Good weight on damage
        
        # Kills
        kills = next_state.get('kills', 0) - state.get('kills', 0)
        reward += kills * 90.0
        
        # Distance maintenance (optimal range 7-12 cells)
        enemy_distance = next_state.get('enemy_distance', 100.0)
        if 7.0 <= enemy_distance <= 12.0:
            reward += 15.0  # Perfect kiting distance
        elif 5.0 <= enemy_distance < 7.0:
            reward += 5.0  # Acceptable
        elif enemy_distance < 5.0:
            reward -= 20.0  # Too close, danger
        elif enemy_distance > 15.0:
            reward -= 5.0  # Too far, inefficient
        
        # Kiting effectiveness (maintaining distance while attacking)
        if next_state.get('kiting_active', False):
            reward += 8.0
        
        # Movement efficiency (shouldn't stand still too long)
        position_changed = (
            state.get('position_x', 0) != next_state.get('position_x', 0) or
            state.get('position_y', 0) != next_state.get('position_y', 0)
        )
        if position_changed and next_state.get('in_combat', False):
            reward += 2.0  # Mobility in combat
        
        # Damage taken (ranged should minimize)
        damage_taken = next_state.get('damage_taken', 0.0)
        reward -= damage_taken * 1.2  # Heavy penalty for taking damage
        
        # Perfect kiting (dealing damage while taking none)
        if damage_dealt > 0 and damage_taken == 0:
            reward += 10.0  # Bonus for perfect kiting
        
        # Survival
        if done and not next_state.get('died', False):
            reward += 110.0
        
        if done and next_state.get('died', False):
            reward -= 70.0
        
        # Attack rate (ranged should attack frequently)
        attacks_made = next_state.get('attacks_made', 0) - state.get('attacks_made', 0)
        reward += attacks_made * 2.0
        
        # Repositioning after attack (proper kiting)
        if attacks_made > 0 and position_changed:
            reward += 5.0  # Reward for hit-and-run
        
        return float(reward)
    
    @staticmethod
    def get_reward(
        archetype: str,
        state: Dict[str, Any],
        action: int,
        next_state: Dict[str, Any],
        done: bool,
        info: Optional[Dict[str, Any]] = None
    ) -> float:
        """
        Get reward for any archetype
        
        Args:
            archetype: Monster archetype
            state: Current state
            action: Action taken
            next_state: Resulting state
            done: Episode done flag
            info: Additional information
        
        Returns:
            Shaped reward
        """
        reward_functions = {
            'aggressive': ArchetypeRewards.aggressive_reward,
            'defensive': ArchetypeRewards.defensive_reward,
            'support': ArchetypeRewards.support_reward,
            'mage': ArchetypeRewards.mage_reward,
            'tank': ArchetypeRewards.tank_reward,
            'ranged': ArchetypeRewards.ranged_reward
        }
        
        if archetype not in reward_functions:
            logger.warning(f"Unknown archetype: {archetype}, using default reward")
            return next_state.get('reward', 0.0)
        
        reward_fn = reward_functions[archetype]
        shaped_reward = reward_fn(state, action, next_state, done, info)
        
        return shaped_reward


def compute_intrinsic_reward(
    state: np.ndarray,
    next_state: np.ndarray,
    state_history: Optional[np.ndarray] = None
) -> float:
    """
    Compute intrinsic reward for exploration
    
    Methods:
    - Novelty bonus (visiting new states)
    - Curiosity reward (prediction error)
    
    Args:
        state: Current state vector
        next_state: Next state vector
        state_history: Optional history of visited states
    
    Returns:
        Intrinsic reward
    """
    intrinsic = 0.0
    
    # State change magnitude (encourage exploration)
    state_change = np.linalg.norm(next_state - state)
    intrinsic += state_change * 0.1
    
    # Novelty bonus (if state history provided)
    if state_history is not None and len(state_history) > 0:
        # Compute distance to nearest visited state
        distances = np.linalg.norm(state_history - next_state, axis=1)
        min_distance = np.min(distances)
        
        # Bonus for visiting novel states (far from previous states)
        if min_distance > 1.0:
            intrinsic += min(min_distance * 2.0, 10.0)  # Cap at 10
    
    return float(intrinsic)


def compute_team_reward(
    pack_states: Dict[str, Dict[str, Any]],
    pack_actions: Dict[str, int],
    pack_next_states: Dict[str, Dict[str, Any]],
    pack_dones: Dict[str, bool]
) -> float:
    """
    Compute team-level reward for multi-agent coordination
    
    Rewards:
    - Pack survival rate
    - Coordinated kills
    - Formation quality
    - Support effectiveness
    
    Args:
        pack_states: Dict of monster_id -> state
        pack_actions: Dict of monster_id -> action
        pack_next_states: Dict of monster_id -> next_state
        pack_dones: Dict of monster_id -> done
    
    Returns:
        Team reward (shared by all pack members)
    """
    team_reward = 0.0
    
    # Pack survival rate
    pack_size = len(pack_states)
    survivors = sum(1 for mid, done in pack_dones.items() if not done or not pack_next_states[mid].get('died', False))
    survival_rate = survivors / pack_size if pack_size > 0 else 0
    team_reward += survival_rate * 10.0
    
    # Coordinated kills (kills with 2+ pack members attacking same target)
    total_kills = sum(
        pack_next_states[mid].get('kills', 0) - pack_states[mid].get('kills', 0)
        for mid in pack_states
    )
    coordinated_kills = sum(
        pack_next_states[mid].get('coordinated_kills', 0)
        for mid in pack_states
    ) / max(pack_size, 1)  # Average per member
    
    team_reward += total_kills * 20.0  # Base kill reward
    team_reward += coordinated_kills * 30.0  # Bonus for coordination
    
    # Formation quality (pack staying cohesive)
    positions = np.array([
        [pack_next_states[mid].get('position_x', 0), pack_next_states[mid].get('position_y', 0)]
        for mid in pack_states
    ])
    
    if len(positions) > 1:
        # Compute position variance (lower = tighter formation = better)
        position_variance = np.var(positions, axis=0).sum()
        formation_quality = 1.0 / (1.0 + position_variance / 100.0)
        team_reward += formation_quality * 3.0
    
    # Support effectiveness (low HP allies received help)
    low_hp_allies = sum(
        1 for mid in pack_states
        if pack_next_states[mid].get('hp_ratio', 1.0) < 0.3
    )
    healing_received = sum(
        pack_next_states[mid].get('healing_received', 0.0)
        for mid in pack_states
        if pack_states[mid].get('hp_ratio', 1.0) < 0.3
    )
    
    if low_hp_allies > 0:
        support_score = healing_received / low_hp_allies
        team_reward += support_score * 2.0
    
    # Focus fire bonus (attacking same target)
    targets = [pack_next_states[mid].get('target_id', -1) for mid in pack_states]
    unique_targets = len(set(t for t in targets if t != -1))
    
    if unique_targets == 1 and targets[0] != -1:
        team_reward += 15.0  # All attacking same target
    elif unique_targets <= 2:
        team_reward += 5.0  # Mostly coordinated
    
    # Pack leader bonus (if leader is alive and commanding)
    leader_alive = False
    for mid in pack_states:
        if pack_next_states[mid].get('is_leader', False) and not pack_dones.get(mid, False):
            leader_alive = True
            team_reward += 5.0
            break
    
    if not leader_alive and pack_size > 1:
        team_reward -= 10.0  # Penalty if leader died
    
    return float(team_reward)


def combine_rewards(
    individual_reward: float,
    team_reward: float,
    archetype: str,
    individual_weight: float = 0.6,
    team_weight: float = 0.4
) -> float:
    """
    Combine individual and team rewards
    
    Args:
        individual_reward: Individual monster reward
        team_reward: Shared team reward
        archetype: Monster archetype
        individual_weight: Weight for individual reward
        team_weight: Weight for team reward
    
    Returns:
        Combined reward
    """
    # Archetype-specific weights
    archetype_weights = {
        'aggressive': (0.7, 0.3),  # More individual
        'defensive': (0.4, 0.6),   # More team-oriented
        'support': (0.3, 0.7),     # Very team-oriented
        'mage': (0.6, 0.4),        # Balanced
        'tank': (0.3, 0.7),        # Very team-oriented
        'ranged': (0.6, 0.4)       # Balanced
    }
    
    if archetype in archetype_weights:
        individual_weight, team_weight = archetype_weights[archetype]
    
    combined = individual_weight * individual_reward + team_weight * team_reward
    
    return float(combined)


def shape_sparse_rewards(
    rewards: np.ndarray,
    gamma: float = 0.99,
    method: str = 'potential'
) -> np.ndarray:
    """
    Shape sparse rewards for better learning
    
    Methods:
    - 'potential': Potential-based reward shaping
    - 'hindsight': Hindsight experience replay style
    - 'curiosity': Curiosity-driven rewards
    
    Args:
        rewards: Array of sparse rewards
        gamma: Discount factor
        method: Shaping method
    
    Returns:
        Shaped rewards
    """
    if method == 'potential':
        # Potential-based shaping: F(s, s') = γΦ(s') - Φ(s)
        # For sparse rewards, use discounted future reward as potential
        shaped = np.zeros_like(rewards)
        
        for t in range(len(rewards)):
            # Find next non-zero reward
            future_reward = 0.0
            discount = 1.0
            
            for i in range(t, len(rewards)):
                if rewards[i] != 0:
                    future_reward = rewards[i] * discount
                    break
                discount *= gamma
            
            # Potential difference
            if t < len(rewards) - 1:
                current_potential = future_reward
                next_potential = future_reward / gamma
                shaped[t] = gamma * next_potential - current_potential
            else:
                shaped[t] = rewards[t]
        
        return shaped
    
    else:
        return rewards


class CurriculumScheduler:
    """
    Curriculum learning scheduler
    
    Progressively increases task difficulty during training
    """
    
    def __init__(
        self,
        num_levels: int = 5,
        level_thresholds: Optional[List[float]] = None
    ):
        """
        Initialize curriculum scheduler
        
        Args:
            num_levels: Number of difficulty levels
            level_thresholds: Performance thresholds to advance levels
        """
        self.num_levels = num_levels
        self.current_level = 0
        
        if level_thresholds is None:
            # Default thresholds (based on average episode reward)
            level_thresholds = [50.0, 100.0, 200.0, 400.0, 800.0][:num_levels]
        
        self.level_thresholds = level_thresholds
        self.recent_performance = []
        
        logger.info(f"CurriculumScheduler initialized: {num_levels} levels")
    
    def update(self, episode_reward: float) -> int:
        """
        Update curriculum based on performance
        
        Args:
            episode_reward: Recent episode reward
        
        Returns:
            Current difficulty level
        """
        self.recent_performance.append(episode_reward)
        
        # Keep last 100 episodes
        if len(self.recent_performance) > 100:
            self.recent_performance.pop(0)
        
        # Check if ready to advance
        if len(self.recent_performance) >= 50:  # Need enough data
            avg_performance = np.mean(self.recent_performance[-50:])
            
            # Try to advance to next level
            for level in range(self.current_level + 1, self.num_levels):
                if avg_performance >= self.level_thresholds[level - 1]:
                    self.current_level = level
                    logger.info(f"Curriculum advanced to level {level} (avg reward: {avg_performance:.2f})")
                else:
                    break
        
        return self.current_level
    
    def get_difficulty_params(self) -> Dict[str, Any]:
        """
        Get current difficulty parameters
        
        Returns:
            Dictionary with difficulty settings
        """
        # Example: enemy count, enemy strength, time limit
        difficulties = [
            {'enemy_count': 1, 'enemy_strength': 0.5, 'time_limit': 60},
            {'enemy_count': 2, 'enemy_strength': 0.7, 'time_limit': 45},
            {'enemy_count': 3, 'enemy_strength': 0.9, 'time_limit': 30},
            {'enemy_count': 4, 'enemy_strength': 1.2, 'time_limit': 20},
            {'enemy_count': 5, 'enemy_strength': 1.5, 'time_limit': 15}
        ]
        
        return difficulties[min(self.current_level, len(difficulties) - 1)]


# Reward debugging utilities

def analyze_reward_distribution(
    rewards: np.ndarray,
    archetype: str
) -> Dict[str, float]:
    """
    Analyze reward distribution for debugging
    
    Args:
        rewards: Array of rewards
        archetype: Monster archetype
    
    Returns:
        Dictionary with statistics
    """
    stats = {
        'mean': float(np.mean(rewards)),
        'std': float(np.std(rewards)),
        'min': float(np.min(rewards)),
        'max': float(np.max(rewards)),
        'median': float(np.median(rewards)),
        'positive_ratio': float(np.sum(rewards > 0) / len(rewards)),
        'zero_ratio': float(np.sum(rewards == 0) / len(rewards)),
        'negative_ratio': float(np.sum(rewards < 0) / len(rewards))
    }
    
    logger.info(f"Reward distribution for {archetype}:")
    logger.info(f"  Mean: {stats['mean']:.2f}, Std: {stats['std']:.2f}")
    logger.info(f"  Range: [{stats['min']:.2f}, {stats['max']:.2f}]")
    logger.info(f"  Positive: {stats['positive_ratio']:.1%}, Zero: {stats['zero_ratio']:.1%}, Negative: {stats['negative_ratio']:.1%}")
    
    return stats
