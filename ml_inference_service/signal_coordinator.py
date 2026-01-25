"""
Signal Coordinator for Pack Communication
==========================================
Manages signal passing between pack members during inference.

Integrates with Apache AGE graph (Schema 6 - SIGNALS edges) for:
- Broadcasting signals from pack leader
- Receiving signals for individual monsters
- Processing and aggregating signals for coordination
- Signal type routing and priority handling

Signal Types:
- ATTACK_TARGET: Coordinate focus fire
- RETREAT: Pack-wide retreat
- FORM_UP: Regroup formation
- DEFEND_LEADER: Protect pack leader
- FLANK_MANEUVER: Coordinated flanking

Architecture Reference: plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md
Integration: Uses graph_manager.py for signal persistence
"""

import asyncio
import logging
import numpy as np
import torch
from typing import Dict, List, Any, Optional, Tuple
from datetime import datetime, timedelta
from collections import defaultdict
import json

logger = logging.getLogger(__name__)


class SignalType:
    """Signal type constants"""
    ATTACK_TARGET = "attack_target"
    RETREAT = "retreat_call"
    FORM_UP = "formation_command"
    DEFEND_LEADER = "defend_leader"
    FLANK_MANEUVER = "flank_maneuver"
    FOCUS_FIRE = "focus_fire"
    HELP_REQUEST = "help_request"
    DANGER_ALERT = "danger_alert"
    REGROUP = "regroup"
    HOLD_POSITION = "hold"
    
    # All valid signal types
    ALL_TYPES = [
        ATTACK_TARGET, RETREAT, FORM_UP, DEFEND_LEADER, FLANK_MANEUVER,
        FOCUS_FIRE, HELP_REQUEST, DANGER_ALERT, REGROUP, HOLD_POSITION
    ]


