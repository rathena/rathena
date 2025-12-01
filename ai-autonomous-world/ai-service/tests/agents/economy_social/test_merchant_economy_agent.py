"""
Unit tests for Merchant Economy Agent
Tests economic analysis, price adjustments, and zeny sink events
"""

import pytest
import json
from datetime import datetime, timedelta, UTC
from unittest.mock import AsyncMock, Mock, patch

from agents.economy_social.merchant_economy_agent import (
    MerchantEconomyAgent, 
    merchant_economy_agent,
    EconomicIndicator
)
from agents.base_agent import AgentResponse


@pytest.fixture
def sample_economy_data():
    """Sample economy data for testing"""
    return {
        "total_zeny": 500000000,
        "active_players": 50,
        "item_supply_index": 1.2,
        "item_demand_index": 1.0,
        "top_traded_items": ["607", "608", "985"],
        "farming_intensity": {"prt_fild01": 100, "prt_fild02": 80}
    }


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.economy_social.merchant_economy_agent.postgres_db') as mock_pg, \
         patch('agents.economy_social.merchant_economy_agent.db') as mock_cache:
        
        # Mock PostgreSQL methods
        mock_pg.fetch_one = AsyncMock(return_value=None)
        mock_pg.fetch_all = AsyncMock(return_value=[])
        mock_pg.execute = AsyncMock(return_value=1)
        
        # Mock cache methods
        mock_cache.get = AsyncMock(return_value=None)
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        
        yield {"postgres": mock_pg, "cache": mock_cache}


