"""
P2P Host Model

Represents a P2P host that can serve game zones to players.
"""

import enum
from datetime import datetime
from typing import Optional
from sqlalchemy import String, Integer, Float, Boolean, DateTime, Enum, JSON, Text
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from models.base import Base


class HostStatus(str, enum.Enum):
    """Host status enumeration"""
    ONLINE = "online"
    OFFLINE = "offline"
    BUSY = "busy"
    MAINTENANCE = "maintenance"


class Host(Base):
    """
    P2P Host Model
    
    Represents a player's machine that can host game zones for other players.
    Tracks hardware capabilities, network quality, and current load.
    """
    
    __tablename__ = "hosts"
    
    # Primary Key
    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    
    # Host Identification
    host_id: Mapped[str] = mapped_column(String(255), unique=True, nullable=False, index=True)
    player_id: Mapped[str] = mapped_column(String(255), nullable=False, index=True)
    player_name: Mapped[str] = mapped_column(String(255), nullable=False)
    
    # Network Information
    ip_address: Mapped[str] = mapped_column(String(45), nullable=False)  # IPv4 or IPv6
    port: Mapped[int] = mapped_column(Integer, nullable=False)
    public_ip: Mapped[Optional[str]] = mapped_column(String(45), nullable=True)
    
    # Hardware Capabilities
    cpu_cores: Mapped[int] = mapped_column(Integer, nullable=False)
    cpu_frequency_mhz: Mapped[int] = mapped_column(Integer, nullable=False)
    memory_mb: Mapped[int] = mapped_column(Integer, nullable=False)
    network_speed_mbps: Mapped[int] = mapped_column(Integer, nullable=False)
    
    # Performance Metrics
    latency_ms: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    packet_loss_percent: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    bandwidth_usage_mbps: Mapped[float] = mapped_column(Float, nullable=False, default=0.0)
    
    # Host Status
    status: Mapped[HostStatus] = mapped_column(
        Enum(HostStatus),
        nullable=False,
        default=HostStatus.OFFLINE,
        index=True
    )
    
    # Capacity
    max_players: Mapped[int] = mapped_column(Integer, nullable=False, default=50)
    current_players: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    max_zones: Mapped[int] = mapped_column(Integer, nullable=False, default=5)
    current_zones: Mapped[int] = mapped_column(Integer, nullable=False, default=0)
    
    # Quality Score (0-100)
    quality_score: Mapped[float] = mapped_column(Float, nullable=False, default=0.0, index=True)
    
    # WebRTC Configuration
    ice_servers: Mapped[Optional[dict]] = mapped_column(JSON, nullable=True)
    
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
    last_heartbeat: Mapped[Optional[datetime]] = mapped_column(
        DateTime(timezone=True),
        nullable=True
    )
    
    # Relationships
    sessions: Mapped[list["P2PSession"]] = relationship(
        "P2PSession",
        back_populates="host",
        cascade="all, delete-orphan"
    )
    
    def __repr__(self) -> str:
        return f"<Host(id={self.id}, host_id='{self.host_id}', player='{self.player_name}', status={self.status.value})>"
    
    @property
    def is_available(self) -> bool:
        """Check if host is available to accept new connections"""
        return (
            self.status == HostStatus.ONLINE
            and self.current_players < self.max_players
            and self.current_zones < self.max_zones
        )
    
    @property
    def capacity_percent(self) -> float:
        """Calculate current capacity usage percentage"""
        max_players = self.max_players if self.max_players is not None else 50
        current_players = self.current_players if self.current_players is not None else 0

        if max_players == 0:
            return 100.0
        return (current_players / max_players) * 100.0
    
    def calculate_quality_score(self) -> float:
        """
        Calculate host quality score based on multiple factors

        Factors:
        - Latency (lower is better)
        - Packet loss (lower is better)
        - Available capacity (higher is better)
        - Hardware specs (higher is better)

        Returns:
            float: Quality score from 0-100
        """
        # Latency score (0-30 points)
        # Use 0 latency if not yet measured (assume best case)
        latency_ms = self.latency_ms if self.latency_ms is not None else 0
        latency_score = max(0, 30 - (latency_ms / 10))

        # Packet loss score (0-20 points)
        # Use 0 packet loss if not yet measured (assume best case)
        packet_loss_percent = self.packet_loss_percent if self.packet_loss_percent is not None else 0
        packet_loss_score = max(0, 20 - (packet_loss_percent * 2))

        # Capacity score (0-25 points)
        available_capacity = 100 - self.capacity_percent
        capacity_score = (available_capacity / 100) * 25

        # Hardware score (0-25 points)
        cpu_score = min(10, self.cpu_cores / 4 * 10)
        memory_score = min(10, self.memory_mb / 8192 * 10)
        network_score = min(5, self.network_speed_mbps / 100 * 5)
        hardware_score = cpu_score + memory_score + network_score

        total_score = latency_score + packet_loss_score + capacity_score + hardware_score
        return round(min(100.0, max(0.0, total_score)), 2)

