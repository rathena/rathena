#!/usr/bin/env python3
"""
ML Monster AI - Initial Data Collection
Enhanced ML Monster AI System v2.0

Collect initial training data from traditional AI monsters to bootstrap training

This script:
1. Queries PostgreSQL ml_experience_replay table
2. Generates synthetic data if no real data exists
3. Populates replay buffers for each archetype
4. Provides ~10K initial samples per archetype

Usage:
    python collect_initial_data.py --archetype aggressive --samples 10000
    python collect_initial_data.py --all --samples-per-archetype 10000
"""

import sys
import os
import argparse
import logging
from pathlib import Path
import asyncio
import asyncpg
import numpy as np
from typing import Dict, List, Optional
import yaml

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from data.replay_buffer import PostgreSQLReplayBuffer, create_replay_buffer
from data.preprocessor import StatePreprocessor, DataValidator
from models.model_factory import ModelFactory

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class DataCollector:
    """
    Collect initial training data
    
    Methods:
    1. Query from PostgreSQL (if traditional AI has logged data)
    2. Generate synthetic data (bootstrap)
    3. Record human demonstrations (future)
    """
    
    def __init__(self, db_config: Optional[Dict[str, str]] = None):
        """
        Initialize data collector
        
        Args:
            db_config: Database configuration
        """
        self.db_config = db_config or self.get_default_db_config()
        self.db_conn = None
        
        logger.info("DataCollector initialized")
    
    @staticmethod
    def get_default_db_config() -> Dict[str, str]:
        """Get default database configuration"""
        return {
            'host': 'localhost',
            'port': 5432,
            'database': 'ragnarok_ml',
            'user': 'ml_user',
            'password': os.getenv('POSTGRES_PASSWORD', '')
        }
    
    async def connect_db(self):
        """Connect to PostgreSQL"""
        try:
            self.db_conn = await asyncpg.connect(
                host=self.db_config['host'],
                port=self.db_config['port'],
                database=self.db_config['database'],
                user=self.db_config['user'],
                password=self.db_config['password']
            )
            
            logger.info("PostgreSQL connection established")
            return True
        
        except Exception as e:
            logger.error(f"Failed to connect to PostgreSQL: {e}")
            return False
    
    async def query_existing_data(
        self,
        archetype: str,
        limit: int = 10000
    ) -> List[Dict[str, any]]:
        """
        Query existing data from PostgreSQL
        
        Args:
            archetype: Monster archetype
            limit: Maximum number of records
        
        Returns:
            List of experience records
        """
        if self.db_conn is None:
            await self.connect_db()
        
        query = """
        SELECT 
            monster_id,
            episode_id,
            state_vector,
            action_type,
            reward,
            next_state_vector,
            done,
            created_at
        FROM ml_experience_replay
        WHERE archetype = $1
        ORDER BY created_at DESC
        LIMIT $2
        """
        
        try:
            records = await self.db_conn.fetch(query, archetype, limit)
            
            logger.info(f"Retrieved {len(records)} existing experiences for {archetype}")
            
            # Convert to experience format
            experiences = []
            for record in records:
                exp = {
                    'state': np.frombuffer(record['state_vector'], dtype=np.float32),
                    'action': record['action_type'],
                    'reward': record['reward'],
                    'next_state': np.frombuffer(record['next_state_vector'], dtype=np.float32),
                    'done': record['done']
                }
                experiences.append(exp)
            
            return experiences
        
        except Exception as e:
            logger.error(f"Failed to query data: {e}")
            return []
    
    def generate_synthetic_data(
        self,
        archetype: str,
        num_samples: int = 10000,
        state_dim: int = 64,
        action_dim: int = 10
    ) -> List[Dict[str, any]]:
        """
        Generate synthetic training data for bootstrapping
        
        This creates reasonable initial data based on archetype characteristics
        
        Args:
            archetype: Monster archetype
            num_samples: Number of samples to generate
            state_dim: State dimension
            action_dim: Action dimension
        
        Returns:
            List of synthetic experiences
        """
        logger.info(f"Generating {num_samples} synthetic samples for {archetype}...")
        
        experiences = []
        
        # Archetype-specific state distributions
        archetype_params = {
            'aggressive': {
                'hp_mean': 0.7, 'hp_std': 0.2,
                'distance_mean': 3.0, 'distance_std': 2.0,
                'damage_mean': 50.0, 'damage_std': 20.0,
                'preferred_actions': [0, 1, 4]  # ATTACK, CHASE, SKILL_HEAVY
            },
            'defensive': {
                'hp_mean': 0.8, 'hp_std': 0.15,
                'distance_mean': 2.0, 'distance_std': 1.0,
                'damage_mean': 20.0, 'damage_std': 10.0,
                'preferred_actions': [1, 2, 6]  # DEFEND, RETREAT, REPOSITION
            },
            'support': {
                'hp_mean': 0.6, 'hp_std': 0.2,
                'distance_mean': 8.0, 'distance_std': 3.0,
                'damage_mean': 10.0, 'damage_std': 5.0,
                'preferred_actions': [6, 7]  # REPOSITION, WAIT (use skills)
            },
            'mage': {
                'hp_mean': 0.6, 'hp_std': 0.25,
                'distance_mean': 10.0, 'distance_std': 3.0,
                'damage_mean': 80.0, 'damage_std': 30.0,
                'preferred_actions': [0, 4, 5, 6]  # ATTACK, SKILL_HEAVY, SKILL_LIGHT, REPOSITION
            },
            'tank': {
                'hp_mean': 0.9, 'hp_std': 0.1,
                'distance_mean': 1.0, 'distance_std': 0.5,
                'damage_mean': 30.0, 'damage_std': 15.0,
                'preferred_actions': [0, 1, 6]  # ATTACK, DEFEND, REPOSITION
            },
            'ranged': {
                'hp_mean': 0.65, 'hp_std': 0.2,
                'distance_mean': 9.0, 'distance_std': 2.0,
                'damage_mean': 40.0, 'damage_std': 15.0,
                'preferred_actions': [0, 3, 6]  # ATTACK, CHASE (kite), REPOSITION
            }
        }
        
        params = archetype_params.get(archetype, archetype_params['aggressive'])
        
        for i in range(num_samples):
            # Generate state
            state = self.generate_state_vector(state_dim, params)
            
            # Sample action (biased towards archetype preferences)
            if np.random.random() < 0.7:  # 70% archetype-appropriate actions
                action = np.random.choice(params['preferred_actions'])
            else:
                action = np.random.randint(0, action_dim)
            
            # Generate next state (slightly modified)
            next_state = self.generate_next_state(state, action, params)
            
            # Generate reward
            reward = self.generate_reward(state, action, next_state, archetype)
            
            # Episode done (10% chance)
            done = np.random.random() < 0.1
            
            experiences.append({
                'state': state,
                'action': action,
                'reward': reward,
                'next_state': next_state,
                'done': done
            })
        
        logger.info(f"Generated {len(experiences)} synthetic experiences for {archetype}")
        
        return experiences
    
    @staticmethod
    def generate_state_vector(
        state_dim: int,
        params: Dict[str, float]
    ) -> np.ndarray:
        """
        Generate realistic state vector
        
        State vector (64 dims):
        [0]: HP ratio
        [1]: SP ratio
        [2-3]: Position (normalized)
        [4]: Enemy distance
        [5-9]: Enemy info
        [10-14]: Ally info
        [15-63]: Additional features
        """
        state = np.random.randn(state_dim).astype(np.float32) * 0.1  # Small noise
        
        # HP ratio
        state[0] = np.clip(np.random.normal(params['hp_mean'], params['hp_std']), 0.1, 1.0)
        
        # SP ratio
        state[1] = np.clip(np.random.normal(0.7, 0.2), 0.0, 1.0)
        
        # Position (normalized to [0, 1])
        state[2] = np.random.random()
        state[3] = np.random.random()
        
        # Enemy distance
        state[4] = np.clip(np.random.normal(params['distance_mean'], params['distance_std']), 0.5, 20.0) / 20.0
        
        # Enemy count
        state[5] = np.random.randint(0, 5) / 5.0
        
        # Ally count
        state[10] = np.random.randint(0, 5) / 5.0
        
        return state
    
    @staticmethod
    def generate_next_state(
        state: np.ndarray,
        action: int,
        params: Dict[str, float]
    ) -> np.ndarray:
        """Generate next state based on action"""
        next_state = state.copy()
        
        # Modify based on action
        if action == 0:  # ATTACK
            next_state[0] -= 0.05  # Lose some HP
            next_state[4] -= 0.02  # Move slightly closer
        
        elif action == 1:  # DEFEND
            next_state[0] += 0.02  # Regenerate
        
        elif action == 2:  # RETREAT
            next_state[4] += 0.05  # Move away
        
        elif action == 3:  # CHASE
            next_state[4] -= 0.05  # Move closer
        
        # Add small noise
        next_state += np.random.randn(len(state)).astype(np.float32) * 0.01
        
        # Clip to valid ranges
        next_state[0] = np.clip(next_state[0], 0.0, 1.0)  # HP
        next_state[1] = np.clip(next_state[1], 0.0, 1.0)  # SP
        next_state[4] = np.clip(next_state[4], 0.0, 1.0)  # Distance
        
        return next_state
    
    @staticmethod
    def generate_reward(
        state: np.ndarray,
        action: int,
        next_state: np.ndarray,
        archetype: str
    ) -> float:
        """Generate reward for synthetic transition"""
        # Simple reward based on HP change and combat
        hp_change = next_state[0] - state[0]
        distance_change = next_state[4] - state[4]
        
        reward = 0.0
        
        if archetype == 'aggressive':
            reward = -hp_change * 10.0  # Losing HP from combat is OK
            reward -= distance_change * 5.0  # Prefer getting closer
        
        elif archetype == 'defensive':
            reward = hp_change * 20.0  # Preserve HP
            reward += min(distance_change, 0) * 5.0  # Retreat is OK
        
        elif archetype == 'support':
            reward = hp_change * 15.0  # Stay alive
        
        elif archetype == 'mage':
            reward = -hp_change * 5.0
            reward += max(distance_change, 0) * 10.0  # Prefer distance
        
        elif archetype == 'tank':
            reward = -hp_change * 5.0  # Taking damage is OK
            reward -= distance_change * 10.0  # Stay close
        
        elif archetype == 'ranged':
            reward = -hp_change * 10.0
            # Prefer medium distance
            optimal_distance = 0.5  # Normalized
            distance_penalty = abs(next_state[4] - optimal_distance)
            reward -= distance_penalty * 10.0
        
        # Add small noise
        reward += np.random.randn() * 2.0
        
        return float(reward)
    
    async def collect_and_store(
        self,
        archetype: str,
        num_samples: int = 10000,
        use_existing: bool = True
    ) -> int:
        """
        Collect and store data for archetype
        
        Args:
            archetype: Monster archetype
            num_samples: Number of samples to collect/generate
            use_existing: Try to use existing data first
        
        Returns:
            Number of samples collected
        """
        experiences = []
        
        # Try to get existing data
        if use_existing:
            existing = await self.query_existing_data(archetype, num_samples)
            experiences.extend(existing)
            logger.info(f"Found {len(existing)} existing experiences")
        
        # Generate synthetic data if needed
        remaining = num_samples - len(experiences)
        if remaining > 0:
            logger.info(f"Generating {remaining} synthetic experiences...")
            synthetic = self.generate_synthetic_data(archetype, remaining)
            experiences.extend(synthetic)
        
        # Save to replay buffer
        buffer = create_replay_buffer(
            buffer_type='prioritized',
            archetype=archetype,
            capacity=num_samples * 2
        )
        
        # Add all experiences
        for exp in experiences:
            buffer.add(
                exp['state'],
                exp['action'],
                exp['reward'],
                exp['next_state'],
                exp['done']
            )
        
        # Save buffer to disk
        save_dir = Path('/opt/ml_monster_ai/data/replay_buffer')
        save_dir.mkdir(parents=True, exist_ok=True)
        
        save_path = save_dir / f"{archetype}_initial_data.npz"
        buffer.save(str(save_path))
        
        logger.info(f"Saved {len(experiences)} experiences to {save_path}")
        
        return len(experiences)
    
    async def close(self):
        """Close database connection"""
        if self.db_conn:
            await self.db_conn.close()
            logger.info("Database connection closed")


