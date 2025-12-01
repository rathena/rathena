"""
Production Chain Models
Defines multi-step production from raw materials to finished goods
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Optional
from datetime import datetime


class ProductionStep(BaseModel):
    """Single step in production chain"""
    step_id: str
    inputs: Dict[str, int]  # {item_id: quantity}
    outputs: Dict[str, int]  # {item_id: quantity}
    labor_cost: float  # Cost in zeny
    time_required: int  # Seconds
    skill_required: Optional[str] = None
    success_rate: float = 1.0  # 0.0 to 1.0
    
    class Config:
        json_schema_extra = {
            "example": {
                "step_id": "smelt_iron",
                "inputs": {"iron_ore": 5, "coal": 2},
                "outputs": {"iron_ingot": 1},
                "labor_cost": 100.0,
                "time_required": 300,
                "skill_required": "smithing",
                "success_rate": 0.95
            }
        }


class ProductionChain(BaseModel):
    """Complete production chain from raw to finished"""
    chain_id: str
    chain_name: str
    steps: List[ProductionStep]
    final_product: str  # Item ID or name
    category: str = "crafting"
    difficulty: int = 1  # 1-10 scale
    created_at: datetime = Field(default_factory=datetime.utcnow)
    
    def calculate_total_cost(self, market_prices: Dict[str, float]) -> float:
        """
        Calculate total production cost given market prices
        
        Args:
            market_prices: Dictionary of item_id -> price
            
        Returns:
            Total cost in zeny
        """
        total = 0.0
        
        for step in self.steps:
            # Input costs
            for item_id, qty in step.inputs.items():
                price = market_prices.get(item_id, 0)
                total += price * qty
            
            # Labor cost
            total += step.labor_cost
        
        return total
    
    def calculate_total_time(self) -> int:
        """Calculate total production time in seconds"""
        return sum(step.time_required for step in self.steps)
    
    def get_all_required_inputs(self) -> Dict[str, int]:
        """
        Get all raw materials needed (first-step inputs only)
        
        Returns:
            Dictionary of item_id -> total quantity
        """
        if not self.steps:
            return {}
        
        # Only count inputs from first step (raw materials)
        raw_materials = {}
        first_step_inputs = self.steps[0].inputs
        
        for item_id, qty in first_step_inputs.items():
            raw_materials[item_id] = raw_materials.get(item_id, 0) + qty
        
        return raw_materials
    
    def get_final_outputs(self) -> Dict[str, int]:
        """Get final products from last step"""
        if not self.steps:
            return {}
        return self.steps[-1].outputs.copy()
    
    def validate_chain(self) -> bool:
        """
        Validate that chain steps connect properly
        
        Returns:
            True if valid, False otherwise
        """
        if not self.steps:
            return False
        
        # Check that each step's outputs match next step's inputs
        for i in range(len(self.steps) - 1):
            current_outputs = set(self.steps[i].outputs.keys())
            next_inputs = set(self.steps[i + 1].inputs.keys())
            
            # At least one output should be used in next step
            if not current_outputs.intersection(next_inputs):
                return False
        
        return True


class ProductionQueue(BaseModel):
    """Queue of production jobs"""
    queue_id: str
    crafter_npc_id: str
    current_job: Optional[str] = None  # ProductionChain chain_id
    pending_jobs: List[str] = Field(default_factory=list)
    completed_jobs: List[Dict] = Field(default_factory=list)
    started_at: Optional[datetime] = None
    
    def add_job(self, chain_id: str) -> None:
        """Add production job to queue"""
        if self.current_job is None:
            self.current_job = chain_id
            self.started_at = datetime.utcnow()
        else:
            self.pending_jobs.append(chain_id)
    
    def complete_current_job(self) -> Optional[str]:
        """
        Complete current job and start next
        
        Returns:
            Completed job chain_id or None
        """
        if self.current_job is None:
            return None
        
        completed = self.current_job
        self.completed_jobs.append({
            "chain_id": completed,
            "completed_at": datetime.utcnow().isoformat()
        })
        
        # Start next job
        if self.pending_jobs:
            self.current_job = self.pending_jobs.pop(0)
            self.started_at = datetime.utcnow()
        else:
            self.current_job = None
            self.started_at = None
        
        return completed


# Pre-defined production chains
PRODUCTION_CHAINS = {
    "iron_sword": ProductionChain(
        chain_id="iron_sword",
        chain_name="Iron Sword Production",
        steps=[
            ProductionStep(
                step_id="smelt_iron",
                inputs={"iron_ore": 5, "coal": 2},
                outputs={"iron_ingot": 2},
                labor_cost=100.0,
                time_required=300,
                skill_required="smithing",
                success_rate=0.95
            ),
            ProductionStep(
                step_id="forge_blade",
                inputs={"iron_ingot": 2},
                outputs={"iron_sword": 1},
                labor_cost=200.0,
                time_required=600,
                skill_required="weaponsmithing",
                success_rate=0.90
            )
        ],
        final_product="iron_sword",
        category="weapons",
        difficulty=3
    ),
    "health_potion": ProductionChain(
        chain_id="health_potion",
        chain_name="Health Potion Production",
        steps=[
            ProductionStep(
                step_id="gather_herbs",
                inputs={"red_herb": 3, "blue_herb": 1},
                outputs={"herb_mixture": 1},
                labor_cost=50.0,
                time_required=120,
                skill_required="alchemy",
                success_rate=0.98
            ),
            ProductionStep(
                step_id="brew_potion",
                inputs={"herb_mixture": 1, "empty_bottle": 1},
                outputs={"health_potion": 1},
                labor_cost=75.0,
                time_required=180,
                skill_required="alchemy",
                success_rate=0.95
            )
        ],
        final_product="health_potion",
        category="consumables",
        difficulty=2
    ),
    "steel_armor": ProductionChain(
        chain_id="steel_armor",
        chain_name="Steel Armor Production",
        steps=[
            ProductionStep(
                step_id="smelt_steel",
                inputs={"iron_ore": 10, "coal": 5, "carbon": 2},
                outputs={"steel_ingot": 3},
                labor_cost=300.0,
                time_required=900,
                skill_required="advanced_smithing",
                success_rate=0.85
            ),
            ProductionStep(
                step_id="forge_plates",
                inputs={"steel_ingot": 3},
                outputs={"steel_plate": 5},
                labor_cost=250.0,
                time_required=600,
                skill_required="advanced_smithing",
                success_rate=0.90
            ),
            ProductionStep(
                step_id="assemble_armor",
                inputs={"steel_plate": 5, "leather": 2},
                outputs={"steel_armor": 1},
                labor_cost=400.0,
                time_required=1200,
                skill_required="armorsmithing",
                success_rate=0.88
            )
        ],
        final_product="steel_armor",
        category="armor",
        difficulty=7
    )
}