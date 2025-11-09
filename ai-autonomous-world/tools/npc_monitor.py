#!/usr/bin/env python3
"""
NPC Monitoring and Debugging Tool
Real-time monitoring of NPC behavior, decisions, and world state
"""

import sys
import argparse
import asyncio
import json
from pathlib import Path
from datetime import datetime
from typing import Optional, List, Dict, Any
from rich.console import Console
from rich.table import Table
from rich.panel import Panel
from rich.layout import Layout
from rich.live import Live
from rich.text import Text
import httpx

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

console = Console()

# API Configuration
API_BASE_URL = "http://localhost:8000"


class NPCMonitor:
    """NPC monitoring and debugging tool"""
    
    def __init__(self, api_url: str = API_BASE_URL):
        self.api_url = api_url
        self.client = httpx.AsyncClient(timeout=30.0)
    
    async def close(self):
        """Close HTTP client"""
        await self.client.aclose()
    
    async def get_npc_state(self, npc_id: str) -> Dict[str, Any]:
        """Get current NPC state"""
        try:
            response = await self.client.get(f"{self.api_url}/ai/npc/{npc_id}/state")
            response.raise_for_status()
            return response.json()
        except httpx.HTTPError as e:
            console.print(f"[red]Error fetching NPC state: {e}[/red]")
            return {}
    
    async def get_npc_decisions(self, npc_id: str, limit: int = 10) -> List[Dict[str, Any]]:
        """Get recent NPC decisions"""
        try:
            response = await self.client.get(
                f"{self.api_url}/ai/npc/{npc_id}/decisions",
                params={"limit": limit}
            )
            response.raise_for_status()
            return response.json().get("decisions", [])
        except httpx.HTTPError as e:
            console.print(f"[red]Error fetching decisions: {e}[/red]")
            return []
    
    async def get_npc_memories(self, npc_id: str, limit: int = 10) -> List[Dict[str, Any]]:
        """Get recent NPC memories"""
        try:
            response = await self.client.get(
                f"{self.api_url}/ai/npc/{npc_id}/memories",
                params={"limit": limit}
            )
            response.raise_for_status()
            return response.json().get("memories", [])
        except httpx.HTTPError as e:
            console.print(f"[red]Error fetching memories: {e}[/red]")
            return []
    
    async def get_npc_relationships(self, npc_id: str) -> List[Dict[str, Any]]:
        """Get NPC relationships"""
        try:
            response = await self.client.get(f"{self.api_url}/ai/npc/{npc_id}/relationships")
            response.raise_for_status()
            return response.json().get("relationships", [])
        except httpx.HTTPError as e:
            console.print(f"[red]Error fetching relationships: {e}[/red]")
            return []
    
    async def get_world_state(self, state_type: str = "all") -> Dict[str, Any]:
        """Get world state"""
        try:
            response = await self.client.get(
                f"{self.api_url}/ai/world/state",
                params={"type": state_type}
            )
            response.raise_for_status()
            return response.json()
        except httpx.HTTPError as e:
            console.print(f"[red]Error fetching world state: {e}[/red]")
            return {}
    
    async def list_npcs(self) -> List[Dict[str, Any]]:
        """List all NPCs"""
        try:
            response = await self.client.get(f"{self.api_url}/ai/npc/list")
            response.raise_for_status()
            return response.json().get("npcs", [])
        except httpx.HTTPError as e:
            console.print(f"[red]Error listing NPCs: {e}[/red]")
            return []
    
    def display_npc_state(self, npc_id: str, state: Dict[str, Any]):
        """Display NPC state in formatted table"""
        if not state:
            console.print(f"[yellow]No state found for NPC: {npc_id}[/yellow]")
            return
        
        # Create main info table
        table = Table(title=f"NPC State: {npc_id}", show_header=True, header_style="bold magenta")
        table.add_column("Property", style="cyan", width=20)
        table.add_column("Value", style="green")
        
        # Basic info
        table.add_row("NPC ID", state.get("npc_id", "N/A"))
        table.add_row("Name", state.get("name", "N/A"))
        table.add_row("Class", state.get("npc_class", "N/A"))
        table.add_row("Level", str(state.get("level", "N/A")))
        
        # Position
        pos = state.get("position", {})
        position_str = f"{pos.get('map', 'N/A')} ({pos.get('x', 0)}, {pos.get('y', 0)})"
        table.add_row("Position", position_str)
        
        # Personality
        personality = state.get("personality", {})
        if personality:
            table.add_row("Openness", f"{personality.get('openness', 0):.2f}")
            table.add_row("Conscientiousness", f"{personality.get('conscientiousness', 0):.2f}")
            table.add_row("Extraversion", f"{personality.get('extraversion', 0):.2f}")
            table.add_row("Agreeableness", f"{personality.get('agreeableness', 0):.2f}")
            table.add_row("Neuroticism", f"{personality.get('neuroticism', 0):.2f}")
            table.add_row("Alignment", personality.get("moral_alignment", "N/A"))
        
        # Goals
        goals = state.get("goals", [])
        if goals:
            table.add_row("Active Goals", ", ".join(goals[:3]))
        
        # Faction
        faction = state.get("faction_id", "None")
        table.add_row("Faction", faction)
        
        console.print(table)
    
    def display_decisions(self, decisions: List[Dict[str, Any]]):
        """Display NPC decisions"""
        if not decisions:
            console.print("[yellow]No decisions found[/yellow]")
            return
        
        table = Table(title="Recent Decisions", show_header=True, header_style="bold magenta")
        table.add_column("Timestamp", style="cyan", width=20)
        table.add_column("Decision Type", style="yellow", width=15)
        table.add_column("Action", style="green", width=30)
        table.add_column("Reasoning", style="white")
        
        for decision in decisions[:10]:
            timestamp = decision.get("timestamp", "N/A")
            decision_type = decision.get("type", "N/A")
            action = decision.get("action", "N/A")
            reasoning = decision.get("reasoning", "N/A")[:50] + "..."
            
            table.add_row(timestamp, decision_type, action, reasoning)
        
        console.print(table)
    
    def display_memories(self, memories: List[Dict[str, Any]]):
        """Display NPC memories"""
        if not memories:
            console.print("[yellow]No memories found[/yellow]")
            return
        
        table = Table(title="Recent Memories", show_header=True, header_style="bold magenta")
        table.add_column("Timestamp", style="cyan", width=20)
        table.add_column("Type", style="yellow", width=15)
        table.add_column("Content", style="white")
        
        for memory in memories[:10]:
            timestamp = memory.get("timestamp", "N/A")
            memory_type = memory.get("type", "N/A")
            content = memory.get("content", "N/A")[:60] + "..."
            
            table.add_row(timestamp, memory_type, content)
        
        console.print(table)
    
    def display_relationships(self, relationships: List[Dict[str, Any]]):
        """Display NPC relationships"""
        if not relationships:
            console.print("[yellow]No relationships found[/yellow]")
            return
        
        table = Table(title="Relationships", show_header=True, header_style="bold magenta")
        table.add_column("Target", style="cyan", width=20)
        table.add_column("Type", style="yellow", width=15)
        table.add_column("Affinity", style="green", width=10)
        table.add_column("Last Interaction", style="white")
        
        for rel in relationships:
            target = rel.get("target_id", "N/A")
            rel_type = rel.get("relationship_type", "N/A")
            affinity = f"{rel.get('affinity_score', 0):.2f}"
            last_interaction = rel.get("last_interaction", "Never")
            
            table.add_row(target, rel_type, affinity, last_interaction)
        
        console.print(table)
    
    def display_world_state(self, world_state: Dict[str, Any]):
        """Display world state"""
        if not world_state:
            console.print("[yellow]No world state found[/yellow]")
            return
        
        # Economy
        economy = world_state.get("economy", {})
        if economy:
            console.print(Panel.fit(
                f"[bold cyan]Economy State[/bold cyan]\n"
                f"Trade Volume: {economy.get('trade_volume', 0)}\n"
                f"Inflation Rate: {economy.get('inflation_rate', 0):.2%}\n"
                f"Active Markets: {len(economy.get('item_prices', {}))}",
                title="Economy"
            ))
        
        # Environment
        environment = world_state.get("environment", {})
        if environment:
            console.print(Panel.fit(
                f"[bold green]Environment State[/bold green]\n"
                f"Weather: {environment.get('weather', 'N/A')}\n"
                f"Time: {environment.get('time_of_day', 'N/A')}\n"
                f"Season: {environment.get('season', 'N/A')}",
                title="Environment"
            ))
        
        # Politics
        politics = world_state.get("politics", {})
        if politics:
            console.print(Panel.fit(
                f"[bold red]Political State[/bold red]\n"
                f"Active Factions: {len(politics.get('factions', []))}\n"
                f"Conflicts: {len(politics.get('conflicts', []))}",
                title="Politics"
            ))