async def collect_all_data(
    archetypes: List[str],
    samples_per_archetype: int = 10000
) -> Dict[str, int]:
    """
    Collect data for all archetypes
    
    Args:
        archetypes: List of archetypes
        samples_per_archetype: Samples per archetype
    
    Returns:
        Dictionary mapping archetype -> sample count
    """
    collector = DataCollector()
    
    try:
        results = {}
        
        for archetype in archetypes:
            logger.info(f"\n{'='*60}")
            logger.info(f"Collecting data for {archetype}")
            logger.info(f"{'='*60}\n")
            
            count = await collector.collect_and_store(archetype, samples_per_archetype)
            results[archetype] = count
        
        return results
    
    finally:
        await collector.close()


def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Collect initial training data')
    
    parser.add_argument(
        '--archetype',
        type=str,
        choices=ModelFactory.ARCHETYPES,
        help='Specific archetype to collect data for'
    )
    
    parser.add_argument(
        '--all',
        action='store_true',
        help='Collect data for all archetypes'
    )
    
    parser.add_argument(
        '--samples',
        type=int,
        default=10000,
        help='Number of samples to collect (default: 10000)'
    )
    
    parser.add_argument(
        '--samples-per-archetype',
        type=int,
        default=10000,
        help='Samples per archetype when using --all'
    )
    
    parser.add_argument(
        '--use-existing',
        action='store_true',
        default=True,
        help='Use existing database records if available'
    )
    
    parser.add_argument(
        '--synthetic-only',
        action='store_true',
        help='Generate synthetic data only (ignore database)'
    )
    
    return parser.parse_args()


