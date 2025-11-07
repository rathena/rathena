"""
Zone Model

Represents a game zone (map) that can be hosted via P2P.
"""

import enum
from datetime import datetime
from typing import Optional, TYPE_CHECKING
from sqlalchemy import String, Integer, Float, Boolean, DateTime, Enum, JSON, Text
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from models.base import Base

if TYPE_CHECKING:
    from models.session import P2PSession  # noqa: F401


class ZoneStatus(str, enum.Enum):
    """Zone status enumeration"""
    ENABLED = "enabled"
    DISABLED = "disabled"
    MAINTENANCE = "maintenance"


class Zone(Base):
    """
    Zone Model
    
    Represents a game zone/map that can be hosted via P2P.
    Tracks zone configuration, requirements, and current hosting status.
    """
    
    __tablename__ = "zones"
    
    # Primary Key
    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    
    # Zone Identification
    zone_id: Mapped[str] = mapped_column(String(255), unique=True, nullable=False, index=True)
    zone_name: Mapped[str] = mapped_column(String(255), nullable=False)
    zone_display_name: Mapped[str] = mapped_column(String(255), nullable=False)
    
    # Zone Configuration
    map_name: Mapped[str] = mapped_column(String(255), nullable=False)
    max_players: Mapped[int] = mapped_column(Integer, nullable=False, default=100)
    recommended_players: Mapped[int] = mapped_column(Integer, nullable=False, default=50)
    
    # P2P Configuration
    p2p_enabled: Mapped[bool] = mapped_column(Boolean, nullable=False, default=True, index=True)
    p2p_priority: Mapped[int] = mapped_column(Integer, nullable=False, default=0)  # Higher = more priority
    fallback_enabled: Mapped[bool] = mapped_column(Boolean, nullable=False, default=True)
    
    # Requirements
    min_host_quality_score: Mapped[float] = mapped_column(Float, nullable=False, default=50.0)
    min_bandwidth_mbps: Mapped[int] = mapped_column(Integer, nullable=False, default=50)
    max_latency_ms: Mapped[int] = mapped_column(Integer, nullable=False, default=100)
    
    # Status
    status: Mapped[ZoneStatus] = mapped_column(
        Enum(ZoneStatus),
        nullable=False,
        default=ZoneStatus.ENABLED,
        index=True
    )
    
    # Statistics
    total_sessions: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    active_sessions: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    total_players_served: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    
    # Metadata
    description: Mapped[Optional[str]] = mapped_column(Text, nullable=True)
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
    
    # Relationships
    sessions: Mapped[list["P2PSession"]] = relationship(
        "P2PSession",
        back_populates="zone",
        cascade="all, delete-orphan"
    )
    
    def __repr__(self) -> str:
        return f"<Zone(id={self.id}, zone_id='{self.zone_id}', name='{self.zone_name}', p2p_enabled={self.p2p_enabled})>"
    
    @property
    def is_available_for_p2p(self) -> bool:
        """Check if zone is available for P2P hosting"""
        return (
            self.status == ZoneStatus.ENABLED
            and self.p2p_enabled
        )
    
    @property
    def capacity_percent(self) -> float:
        """Calculate current capacity usage percentage"""
        if self.max_players == 0:
            return 100.0
        # This would need to be calculated from active sessions
        return 0.0  # Placeholder