class TestMerchantEconomyAgent:
    """Test suite for Merchant Economy Agent"""
    
    @pytest.mark.asyncio
    async def test_initialization(self):
        """Test agent initialization"""
        agent = MerchantEconomyAgent()
        
        assert agent.agent_type == "merchant_economy"
        assert len(agent.zeny_sink_templates) == 4
        assert 'zeny_per_capita_inflation' in agent.thresholds
    
    @pytest.mark.asyncio
    async def test_analyze_economic_health_inflation(self):
        """Test inflation detection"""
        agent = MerchantEconomyAgent()
        
        # High zeny per capita = inflation
        inflation_data = {
            "total_zeny": 600000000,
            "active_players": 10,  # 60M per capita
            "item_supply_index": 1.0,
            "item_demand_index": 1.0
        }
        
        indicator = await agent.analyze_economic_health(inflation_data)
        
        assert indicator == EconomicIndicator.INFLATION
    
    @pytest.mark.asyncio
    async def test_analyze_economic_health_deflation(self):
        """Test deflation detection"""
        agent = MerchantEconomyAgent()
        
        # Low zeny per capita = deflation
        deflation_data = {
            "total_zeny": 5000000,
            "active_players": 100,  # 50K per capita
            "item_supply_index": 1.0,
            "item_demand_index": 1.0
        }
        
        indicator = await agent.analyze_economic_health(deflation_data)
        
        assert indicator == EconomicIndicator.DEFLATION
    
    @pytest.mark.asyncio
    async def test_analyze_economic_health_item_glut(self):
        """Test item oversupply detection"""
        agent = MerchantEconomyAgent()
        
        # High supply, low demand = glut
        glut_data = {
            "total_zeny": 500000000,
            "active_players": 50,
            "item_supply_index": 2.5,
            "item_demand_index": 1.0
        }
        
        indicator = await agent.analyze_economic_health(glut_data)
        
        assert indicator == EconomicIndicator.ITEM_GLUT
    
    @pytest.mark.asyncio
    async def test_analyze_economic_health_balanced(self, sample_economy_data):
        """Test balanced economy detection"""
        agent = MerchantEconomyAgent()
        
        indicator = await agent.analyze_economic_health(sample_economy_data)
        
        # Should be balanced or scarcity (depending on exact ratios)
        assert indicator in [EconomicIndicator.BALANCED, EconomicIndicator.ITEM_SCARCITY]
    
    @pytest.mark.asyncio
    async def test_adjust_merchant_prices_inflation(self, mock_db):
        """Test price adjustment for inflation"""
        agent = MerchantEconomyAgent()
        
        # Mock cache miss
        mock_db['cache'].get.return_value = None
        
        # Mock item price
        with patch.object(agent, '_get_item_price', return_value=1000):
            response = await agent.adjust_merchant_prices(
                indicator=EconomicIndicator.INFLATION,
                affected_items=["607", "608"]
            )
        
        assert response.success
        assert response.data['indicator'] == EconomicIndicator.INFLATION.value
        assert response.data['adjustment_percent'] > 0  # Positive for inflation
        assert response.data['items_adjusted'] == 2
    
    @pytest.mark.asyncio
    async def test_adjust_merchant_prices_cached(self, mock_db):
        """Test cached price adjustment"""
        agent = MerchantEconomyAgent()
        
        # Mock cached adjustment
        cached_data = {
            'indicator': 'inflation',
            'adjustment_percent': 25.0,
            'items_adjusted': 10,
            'timestamp': datetime.now(UTC).isoformat()
        }
        mock_db['cache'].get.return_value = cached_data
        
        response = await agent.adjust_merchant_prices(
            indicator=EconomicIndicator.INFLATION,
            affected_items=None
        )
        
        assert response.success
        assert "cached" in response.reasoning.lower()
    
    @pytest.mark.asyncio
    async def test_create_zeny_sink_event_high_severity(self, mock_db):
        """Test zeny sink creation for high severity inflation"""
        agent = MerchantEconomyAgent()
        
        # Mock database to return event ID
        mock_db['postgres'].fetch_one.return_value = {"event_id": 123}
        
        response = await agent.create_zeny_sink_event(severity=0.8)
        
        assert response.success
        assert response.data['event_id'] == 123
        assert response.data['event_type'] == 'card_packs'  # High severity
        assert response.data['target_zeny_drain'] > 0
    
    @pytest.mark.asyncio
    async def test_create_zeny_sink_event_low_severity(self, mock_db):
        """Test zeny sink creation for low severity"""
        agent = MerchantEconomyAgent()
        
        mock_db['postgres'].fetch_one.return_value = {"event_id": 124}
        
        response = await agent.create_zeny_sink_event(severity=0.15)
        
        assert response.success
        assert response.data['event_type'] in ['luxury_buffs', 'cosmetic_sale']
    
    @pytest.mark.asyncio
    async def test_recommend_drop_rate_changes(self, mock_db):
        """Test drop rate recommendation generation"""
        agent = MerchantEconomyAgent()
        
        oversupplied = ["607", "608", "985"]
        undersupplied = ["616", "617"]
        
        recommendations = await agent.recommend_drop_rate_changes(
            oversupplied_items=oversupplied,
            undersupplied_items=undersupplied
        )
        
        assert 'problem_agent' in recommendations
        assert 'event_agent' in recommendations
        assert 'treasure_agent' in recommendations
        assert len(recommendations['problem_agent']) > 0
    
    @pytest.mark.asyncio
    async def test_get_current_economy_snapshot(self, mock_db):
        """Test economy snapshot retrieval"""
        agent = MerchantEconomyAgent()
        
        # Mock database snapshot
        snapshot_data = {
            "snapshot_id": 1,
            "zeny_circulation": 500000000,
            "zeny_per_capita": 10000000,
            "active_players": 50,
            "inflation_rate": 1.05,
            "economic_indicator": "balanced",
            "timestamp": datetime.now(UTC)
        }
        mock_db['postgres'].fetch_one.return_value = snapshot_data
        
        snapshot = await agent.get_current_economy_snapshot()
        
        assert snapshot is not None
        assert snapshot['snapshot_id'] == 1
        assert snapshot['economic_indicator'] == "balanced"
    
    def test_calculate_price_adjustment_inflation(self):
        """Test price adjustment calculation for inflation"""
        agent = MerchantEconomyAgent()
        
        adjustment = agent._calculate_price_adjustment(EconomicIndicator.INFLATION)
        
        # Inflation = increase prices (+10% to +50%)
        assert 0.10 <= adjustment <= 0.50
    
    def test_calculate_price_adjustment_deflation(self):
        """Test price adjustment calculation for deflation"""
        agent = MerchantEconomyAgent()
        
        adjustment = agent._calculate_price_adjustment(EconomicIndicator.DEFLATION)
        
        # Deflation = decrease prices (-30% to -10%)
        assert -0.30 <= adjustment <= -0.10
    
    def test_calculate_price_adjustment_balanced(self):
        """Test price adjustment for balanced economy"""
        agent = MerchantEconomyAgent()
        
        adjustment = agent._calculate_price_adjustment(EconomicIndicator.BALANCED)
        
        # Balanced = no adjustment
        assert adjustment == 0.0
    
    @pytest.mark.asyncio
    async def test_generate_event_offerings_card_packs(self):
        """Test event offering generation for card packs"""
        agent = MerchantEconomyAgent()
        
        offerings = await agent._generate_event_offerings('card_packs', severity=0.8)
        
        assert len(offerings) > 0
        assert all('item_name' in offer for offer in offerings)
        assert all('price' in offer for offer in offerings)
        assert all('quantity_limit' in offer for offer in offerings)


@pytest.mark.asyncio
async def test_global_instance():
    """Test global agent instance"""
    assert merchant_economy_agent is not None
    assert merchant_economy_agent.agent_type == "merchant_economy"