async def main_async(args):
    """Async main function"""
    if args.all:
        archetypes = ModelFactory.ARCHETYPES
        samples = args.samples_per_archetype
    elif args.archetype:
        archetypes = [args.archetype]
        samples = args.samples
    else:
        logger.error("Must specify --archetype or --all")
        return 1
    
    logger.info(f"{'='*60}")
    logger.info(f"INITIAL DATA COLLECTION")
    logger.info(f"{'='*60}")
    logger.info(f"Archetypes: {archetypes}")
    logger.info(f"Samples per archetype: {samples}")
    logger.info(f"Use existing data: {not args.synthetic_only}")
    logger.info(f"{'='*60}\n")
    
    # Collect data
    results = await collect_all_data(archetypes, samples)
    
    # Summary
    logger.info(f"\n{'='*60}")
    logger.info(f"DATA COLLECTION SUMMARY")
    logger.info(f"{'='*60}")
    
    total_samples = 0
    for archetype, count in results.items():
        logger.info(f"{archetype}: {count} samples")
        total_samples += count
    
    logger.info(f"\nTotal: {total_samples} samples collected")
    logger.info(f"Data saved to: /opt/ml_monster_ai/data/replay_buffer/")
    logger.info(f"{'='*60}\n")
    
    return 0


def main():
    """Main entry point"""
    args = parse_args()
    return asyncio.run(main_async(args))


if __name__ == '__main__':
    sys.exit(main())
