"""
Test Suite for Economy Router

Tests cover:
- Market price retrieval
- Transaction processing
- Market trend analysis
- Economic event triggering
- Authentication and validation
- Error handling
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, MagicMock, patch
from fastapi import HTTPException


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def mock_price_data():
    """Mock price information."""
    return {
        "item_id": "health_potion",
        "current_price": 50.0,
        "base_price": 45.0,
        "supply": 1000,
        "demand": 0.7,
        "trend": "rising",
        "last_updated": datetime.utcnow()
    }


@pytest.fixture
def sample_transaction():
    """Sample transaction data."""
    return {
        "transaction_type": "buy",
        "item_id": "sword",
        "quantity": 1,
        "price": 100.0,
        "buyer_id": "player_123",
        "seller_id": "npc_456"
    }


# ============================================================================
# PRICE RETRIEVAL TESTS
# ============================================================================

@pytest.mark.unit
def test_get_prices_all_items(test_client):
    """Test getting prices for all items."""
    response = test_client.get("/api/economy/prices")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "prices" in data["data"]


@pytest.mark.unit
def test_get_prices_specific_items(test_client):
    """Test getting prices for specific items."""
    response = test_client.get("/api/economy/prices?item_ids=potion,sword")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_get_prices_single_item(test_client):
    """Test getting price for single item."""
    response = test_client.get("/api/economy/prices?item_ids=potion")
    
    assert response.status_code == 200
    data = response.json()
    assert "prices" in data["data"]


@pytest.mark.unit
def test_get_prices_with_auth(authenticated_client):
    """Test price retrieval with authentication."""
    response = authenticated_client.get("/api/economy/prices")
    
    assert response.status_code == 200


# ============================================================================
# TRANSACTION PROCESSING TESTS
# ============================================================================

@pytest.mark.unit
def test_process_buy_transaction(test_client, sample_transaction):
    """Test processing a buy transaction."""
    response = test_client.post("/api/economy/transaction", json=sample_transaction)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "transaction_id" in data["data"]
    assert data["data"]["type"] == "buy"


@pytest.mark.unit
def test_process_sell_transaction(test_client):
    """Test processing a sell transaction."""
    transaction = {
        "transaction_type": "sell",
        "item_id": "ore",
        "quantity": 10,
        "price": 5.0
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 200
    data = response.json()
    assert data["data"]["type"] == "sell"


@pytest.mark.unit
def test_process_trade_transaction(test_client):
    """Test processing a trade transaction."""
    transaction = {
        "transaction_type": "trade",
        "item_id": "gem",
        "quantity": 1,
        "price": 1000.0,
        "buyer_id": "player_1",
        "seller_id": "player_2"
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_transaction_missing_required_fields(test_client):
    """Test transaction with missing required fields."""
    transaction = {
        "transaction_type": "buy"
        # Missing item_id and quantity
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 422  # Validation error


@pytest.mark.unit
def test_transaction_invalid_quantity(test_client):
    """Test transaction with invalid quantity."""
    transaction = {
        "transaction_type": "buy",
        "item_id": "item",
        "quantity": 0  # Invalid: must be > 0
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_transaction_negative_price(test_client):
    """Test transaction with negative price."""
    transaction = {
        "transaction_type": "buy",
        "item_id": "item",
        "quantity": 1,
        "price": -50.0  # Invalid
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 422


# ============================================================================
# MARKET TRENDS TESTS
# ============================================================================

@pytest.mark.unit
def test_get_market_trends_all(test_client):
    """Test getting trends for all items."""
    response = test_client.get("/api/economy/trends")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_get_market_trends_specific_item(test_client):
    """Test getting trends for specific item."""
    response = test_client.get("/api/economy/trends?item_id=potion")
    
    assert response.status_code == 200
    data = response.json()
    assert "trend" in data["data"]


@pytest.mark.unit
def test_market_trends_includes_predictions(test_client):
    """Test that trends include price predictions."""
    response = test_client.get("/api/economy/trends?item_id=sword")
    
    assert response.status_code == 200
    data = response.json()
    assert "prediction" in data["data"]


# ============================================================================
# ECONOMIC EVENT TESTS
# ============================================================================

@pytest.mark.unit
def test_trigger_inflation_event(test_client):
    """Test triggering inflation economic event."""
    event = {
        "event_type": "inflation",
        "affected_items": ["all"],
        "magnitude": 0.1,
        "duration": 3600
    }
    
    response = test_client.post("/api/economy/event", json=event)
    
    assert response.status_code == 200
    data = response.json()
    assert "event_id" in data["data"]
    assert data["data"]["event_type"] == "inflation"


@pytest.mark.unit
def test_trigger_shortage_event(test_client):
    """Test triggering shortage event."""
    event = {
        "event_type": "shortage",
        "affected_items": ["potion", "food"],
        "magnitude": 0.5
    }
    
    response = test_client.post("/api/economy/event", json=event)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_trigger_surplus_event(test_client):
    """Test triggering surplus event."""
    event = {
        "event_type": "surplus",
        "affected_items": ["ore"],
        "magnitude": 0.3,
        "duration": 7200
    }
    
    response = test_client.post("/api/economy/event", json=event)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_economic_event_magnitude_validation(test_client):
    """Test event magnitude validation."""
    event = {
        "event_type": "inflation",
        "affected_items": ["all"],
        "magnitude": 1.5  # Invalid: > 1.0
    }
    
    response = test_client.post("/api/economy/event", json=event)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_economic_event_missing_magnitude(test_client):
    """Test event without magnitude."""
    event = {
        "event_type": "inflation",
        "affected_items": ["all"]
    }
    
    response = test_client.post("/api/economy/event", json=event)
    
    assert response.status_code == 422


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_transaction_server_error_handling(test_client):
    """Test transaction with simulated server error."""
    transaction = {
        "transaction_type": "buy",
        "item_id": "error_item",
        "quantity": 1
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    # Should handle gracefully
    assert response.status_code in [200, 500]


@pytest.mark.unit
def test_invalid_transaction_type(test_client):
    """Test transaction with invalid type."""
    transaction = {
        "transaction_type": "invalid_type",
        "item_id": "item",
        "quantity": 1
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    # Should be validation error
    assert response.status_code in [400, 422]


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_price_to_transaction_flow(test_client):
    """Test flow from checking price to making transaction."""
    # Check price
    price_response = test_client.get("/api/economy/prices?item_ids=sword")
    assert price_response.status_code == 200
    
    # Make transaction
    transaction = {
        "transaction_type": "buy",
        "item_id": "sword",
        "quantity": 1,
        "price": 100.0
    }
    
    txn_response = test_client.post("/api/economy/transaction", json=transaction)
    assert txn_response.status_code == 200


@pytest.mark.integration
def test_event_to_trend_analysis(test_client):
    """Test economic event affecting trends."""
    # Trigger event
    event = {
        "event_type": "inflation",
        "affected_items": ["all"],
        "magnitude": 0.2
    }
    
    event_response = test_client.post("/api/economy/event", json=event)
    assert event_response.status_code == 200
    
    # Check trends
    trend_response = test_client.get("/api/economy/trends")
    assert trend_response.status_code == 200


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_transaction_with_zero_price(test_client):
    """Test transaction with zero price (free item)."""
    transaction = {
        "transaction_type": "buy",
        "item_id": "free_item",
        "quantity": 1,
        "price": 0.0
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_large_quantity_transaction(test_client):
    """Test transaction with large quantity."""
    transaction = {
        "transaction_type": "buy",
        "item_id": "arrow",
        "quantity": 10000,
        "price": 0.1
    }
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_event_with_empty_affected_items(test_client):
    """Test economic event with no affected items."""
    event = {
        "event_type": "inflation",
        "affected_items": [],
        "magnitude": 0.1
    }
    
    response = test_client.post("/api/economy/event", json=event)
    
    assert response.status_code in [200, 422]


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_price_retrieval_performance(test_client, performance_timer):
    """Test price retrieval performance."""
    performance_timer.start()
    
    response = test_client.get("/api/economy/prices")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500


@pytest.mark.performance
def test_transaction_processing_performance(test_client, performance_timer):
    """Test transaction processing performance."""
    transaction = {
        "transaction_type": "buy",
        "item_id": "item",
        "quantity": 1,
        "price": 10.0
    }
    
    performance_timer.start()
    
    response = test_client.post("/api/economy/transaction", json=transaction)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000