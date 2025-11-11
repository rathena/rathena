"""
Helper functions for tests.
"""

import uuid
from typing import Dict, Any, List
from faker import Faker

fake = Faker()


def generate_host_id() -> str:
    """Generate a unique host ID for testing."""
    return f"test_host_{uuid.uuid4().hex[:8]}"


def generate_session_id() -> str:
    """Generate a unique session ID for testing."""
    return f"test_session_{uuid.uuid4().hex[:8]}"


def generate_player_id() -> str:
    """Generate a unique player ID for testing."""
    return f"test_player_{uuid.uuid4().hex[:8]}"


def create_test_host(
    host_id: str = None,
    player_id: str = None,
    **kwargs
) -> Dict[str, Any]:
    """
    Create test host data.
    
    Args:
        host_id: Optional host ID (generated if not provided)
        player_id: Optional player ID (generated if not provided)
        **kwargs: Additional fields to override defaults
    
    Returns:
        Dictionary with host registration data
    """
    data = {
        "host_id": host_id or generate_host_id(),
        "player_id": player_id or generate_player_id(),
        "player_name": fake.user_name(),
        "ip_address": fake.ipv4_private(),
        "port": fake.random_int(min=7000, max=8000),
        "cpu_cores": fake.random_int(min=4, max=16),
        "cpu_frequency_mhz": fake.random_int(min=2400, max=4800),
        "memory_mb": fake.random_int(min=8192, max=32768),
        "network_speed_mbps": fake.random_int(min=100, max=1000),
    }
    data.update(kwargs)
    return data


def create_test_zone(
    zone_id: str = None,
    **kwargs
) -> Dict[str, Any]:
    """
    Create test zone data.

    Args:
        zone_id: Optional zone ID (defaults to random)
        **kwargs: Additional fields to override defaults

    Returns:
        Dictionary with zone data
    """
    zone_name = fake.city()
    data = {
        "zone_id": zone_id or f"zone_{uuid.uuid4().hex[:8]}",
        "zone_name": zone_name,
        "zone_display_name": zone_name,
        "map_name": f"map_{fake.word()}",
        "max_players": fake.random_int(min=50, max=200),
        "recommended_players": fake.random_int(min=20, max=50),
        "p2p_enabled": True,
        "p2p_priority": 1,
        "fallback_enabled": True,
        "min_host_quality_score": 7.0,
        "min_bandwidth_mbps": 100,
        "max_latency_ms": 100,
        "description": f"Test zone {zone_name}",
    }
    data.update(kwargs)
    return data


def create_test_session(
    session_id: str = None,
    host_id: str = None,
    zone_id: str = None,
    player_ids: List[str] = None,
    **kwargs
) -> Dict[str, Any]:
    """
    Create test session data.

    Args:
        session_id: Optional session ID (generated if not provided)
        host_id: Optional host ID (generated if not provided)
        zone_id: Optional zone ID (defaults to "prontera")
        player_ids: Optional list of player IDs (generated if not provided)
        **kwargs: Additional fields to override defaults

    Returns:
        Dictionary with session data
    """
    data = {
        "host_id": host_id or generate_host_id(),
        "zone_id": zone_id or "prontera",
        "player_ids": player_ids or [generate_player_id() for _ in range(fake.random_int(min=1, max=5))],
    }
    data.update(kwargs)
    return data


def create_test_signaling_message(
    message_type: str,
    peer_id: str = None,
    target_peer_id: str = None,
    **kwargs
) -> Dict[str, Any]:
    """
    Create test WebSocket signaling message.
    
    Args:
        message_type: Type of message (join, leave, offer, answer, ice-candidate)
        peer_id: Optional peer ID (generated if not provided)
        target_peer_id: Optional target peer ID
        **kwargs: Additional fields based on message type
    
    Returns:
        Dictionary with signaling message data
    """
    data = {
        "type": message_type,
        "peer_id": peer_id or generate_player_id(),
    }
    
    if target_peer_id:
        data["target_peer_id"] = target_peer_id
    
    if message_type == "join":
        data["session_id"] = kwargs.get("session_id", generate_session_id())
    elif message_type in ["offer", "answer"]:
        data["sdp"] = kwargs.get("sdp", "v=0\r\no=- 0 0 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n")
    elif message_type == "ice-candidate":
        data["candidate"] = kwargs.get("candidate", "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host")
    
    data.update(kwargs)
    return data


def create_test_jwt_payload(
    player_id: str = None,
    **kwargs
) -> Dict[str, Any]:
    """
    Create test JWT payload.
    
    Args:
        player_id: Optional player ID (generated if not provided)
        **kwargs: Additional claims
    
    Returns:
        Dictionary with JWT payload
    """
    from datetime import datetime, timedelta
    
    data = {
        "sub": player_id or generate_player_id(),
        "player_id": player_id or generate_player_id(),
        "exp": datetime.utcnow() + timedelta(hours=1),
        "iat": datetime.utcnow(),
    }
    data.update(kwargs)
    return data