async def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="NPC Monitoring and Debugging Tool")
    parser.add_argument("--npc-id", type=str, help="NPC ID to monitor")
    parser.add_argument("--list-npcs", action="store_true", help="List all NPCs")
    parser.add_argument("--show-state", action="store_true", help="Show NPC state")
    parser.add_argument("--show-decisions", action="store_true", help="Show NPC decisions")
    parser.add_argument("--show-memories", action="store_true", help="Show NPC memories")
    parser.add_argument("--show-relationships", action="store_true", help="Show NPC relationships")
    parser.add_argument("--world-state", type=str, choices=["all", "economy", "environment", "politics"], 
                       help="Show world state")
    parser.add_argument("--api-url", type=str, default=API_BASE_URL, help="API base URL")
    
    args = parser.parse_args()
    
    monitor = NPCMonitor(api_url=args.api_url)
    
    try:
        if args.list_npcs:
            console.print("[bold cyan]Listing all NPCs...[/bold cyan]")
            npcs = await monitor.list_npcs()
            
            table = Table(title="All NPCs", show_header=True, header_style="bold magenta")
            table.add_column("NPC ID", style="cyan")
            table.add_column("Name", style="green")
            table.add_column("Class", style="yellow")
            table.add_column("Level", style="white")
            table.add_column("Position", style="blue")
            
            for npc in npcs:
                pos = npc.get("position", {})
                position_str = f"{pos.get('map', 'N/A')} ({pos.get('x', 0)}, {pos.get('y', 0)})"
                table.add_row(
                    npc.get("npc_id", "N/A"),
                    npc.get("name", "N/A"),
                    npc.get("npc_class", "N/A"),
                    str(npc.get("level", "N/A")),
                    position_str
                )
            
            console.print(table)
        
        elif args.npc_id:
            if args.show_state or not any([args.show_decisions, args.show_memories, args.show_relationships]):
                console.print(f"[bold cyan]Fetching state for NPC: {args.npc_id}[/bold cyan]")
                state = await monitor.get_npc_state(args.npc_id)
                monitor.display_npc_state(args.npc_id, state)
            
            if args.show_decisions:
                console.print(f"\n[bold cyan]Fetching decisions for NPC: {args.npc_id}[/bold cyan]")
                decisions = await monitor.get_npc_decisions(args.npc_id)
                monitor.display_decisions(decisions)
            
            if args.show_memories:
                console.print(f"\n[bold cyan]Fetching memories for NPC: {args.npc_id}[/bold cyan]")
                memories = await monitor.get_npc_memories(args.npc_id)
                monitor.display_memories(memories)
            
            if args.show_relationships:
                console.print(f"\n[bold cyan]Fetching relationships for NPC: {args.npc_id}[/bold cyan]")
                relationships = await monitor.get_npc_relationships(args.npc_id)
                monitor.display_relationships(relationships)
        
        elif args.world_state:
            console.print(f"[bold cyan]Fetching world state: {args.world_state}[/bold cyan]")
            world_state = await monitor.get_world_state(args.world_state)
            monitor.display_world_state(world_state)
        
        else:
            parser.print_help()
    
    finally:
        await monitor.close()


if __name__ == "__main__":
    asyncio.run(main())