class SignalCoordinator:
    """
    Manages signal passing between pack members
    
    Features:
    - Integration with Apache AGE graph for signal persistence
    - Signal aggregation into fixed-size vectors (32-dim)
    - Priority-based signal processing
    - Signal expiration and cleanup
    - Broadcast from leader to pack
    - Individual signal reception
    
    Signal Vector Encoding (32 dimensions):
    - Dims 0-7: Target information (priority targets)
    - Dims 8-15: Formation commands (movement/positioning)
    - Dims 16-23: Support requests (urgency levels)
    - Dims 24-31: Strategic directives (retreat, advance, hold)
    """
    
    def __init__(
        self,
        graph_manager,
        signal_dim: int = 32,
        signal_range: int = 15,
        signal_ttl: int = 5
    ):
        """
        Initialize signal coordinator
        
        Args:
            graph_manager: AGEGraphManager instance for graph operations
            signal_dim: Signal vector dimension (must be 32)
            signal_range: Maximum signal range in cells
            signal_ttl: Signal time-to-live in seconds
        """
        self.graph_manager = graph_manager
        self.signal_dim = signal_dim
        self.signal_range = signal_range
        self.signal_ttl = signal_ttl
        
        # Signal processing stats
        self.signals_sent = 0
        self.signals_received = 0
        self.signals_aggregated = 0
        
        # Cache for recent signals (avoid repeated DB queries)
        self.signal_cache: Dict[int, List[Dict[str, Any]]] = {}
        self.cache_ttl = 1.0  # 1 second cache
        self.last_cache_update: Dict[int, float] = {}
        
        logger.info(
            f"SignalCoordinator initialized: dim={signal_dim}, "
            f"range={signal_range}, ttl={signal_ttl}s"
        )
    
    async def broadcast_signal(
        self,
        from_monster_id: int,
        signal_type: str,
        data: Dict[str, Any],
        pack_monsters: Optional[List[int]] = None,
        priority: int = 5
    ) -> int:
        """
        Broadcast signal from monster to pack members
        
        Args:
            from_monster_id: Monster sending signal
            signal_type: Type of signal (from SignalType)
            data: Signal payload data
            pack_monsters: List of pack member IDs (if None, query from graph)
            priority: Signal priority (1=highest, 10=lowest)
        
        Returns:
            Number of monsters signal was sent to
        """
        # Validate signal type
        if signal_type not in SignalType.ALL_TYPES:
            logger.warning(f"Unknown signal type: {signal_type}")
            return 0
        
        # Get pack members if not provided
        if pack_monsters is None:
            pack_monsters = await self._get_pack_members(from_monster_id)
        
        if not pack_monsters:
            logger.debug(f"No pack members found for monster {from_monster_id}")
            return 0
        
        # Send signal to each pack member
        sent_count = 0
        
        for to_monster_id in pack_monsters:
            if to_monster_id == from_monster_id:
                continue  # Don't send to self
            
            try:
                success = await self.graph_manager.send_signal(
                    sender_id=from_monster_id,
                    receiver_id=to_monster_id,
                    signal_type=signal_type,
                    content=data,
                    priority=priority
                )
                
                if success:
                    sent_count += 1
                    
                    # Invalidate cache for receiver
                    if to_monster_id in self.signal_cache:
                        del self.signal_cache[to_monster_id]
            
            except Exception as e:
                logger.error(
                    f"Failed to send signal from {from_monster_id} to {to_monster_id}: {e}"
                )
        
        self.signals_sent += sent_count
        
        logger.debug(
            f"Broadcast signal '{signal_type}' from {from_monster_id} to {sent_count} pack members"
        )
        
        return sent_count
    
    async def receive_signals(
        self,
        monster_id: int,
        use_cache: bool = True
    ) -> List[Dict[str, Any]]:
        """
        Receive all pending signals for monster
        
        Args:
            monster_id: Monster receiving signals
            use_cache: Whether to use cached signals
        
        Returns:
            List of signal dicts ordered by priority
        """
        current_time = asyncio.get_event_loop().time()
        
        # Check cache
        if use_cache and monster_id in self.signal_cache:
            last_update = self.last_cache_update.get(monster_id, 0)
            if current_time - last_update < self.cache_ttl:
                return self.signal_cache[monster_id]
        
        # Query from graph
        try:
            signals = await self.graph_manager.receive_signals(monster_id)
            
            # Update cache
            self.signal_cache[monster_id] = signals
            self.last_cache_update[monster_id] = current_time
            
            self.signals_received += len(signals)
            
            return signals
        
        except Exception as e:
            logger.error(f"Failed to receive signals for monster {monster_id}: {e}")
            return []
    
    async def process_coordination_signals(
        self,
        pack_monsters: List[int]
    ) -> Dict[int, np.ndarray]:
        """
        Process and aggregate signals for all pack members
        
        Args:
            pack_monsters: List of monster IDs in pack
        
        Returns:
            Dict mapping monster_id -> signal_vector (32-dim numpy array)
        """
        signal_vectors = {}
        
        # Get signals for each monster
        for monster_id in pack_monsters:
            signals = await self.receive_signals(monster_id)
            
            # Aggregate into signal vector
            signal_vector = self.aggregate_signals_to_vector(signals)
            
            signal_vectors[monster_id] = signal_vector
        
        self.signals_aggregated += len(pack_monsters)
        
        return signal_vectors
    
    def aggregate_signals_to_vector(
        self,
        signals: List[Dict[str, Any]]
    ) -> np.ndarray:
        """
        Aggregate multiple signals into fixed-size vector
        
        Signal Vector Encoding (32 dimensions):
        - Dims 0-7: Target information (which target to attack)
        - Dims 8-15: Formation commands (where to position)
        - Dims 16-23: Support requests (who needs help)
        - Dims 24-31: Strategic directives (retreat, advance, hold, etc.)
        
        Args:
            signals: List of signal dicts from graph_manager.receive_signals()
        
        Returns:
            signal_vector: (32,) numpy array
        """
        signal_vector = np.zeros(self.signal_dim, dtype=np.float32)
        
        if not signals:
            return signal_vector
        
        for signal in signals:
            signal_type = signal.get('signal_type', 'unknown')
            content = signal.get('content', {})
            priority = signal.get('priority', 5)
            
            # Priority weight (higher priority = more influence)
            priority_weight = 1.0 - (priority - 1) / 9.0  # 1=1.0, 10=0.0
            
            # Encode based on signal type
            if signal_type == SignalType.ATTACK_TARGET or signal_type == SignalType.FOCUS_FIRE:
                # Target information (dims 0-7)
                threat_level = content.get('target_threat', 0.5)
                target_priority = content.get('priority', 0.5)
                
                # Encode as distribution over priority levels
                idx = min(int(threat_level * 7), 7)
                signal_vector[idx] = max(signal_vector[idx], target_priority * priority_weight)
            
            elif signal_type == SignalType.FORM_UP or signal_type == SignalType.FLANK_MANEUVER:
                # Formation commands (dims 8-15)
                formation_map = {
                    'flanking': 8,
                    'defensive_circle': 9,
                    'spread_out': 10,
                    'tight_cluster': 11,
                    'line_formation': 12,
                    'v_formation': 13,
                    'surround': 14
                }
                
                formation_type = content.get('formation_type', 'default')
                idx = formation_map.get(formation_type, 15)
                signal_vector[idx] = priority_weight
                
                # Add position offset if specified
                if 'assigned_position' in content:
                    pos = content['assigned_position']
                    # Normalize position to [-1, 1] range
                    signal_vector[idx] = min(signal_vector[idx] + 0.5, 1.0)
            
            elif signal_type == SignalType.HELP_REQUEST or signal_type == SignalType.DEFEND_LEADER:
                # Support requests (dims 16-23)
                urgency = content.get('urgency', 0.5)
                hp_ratio = content.get('hp_ratio', 0.5)
                
                # Encode urgency level
                idx = 16 + min(int(urgency * 7), 7)
                signal_vector[idx] = max(signal_vector[idx], urgency * priority_weight)
            
            elif signal_type in [SignalType.RETREAT, SignalType.REGROUP, SignalType.HOLD_POSITION]:
                # Strategic directives (dims 24-31)
                directive_map = {
                    SignalType.RETREAT: 24,
                    SignalType.REGROUP: 25,
                    SignalType.HOLD_POSITION: 26,
                    'advance': 27,
                    'scatter': 28,
                    'fallback': 29,
                    'charge': 30
                }
                
                idx = directive_map.get(signal_type, 31)
                signal_vector[idx] = priority_weight
                
                # Add intensity if specified
                intensity = content.get('intensity', 1.0)
                signal_vector[idx] = min(signal_vector[idx] * intensity, 1.0)
            
            elif signal_type == SignalType.DANGER_ALERT:
                # Danger alert - combine with retreat directive
                danger_level = content.get('danger_level', 0.5)
                signal_vector[24] = max(signal_vector[24], danger_level * priority_weight)  # Retreat
        
        return signal_vector
    
    async def send_attack_target_signal(
        self,
        from_monster_id: int,
        target_id: int,
        target_threat: float,
        pack_monsters: Optional[List[int]] = None
    ) -> int:
        """
        Send attack target signal to coordinate focus fire
        
        Args:
            from_monster_id: Monster initiating signal
            target_id: Target to attack
            target_threat: Threat level of target (0.0-1.0)
            pack_monsters: Pack member IDs
        
        Returns:
            Number of pack members notified
        """
        return await self.broadcast_signal(
            from_monster_id=from_monster_id,
            signal_type=SignalType.ATTACK_TARGET,
            data={
                'target_id': target_id,
                'target_threat': target_threat,
                'priority': target_threat,
                'timestamp': datetime.now().isoformat()
            },
            pack_monsters=pack_monsters,
            priority=2  # High priority
        )
    
    async def send_retreat_signal(
        self,
        from_monster_id: int,
        rally_point: Tuple[int, int],
        reason: str,
        pack_monsters: Optional[List[int]] = None
    ) -> int:
        """
        Send retreat signal to pack
        
        Args:
            from_monster_id: Monster initiating retreat
            rally_point: (x, y) coordinates to retreat to
            reason: Reason for retreat
            pack_monsters: Pack member IDs
        
        Returns:
            Number of pack members notified
        """
        return await self.broadcast_signal(
            from_monster_id=from_monster_id,
            signal_type=SignalType.RETREAT,
            data={
                'rally_point': rally_point,
                'reason': reason,
                'intensity': 1.0,
                'timestamp': datetime.now().isoformat()
            },
            pack_monsters=pack_monsters,
            priority=1  # Highest priority
        )
    
    async def send_formation_command(
        self,
        leader_id: int,
        formation_type: str,
        positions: Dict[int, Tuple[int, int]],
        pack_monsters: Optional[List[int]] = None
    ) -> int:
        """
        Send formation command from pack leader
        
        Args:
            leader_id: Pack leader monster ID
            formation_type: Type of formation (flanking, defensive, etc.)
            positions: Dict mapping monster_id -> (x, y) assigned position
            pack_monsters: Pack member IDs
        
        Returns:
            Number of pack members notified
        """
        return await self.broadcast_signal(
            from_monster_id=leader_id,
            signal_type=SignalType.FORM_UP,
            data={
                'formation_type': formation_type,
                'positions': positions,
                'duration': 10.0,  # Formation duration in seconds
                'timestamp': datetime.now().isoformat()
            },
            pack_monsters=pack_monsters,
            priority=3  # Medium-high priority
        )
    
    async def send_help_request(
        self,
        from_monster_id: int,
        urgency: float,
        hp_ratio: float,
        position: Tuple[int, int],
        pack_monsters: Optional[List[int]] = None
    ) -> int:
        """
        Send help request signal
        
        Args:
            from_monster_id: Monster requesting help
            urgency: Urgency level (0.0-1.0)
            hp_ratio: Current HP ratio
            position: Monster position
            pack_monsters: Pack member IDs
        
        Returns:
            Number of pack members notified
        """
        return await self.broadcast_signal(
            from_monster_id=from_monster_id,
            signal_type=SignalType.HELP_REQUEST,
            data={
                'urgency': urgency,
                'hp_ratio': hp_ratio,
                'position': position,
                'timestamp': datetime.now().isoformat()
            },
            pack_monsters=pack_monsters,
            priority=max(1, int(10 * (1.0 - urgency)))  # Higher urgency = higher priority
        )
    
    async def send_defend_leader_signal(
        self,
        leader_id: int,
        threat_direction: Tuple[int, int],
        pack_monsters: Optional[List[int]] = None
    ) -> int:
        """
        Send defend leader signal
        
        Args:
            leader_id: Pack leader ID
            threat_direction: Direction of threat (x, y offset)
            pack_monsters: Pack member IDs
        
        Returns:
            Number of pack members notified
        """
        return await self.broadcast_signal(
            from_monster_id=leader_id,
            signal_type=SignalType.DEFEND_LEADER,
            data={
                'leader_id': leader_id,
                'threat_direction': threat_direction,
                'urgency': 0.8,
                'timestamp': datetime.now().isoformat()
            },
            pack_monsters=pack_monsters,
            priority=2  # High priority
        )
    
    async def _get_pack_members(self, monster_id: int) -> List[int]:
        """
        Get pack members for a monster using Apache AGE graph
        
        Args:
            monster_id: Monster ID
        
        Returns:
            List of pack member monster IDs
        """
        try:
            # Query pack structure from graph
            team_data = await self.graph_manager.get_monster_team(monster_id)
            
            if not team_data:
                return []
            
            team_info = team_data[0]
            
            # Collect all pack members
            pack_members = set()
            
            # Add leader
            leader_id = team_info.get('leader_id')
            if leader_id:
                pack_members.add(leader_id)
            
            # Add followers
            follower_ids = team_info.get('follower_ids', [])
            if follower_ids and follower_ids != [None]:
                pack_members.update(follower_ids)
            
            # Add teammates
            teammate_ids = team_info.get('teammate_ids', [])
            if teammate_ids and teammate_ids != [None]:
                pack_members.update(teammate_ids)
            
            # Remove None values
            pack_members.discard(None)
            
            return list(pack_members)
        
        except Exception as e:
            logger.error(f"Failed to get pack members for {monster_id}: {e}")
            return []
    
    def convert_signals_to_tensor(
        self,
        signals_dict: Dict[int, np.ndarray],
        monster_ids: List[int]
    ) -> torch.Tensor:
        """
        Convert signal dict to tensor for batch inference
        
        Args:
            signals_dict: Dict mapping monster_id -> signal_vector
            monster_ids: Ordered list of monster IDs
        
        Returns:
            signals_tensor: (len(monster_ids), signal_dim) tensor
        """
        signal_vectors = []
        
        for monster_id in monster_ids:
            if monster_id in signals_dict:
                signal_vectors.append(signals_dict[monster_id])
            else:
                # No signals, use zero vector
                signal_vectors.append(np.zeros(self.signal_dim, dtype=np.float32))
        
        signals_tensor = torch.from_numpy(np.array(signal_vectors))
        
        return signals_tensor
    
    async def cleanup_expired_signals(self) -> int:
        """
        Clean up expired signals from graph
        
        Called periodically to remove old SIGNALS edges
        
        Returns:
            Number of signals cleaned up
        """
        try:
            # This uses the graph_manager's cleanup method
            # Graph manager should have cleanup logic for expired signals
            cleanup_results = await self.graph_manager.cleanup_expired_data(retention_days=0)
            
            signals_deleted = cleanup_results.get('signals_deleted', 0)
            
            logger.debug(f"Cleaned up {signals_deleted} expired signals")
            
            return signals_deleted
        
        except Exception as e:
            logger.error(f"Failed to cleanup expired signals: {e}")
            return 0
    
    def clear_cache(self) -> None:
        """Clear signal cache"""
        self.signal_cache.clear()
        self.last_cache_update.clear()
        logger.debug("Signal cache cleared")
    
    def get_statistics(self) -> Dict[str, Any]:
        """
        Get signal coordinator statistics
        
        Returns:
            Statistics dict
        """
        return {
            'signals_sent': self.signals_sent,
            'signals_received': self.signals_received,
            'signals_aggregated': self.signals_aggregated,
            'cached_monsters': len(self.signal_cache),
            'signal_dim': self.signal_dim,
            'signal_range': self.signal_range,
            'signal_ttl': self.signal_ttl
        }


