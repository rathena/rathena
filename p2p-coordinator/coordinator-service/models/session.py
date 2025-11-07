"""
P2P Session Model

Represents an active P2P session between a host and players in a zone.
"""

import enum
from datetime import datetime
from typing import Optional, TYPE_CHECKING
from sqlalchemy import String, Integer, Float, Boolean, DateTime, Enum, JSON, Text, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from models.base import Base

if TYPE_CHECKING:
    from models.host import Host  # noqa: F401
    from models.zone import Zone  # noqa: F401


class SessionStatus(str, enum.Enum):
    """Session status enumeration"""
    PENDING = "pending"
    ACTIVE = "active"
    PAUSED = "paused"
    ENDED = "ended"
    FAILED = "failed"


class P2PSession(Base):
    """
    P2P Session Model
    
    Represents an active P2P session where a host is serving a zone to players.
    Tracks session state, performance metrics, and player connections.
    """
    
    __tablename__ = "p2p_sessions"
    
    # Primary Key
    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    
    # Session Identification
    session_id: Mapped[str] = mapped_column(String(255), unique=True, nullable=False, index=True)
    
    # Foreign Keys
    host_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("hosts.id", ondelete="CASCADE"),
        nullable=False,
        index=True
    )
    zone_id: Mapped[int] = mapped_column(
        Integer,
        ForeignKey("zones.id", ondelete="CASCADE"),
        nullable=False,
        index=True
    )
    
    # Session Configuration
    max_players: Mapped[int] = mapped_column(Integer, nullable=False, default=50)
    current_players: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    
    # Status
    status: Mapped[SessionStatus] = mapped_column(
        Enum(SessionStatus),
        nullable=False,
        default=SessionStatus.PENDING,
        index=True
    )
    
    # Performance Metrics
    average_latency_ms: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    average_packet_loss_percent: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    bandwidth_usage_mbps: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    
    # Quality Metrics
    quality_score: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    player_satisfaction_score: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    
    # Connection Information
    connection_type: Mapped[str] = mapped_column(String(50), nullable=False, default="webrtc")  # webrtc, fallback
    ice_connection_state: Mapped[Optional[str]] = mapped_column(String(50), nullable=True)
    
    # Player List (JSON array of player IDs)
    connected_players: Mapped[Optional[list]] = mapped_column(JSON, nullable=True, default=list)
    
    # Session Events (JSON array of events)
    events: Mapped[Optional[list]] = mapped_column(JSON, nullable=True, default=list)
    
    # Metadata
    extra_metadata: Mapped[Optional[dict]] = mapped_column(JSON, nullable=True)
    
    # Timestamps
    created_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        nullable=False
    )
    updated_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        onupdate=func.now(),
        nullable=False
    )
    started_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime(timezone=True),
        nullable=True
    )
    ended_at: Mapped[Optional[datetime]] = mapped_column(
        DateTime(timezone=True),
        nullable=True
    )
    
    # Relationships
    host: Mapped["Host"] = relationship("Host", back_populates="sessions")
    zone: Mapped["Zone"] = relationship("Zone", back_populates="sessions")
    
    def __repr__(self) -> str:
        return f"<P2PSession(id={self.id}, session_id='{self.session_id}', status={self.status.value}, players={self.current_players}/{self.max_players})>"
    
    @property
    def is_active(self) -> bool:
        """Check if session is currently active"""
        return self.status == SessionStatus.ACTIVE
    
    @property
    def capacity_percent(self) -> float:
        """Calculate current capacity usage percentage"""
        if self.max_players == 0:
            return 100.0
        return (self.current_players / self.max_players) * 100.0
    
    @property
    def duration_seconds(self) -> Optional[float]:
        """Calculate session duration in seconds"""
        if not self.started_at:
            return None
        
        end_time = self.ended_at if self.ended_at else datetime.utcnow()
        duration = end_time - self.started_at
        return duration.total_seconds()
    
    def add_player(self, player_id: str) -> bool:
        """
        Add a player to the session
        
        Args:
            player_id: Player identifier
            
        Returns:
            bool: True if player was added, False if session is full
        """
        if self.current_players >= self.max_players:
            return False
        
        if self.connected_players is None:
            self.connected_players = []
        
        if player_id not in self.connected_players:
            self.connected_players.append(player_id)
            self.current_players = len(self.connected_players)
        
        return True
    
    def remove_player(self, player_id: str) -> bool:
        """
        Remove a player from the session
        
        Args:
            player_id: Player identifier
            
        Returns:
            bool: True if player was removed, False if player not found
        """
        if self.connected_players is None or player_id not in self.connected_players:
            return False
        
        self.connected_players.remove(player_id)
        self.current_players = len(self.connected_players)
        return True

