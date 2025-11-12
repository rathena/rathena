"""
Shop Restocking Tasks - NPC-Driven Inventory Management
Shop owner NPCs make intelligent restocking decisions based on sales data
"""

import asyncio
from datetime import datetime, timedelta
from typing import List, Dict, Any, Optional
from loguru import logger

from config import settings
from database import db, postgres_db
from agents.decision_agent import DecisionAgent
from agents.base_agent import AgentContext


class ShopRestockManager:
    """
    Manages shop restocking with NPC-driven decisions
    Shop owner NPCs analyze sales and decide when/what/how much to restock
    """
    
    def __init__(self):
        self.decision_agent = DecisionAgent()
        logger.info("ShopRestockManager initialized")
    
    async def check_npc_driven_restocking(self):
        """
        Check all shops and let shop owner NPCs decide if restocking is needed
        Called periodically by scheduler in npc_driven mode
        """
        if not settings.shop_restock_enabled or settings.shop_restock_mode != "npc_driven":
            return
        
        logger.debug("Checking NPC-driven shop restocking...")
        
        try:
            # Get all active shops
            shops = await self._get_active_shops()
            
            restocked_count = 0
            
            for shop in shops:
                shop_id = shop['shop_id']
                owner_npc_id = shop.get('owner_npc_id')
                
                if not owner_npc_id:
                    logger.debug(f"Shop {shop_id} has no owner NPC, skipping")
                    continue
                
                # Shop owner NPC decides if restocking is needed
                should_restock, restock_plan = await self._npc_decides_restocking(
                    shop_id=shop_id,
                    owner_npc_id=owner_npc_id,
                    shop_data=shop
                )
                
                if should_restock:
                    await self._execute_restock(shop_id, owner_npc_id, restock_plan)
                    restocked_count += 1
            
            if restocked_count > 0:
                logger.info(f"NPC-driven restocking: {restocked_count}/{len(shops)} shops restocked")
        
        except Exception as e:
            logger.error(f"Error in NPC-driven restocking check: {e}", exc_info=True)
    
    async def restock_all_shops_fixed(self):
        """
        Restock all shops in fixed interval mode
        Uses simple rules instead of NPC decisions
        """
        if not settings.shop_restock_enabled or settings.shop_restock_mode != "fixed_interval":
            return
        
        logger.debug("Fixed interval shop restocking...")
        
        try:
            shops = await self._get_active_shops()
            restocked_count = 0
            
            for shop in shops:
                shop_id = shop['shop_id']
                
                # Simple rule: restock if inventory below threshold
                inventory = await self._get_shop_inventory(shop_id)
                
                if self._needs_restock_simple(inventory):
                    await self._execute_simple_restock(shop_id)
                    restocked_count += 1
            
            logger.info(f"Fixed interval restocking: {restocked_count}/{len(shops)} shops restocked")
        
        except Exception as e:
            logger.error(f"Error in fixed interval restocking: {e}", exc_info=True)
    
    async def restock_shop_with_npc_decision(
        self,
        shop_id: str,
        force: bool = False
    ) -> Dict[str, Any]:
        """
        Restock specific shop with NPC decision making
        Called from API endpoint
        """
        try:
            # Get shop data
            shop = await self._get_shop_data(shop_id)
            
            if not shop:
                raise ValueError(f"Shop {shop_id} not found")
            
            owner_npc_id = shop.get('owner_npc_id')
            
            if not owner_npc_id:
                raise ValueError(f"Shop {shop_id} has no owner NPC")
            
            # NPC decides restocking
            should_restock, restock_plan = await self._npc_decides_restocking(
                shop_id=shop_id,
                owner_npc_id=owner_npc_id,
                shop_data=shop,
                force=force
            )
            
            if not should_restock and not force:
                return {
                    "success": True,
                    "restocked": False,
                    "reason": "NPC decided restocking not needed",
                    "npc_decision": restock_plan
                }
            
            # Execute restocking
            result = await self._execute_restock(shop_id, owner_npc_id, restock_plan)
            
            return {
                "success": True,
                "restocked": True,
                "items_restocked": result.get("items_restocked", 0),
                "total_cost": result.get("total_cost", 0),
                "npc_decision": restock_plan
            }
        
        except Exception as e:
            logger.error(f"Error restocking shop {shop_id}: {e}", exc_info=True)
            raise
    
    # ========================================================================
    # Helper Methods
    # ========================================================================
    
    async def _npc_decides_restocking(
        self,
        shop_id: str,
        owner_npc_id: str,
        shop_data: Dict[str, Any],
        force: bool = False
    ) -> tuple[bool, Dict[str, Any]]:
        """
        Shop owner NPC decides if and what to restock
        Uses DecisionAgent with shop-specific context
        """
        try:
            # Get shop inventory
            inventory = await self._get_shop_inventory(shop_id)
            
            # Get sales history
            sales_history = await self._get_sales_history(shop_id, days=7)
            
            # Get NPC's past restocking decisions for learning
            past_decisions = await self._get_past_restock_decisions(
                owner_npc_id,
                limit=settings.agent_context_window_size
            )
            
            # Build context for NPC decision
            context = AgentContext(
                npc_id=owner_npc_id,
                task_type="shop_restocking_decision",
                shop_id=shop_id,
                shop_data=shop_data,
                current_inventory=inventory,
                sales_history=sales_history,
                past_decisions=past_decisions,
                learning_enabled=settings.shop_npc_learning_enabled,
                force=force
            )
            
            # NPC makes decision
            decision = await self.decision_agent.process(context)
            
            if not decision.success:
                logger.warning(f"NPC {owner_npc_id} decision failed for shop {shop_id}")
                return False, {}
            
            should_restock = decision.data.get('should_restock', False) or force
            restock_plan = decision.data.get('restock_plan', {})
            
            # Store decision for learning
            if settings.agent_decision_logging_enabled:
                await self._store_restock_decision(owner_npc_id, shop_id, decision.data, context)
            
            logger.debug(f"NPC {owner_npc_id} decided: restock={should_restock} for shop {shop_id}")
            
            return should_restock, restock_plan
        
        except Exception as e:
            logger.error(f"Error in NPC restocking decision: {e}")
            return False, {}

    async def _get_active_shops(self) -> List[Dict[str, Any]]:
        """Get all active shops"""
        try:
            cache_key = "shops:active"
            cached = await db.get(cache_key)

            if cached:
                return cached

            query = """
                SELECT * FROM shops
                WHERE is_active = true
                ORDER BY last_restock_time ASC
            """
            results = await postgres_db.fetch_all(query)

            shops = [dict(row) for row in results]

            # Cache for 5 minutes
            await db.set(cache_key, shops, expire=300)

            return shops

        except Exception as e:
            logger.error(f"Error getting active shops: {e}")
            return []

    async def _get_shop_data(self, shop_id: str) -> Optional[Dict[str, Any]]:
        """Get shop data"""
        try:
            cache_key = f"shop:{shop_id}"
            cached = await db.get(cache_key)

            if cached:
                return cached

            query = "SELECT * FROM shops WHERE shop_id = $1"
            result = await postgres_db.fetch_one(query, shop_id)

            if result:
                shop_data = dict(result)
                await db.set(cache_key, shop_data, expire=300)
                return shop_data

            return None

        except Exception as e:
            logger.error(f"Error getting shop data: {e}")
            return None

    async def _get_shop_inventory(self, shop_id: str) -> Dict[str, Any]:
        """Get shop inventory"""
        try:
            query = """
                SELECT item_id, quantity, min_stock, max_stock
                FROM shop_inventory
                WHERE shop_id = $1
            """
            results = await postgres_db.fetch_all(query, shop_id)

            inventory = {}
            for row in results:
                inventory[row['item_id']] = {
                    'quantity': row['quantity'],
                    'min_stock': row['min_stock'],
                    'max_stock': row['max_stock']
                }

            return inventory

        except Exception as e:
            logger.error(f"Error getting shop inventory: {e}")
            return {}

    async def _get_sales_history(self, shop_id: str, days: int = 7) -> List[Dict[str, Any]]:
        """Get sales history for analysis"""
        try:
            query = """
                SELECT * FROM shop_sales_history
                WHERE shop_id = $1
                AND sale_time >= $2
                ORDER BY sale_time DESC
            """
            cutoff = datetime.utcnow() - timedelta(days=days)
            results = await postgres_db.fetch_all(query, shop_id, cutoff)

            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting sales history: {e}")
            return []

    async def _get_past_restock_decisions(self, owner_npc_id: str, limit: int = 10) -> List[Dict[str, Any]]:
        """Get past restocking decisions for learning"""
        if not settings.shop_npc_learning_enabled:
            return []

        try:
            query = """
                SELECT * FROM shop_restock_history
                WHERE owner_npc_id = $1
                ORDER BY timestamp DESC
                LIMIT $2
            """
            results = await postgres_db.fetch_all(query, owner_npc_id, limit)
            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting past restock decisions: {e}")
            return []

    async def _execute_restock(
        self,
        shop_id: str,
        owner_npc_id: str,
        restock_plan: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Execute restocking based on NPC's plan"""
        try:
            items_restocked = 0
            total_cost = 0

            # Get items to restock from plan
            items = restock_plan.get('items', [])

            for item in items:
                item_id = item['item_id']
                quantity = item['quantity']
                cost_per_item = item.get('cost', 0)

                # Update inventory
                await self._add_to_inventory(shop_id, item_id, quantity)

                items_restocked += 1
                total_cost += quantity * cost_per_item

            # Update shop last restock time
            await self._update_shop_restock_time(shop_id)

            # Record restocking outcome for learning
            await self._record_restock_outcome(
                shop_id=shop_id,
                owner_npc_id=owner_npc_id,
                items_restocked=items_restocked,
                total_cost=total_cost,
                restock_plan=restock_plan
            )

            logger.info(f"Shop {shop_id} restocked: {items_restocked} items, cost: {total_cost}")

            return {
                "items_restocked": items_restocked,
                "total_cost": total_cost
            }

        except Exception as e:
            logger.error(f"Error executing restock for shop {shop_id}: {e}")
            return {"items_restocked": 0, "total_cost": 0}

    async def _needs_restock_simple(self, inventory: Dict[str, Any]) -> bool:
        """Simple rule-based check if restocking needed"""
        for item_id, item_data in inventory.items():
            if item_data['quantity'] < item_data['min_stock']:
                return True
        return False

    async def _execute_simple_restock(self, shop_id: str):
        """Simple restocking without NPC decision"""
        try:
            inventory = await self._get_shop_inventory(shop_id)

            for item_id, item_data in inventory.items():
                if item_data['quantity'] < item_data['min_stock']:
                    # Restock to max
                    quantity_to_add = item_data['max_stock'] - item_data['quantity']
                    await self._add_to_inventory(shop_id, item_id, quantity_to_add)

            await self._update_shop_restock_time(shop_id)

        except Exception as e:
            logger.error(f"Error in simple restock for shop {shop_id}: {e}")

    async def _add_to_inventory(self, shop_id: str, item_id: str, quantity: int):
        """Add items to shop inventory"""
        try:
            query = """
                UPDATE shop_inventory
                SET quantity = quantity + $1
                WHERE shop_id = $2 AND item_id = $3
            """
            await postgres_db.execute(query, quantity, shop_id, item_id)

        except Exception as e:
            logger.error(f"Error adding to inventory: {e}")

    async def _update_shop_restock_time(self, shop_id: str):
        """Update shop's last restock time"""
        try:
            query = "UPDATE shops SET last_restock_time = $1 WHERE shop_id = $2"
            await postgres_db.execute(query, datetime.utcnow(), shop_id)

            # Invalidate cache
            await db.delete(f"shop:{shop_id}")

        except Exception as e:
            logger.error(f"Error updating restock time: {e}")

    async def _store_restock_decision(
        self,
        owner_npc_id: str,
        shop_id: str,
        decision_data: Dict[str, Any],
        context: AgentContext
    ):
        """Store restocking decision for learning"""
        try:
            query = """
                INSERT INTO shop_restock_history
                (owner_npc_id, shop_id, decision_data, context_data, timestamp)
                VALUES ($1, $2, $3, $4, $5)
            """
            await postgres_db.execute(
                query,
                owner_npc_id,
                shop_id,
                decision_data,
                context.dict(),
                datetime.utcnow()
            )

        except Exception as e:
            logger.error(f"Error storing restock decision: {e}")

    async def _record_restock_outcome(
        self,
        shop_id: str,
        owner_npc_id: str,
        items_restocked: int,
        total_cost: int,
        restock_plan: Dict[str, Any]
    ):
        """Record restocking outcome for learning"""
        try:
            query = """
                INSERT INTO shop_restock_outcomes
                (shop_id, owner_npc_id, items_restocked, total_cost, restock_plan, timestamp)
                VALUES ($1, $2, $3, $4, $5, $6)
            """
            await postgres_db.execute(
                query,
                shop_id,
                owner_npc_id,
                items_restocked,
                total_cost,
                restock_plan,
                datetime.utcnow()
            )

        except Exception as e:
            logger.error(f"Error recording restock outcome: {e}")


# Global instance
shop_restock_manager = ShopRestockManager()


# Public functions for scheduler and router
async def check_npc_driven_restocking():
    """Check NPC-driven shop restocking"""
    await shop_restock_manager.check_npc_driven_restocking()


async def restock_all_shops_fixed():
    """Restock all shops in fixed interval mode"""
    await shop_restock_manager.restock_all_shops_fixed()


async def restock_shop_with_npc_decision(shop_id: str, force: bool = False) -> Dict[str, Any]:
    """Restock shop with NPC decision"""
    return await shop_restock_manager.restock_shop_with_npc_decision(shop_id, force)