class CoordinationAnalyzer:
    """
    Analyzes pack coordination effectiveness
    
    Tracks:
    - Signal response rates
    - Coordination success metrics
    - Pack cohesion over time
    - Communication patterns
    """
    
    def __init__(self):
        """Initialize coordination analyzer"""
        self.signal_responses: Dict[str, int] = defaultdict(int)
        self.coordination_successes: Dict[str, int] = defaultdict(int)
        self.coordination_attempts: Dict[str, int] = defaultdict(int)
        
        logger.info("CoordinationAnalyzer initialized")
    
    def record_signal_response(
        self,
        signal_type: str,
        responded: bool,
        response_time: float
    ) -> None:
        """
        Record signal response
        
        Args:
            signal_type: Type of signal
            responded: Whether agent responded to signal
            response_time: Time taken to respond (seconds)
        """
        key = f"{signal_type}_{'responded' if responded else 'ignored'}"
        self.signal_responses[key] += 1
    
    def record_coordination_attempt(
        self,
        coordination_type: str,
        success: bool,
        pack_size: int
    ) -> None:
        """
        Record coordination attempt outcome
        
        Args:
            coordination_type: Type of coordination (focus_fire, flanking, etc.)
            success: Whether coordination succeeded
            pack_size: Size of pack involved
        """
        attempt_key = f"{coordination_type}_size_{pack_size}"
        self.coordination_attempts[attempt_key] += 1
        
        if success:
            self.coordination_successes[attempt_key] += 1
    
    def get_coordination_success_rate(
        self,
        coordination_type: Optional[str] = None
    ) -> float:
        """
        Get coordination success rate
        
        Args:
            coordination_type: Specific type (if None, overall rate)
        
        Returns:
            Success rate (0.0-1.0)
        """
        if coordination_type:
            pattern = coordination_type
        else:
            pattern = None
        
        total_attempts = 0
        total_successes = 0
        
        for key, attempts in self.coordination_attempts.items():
            if pattern is None or key.startswith(pattern):
                total_attempts += attempts
                total_successes += self.coordination_successes.get(key, 0)
        
        if total_attempts == 0:
            return 0.0
        
        return total_successes / total_attempts
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get coordination statistics"""
        return {
            'signal_responses': dict(self.signal_responses),
            'coordination_attempts': dict(self.coordination_attempts),
            'coordination_successes': dict(self.coordination_successes),
            'overall_success_rate': self.get_coordination_success_rate()
        }


# ============================================================================
# UTILITIES
# ============================================================================

def create_default_signal_vector() -> np.ndarray:
    """Create default (empty) signal vector"""
    return np.zeros(32, dtype=np.float32)


def decode_signal_vector(signal_vector: np.ndarray) -> Dict[str, Any]:
    """
    Decode signal vector back to human-readable format
    
    Args:
        signal_vector: (32,) signal vector
    
    Returns:
        Decoded signals dict
    """
    decoded = {
        'targets': [],
        'formations': [],
        'support_requests': [],
        'directives': []
    }
    
    # Target information (dims 0-7)
    for i in range(8):
        if signal_vector[i] > 0.1:
            decoded['targets'].append({
                'priority_level': i,
                'strength': float(signal_vector[i])
            })
    
    # Formation commands (dims 8-15)
    formation_names = [
        'flanking', 'defensive_circle', 'spread_out', 'tight_cluster',
        'line_formation', 'v_formation', 'surround', 'custom'
    ]
    for i in range(8):
        if signal_vector[8 + i] > 0.1:
            decoded['formations'].append({
                'type': formation_names[i],
                'strength': float(signal_vector[8 + i])
            })
    
    # Support requests (dims 16-23)
    for i in range(8):
        if signal_vector[16 + i] > 0.1:
            decoded['support_requests'].append({
                'urgency_level': i,
                'strength': float(signal_vector[16 + i])
            })
    
    # Strategic directives (dims 24-31)
    directive_names = [
        'retreat', 'regroup', 'hold', 'advance',
        'scatter', 'fallback', 'charge', 'other'
    ]
    for i in range(8):
        if signal_vector[24 + i] > 0.1:
            decoded['directives'].append({
                'type': directive_names[i],
                'strength': float(signal_vector[24 + i])
            })
    
    return decoded


def visualize_signal_vector(signal_vector: np.ndarray) -> str:
    """
    Create ASCII visualization of signal vector
    
    Args:
        signal_vector: (32,) signal vector
    
    Returns:
        ASCII visualization string
    """
    lines = []
    lines.append("Signal Vector Visualization:")
    lines.append("=" * 50)
    
    # Targets (0-7)
    lines.append("\nTargets (0-7):")
    for i in range(8):
        val = signal_vector[i]
        bar = '█' * int(val * 20)
        lines.append(f"  [{i}] {bar} {val:.3f}")
    
    # Formations (8-15)
    lines.append("\nFormations (8-15):")
    formation_names = ['flank', 'defend', 'spread', 'cluster', 'line', 'v', 'surround', 'custom']
    for i in range(8):
        val = signal_vector[8 + i]
        bar = '█' * int(val * 20)
        lines.append(f"  {formation_names[i]:8s} {bar} {val:.3f}")
    
    # Support (16-23)
    lines.append("\nSupport Requests (16-23):")
    for i in range(8):
        val = signal_vector[16 + i]
        if val > 0.05:
            bar = '█' * int(val * 20)
            lines.append(f"  Urgency {i}: {bar} {val:.3f}")
    
    # Directives (24-31)
    lines.append("\nDirectives (24-31):")
    directive_names = ['retreat', 'regroup', 'hold', 'advance', 'scatter', 'fallback', 'charge', 'other']
    for i in range(8):
        val = signal_vector[24 + i]
        if val > 0.05:
            bar = '█' * int(val * 20)
            lines.append(f"  {directive_names[i]:8s} {bar} {val:.3f}")
    
    lines.append("=" * 50)
    
    return '\n'.join(lines)


# ============================================================================
# TESTING
# ============================================================================

async def test_signal_coordinator():
    """Test signal coordinator functionality"""
    import sys
    sys.path.append('..')
    
    from graph_manager import AGEGraphManager
    import asyncpg
    import os
    
    logger.info("Testing SignalCoordinator...")
    
    # Create test database connection
    try:
        pool = await asyncpg.create_pool(
            host=os.getenv('POSTGRES_HOST', 'localhost'),
            port=int(os.getenv('POSTGRES_PORT', '5432')),
            database=os.getenv('POSTGRES_DB', 'ragnarok_ml'),
            user=os.getenv('POSTGRES_USER', 'ml_user'),
            password=os.getenv('POSTGRES_PASSWORD', ''),
            min_size=2,
            max_size=5
        )
    except Exception as e:
        logger.error(f"Cannot connect to database for testing: {e}")
        logger.info("Skipping database-dependent tests")
        
        # Test signal vector encoding without database
        logger.info("\nTesting signal vector encoding (no DB required)...")
        
        # Create mock graph manager
        class MockGraphManager:
            async def send_signal(self, **kwargs):
                return True
            
            async def receive_signals(self, monster_id):
                return [
                    {
                        'signal_type': SignalType.ATTACK_TARGET,
                        'content': {'target_threat': 0.8, 'priority': 0.9},
                        'priority': 2
                    },
                    {
                        'signal_type': SignalType.FORM_UP,
                        'content': {'formation_type': 'flanking'},
                        'priority': 3
                    }
                ]
            
            async def get_monster_team(self, monster_id):
                return [{'leader_id': monster_id, 'follower_ids': [monster_id + 1, monster_id + 2]}]
        
        coordinator = SignalCoordinator(MockGraphManager())
        
        # Test aggregation
        signals = await coordinator.receive_signals(1001)
        signal_vector = coordinator.aggregate_signals_to_vector(signals)
        
        assert signal_vector.shape == (32,), f"Expected shape (32,), got {signal_vector.shape}"
        assert signal_vector.sum() > 0, "Signal vector should not be empty"
        
        logger.info(f"✓ Signal vector shape: {signal_vector.shape}")
        logger.info(f"✓ Signal vector sum: {signal_vector.sum():.3f}")
        
        # Test visualization
        logger.info("\nSignal Vector Visualization:")
        print(visualize_signal_vector(signal_vector))
        
        # Test decoding
        decoded = decode_signal_vector(signal_vector)
        logger.info(f"\n✓ Decoded signals: {json.dumps(decoded, indent=2)}")
        
        logger.info("\n✓ All SignalCoordinator tests passed (without database)!")
        return
    
    # If database is available, run full tests
    try:
        graph_mgr = AGEGraphManager(pool, graph_name='monster_ai')
        coordinator = SignalCoordinator(graph_mgr)
        
        # Test 1: Broadcast signal
        logger.info("\n1. Testing broadcast_signal()...")
        sent_count = await coordinator.broadcast_signal(
            from_monster_id=1001,
            signal_type=SignalType.ATTACK_TARGET,
            data={'target_id': 5001, 'target_threat': 0.8},
            pack_monsters=[1002, 1003, 1004],
            priority=2
        )
        logger.info(f"✓ Broadcast signal sent to {sent_count} monsters")
        
        # Test 2: Receive signals
        logger.info("\n2. Testing receive_signals()...")
        signals = await coordinator.receive_signals(1002)
        logger.info(f"✓ Received {len(signals)} signals for monster 1002")
        
        # Test 3: Aggregate signals
        logger.info("\n3. Testing signal aggregation...")
        signal_vector = coordinator.aggregate_signals_to_vector(signals)
        logger.info(f"✓ Aggregated signal vector shape: {signal_vector.shape}")
        logger.info(f"  Signal vector: {signal_vector[:8]}")  # First 8 dims
        
        # Test 4: Process coordination signals
        logger.info("\n4. Testing process_coordination_signals()...")
        pack_monsters = [1001, 1002, 1003]
        signals_dict = await coordinator.process_coordination_signals(pack_monsters)
        logger.info(f"✓ Processed signals for {len(signals_dict)} monsters")
        
        # Test 5: Helper methods
        logger.info("\n5. Testing helper signal methods...")
        
        sent = await coordinator.send_attack_target_signal(
            from_monster_id=1001,
            target_id=5001,
            target_threat=0.9,
            pack_monsters=[1002, 1003]
        )
        logger.info(f"✓ Attack target signal sent to {sent} monsters")
        
        sent = await coordinator.send_retreat_signal(
            from_monster_id=1001,
            rally_point=(150, 200),
            reason='outnumbered',
            pack_monsters=[1002, 1003]
        )
        logger.info(f"✓ Retreat signal sent to {sent} monsters")
        
        # Test 6: Statistics
        logger.info("\n6. Testing statistics...")
        stats = coordinator.get_statistics()
        logger.info(f"✓ Coordinator stats: {json.dumps(stats, indent=2)}")
        
        logger.info("\n✓ All SignalCoordinator tests passed!")
    
    finally:
        await pool.close()


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    asyncio.run(test_signal_coordinator())
