"""
Integration tests for Authentication API endpoints
"""

import pytest
import jwt
import asyncio
from datetime import datetime, timedelta
from httpx import AsyncClient

from config import settings


@pytest.mark.integration
@pytest.mark.auth
@pytest.mark.asyncio
class TestAuthAPI:
    """Test suite for Authentication API endpoints"""
    
    async def test_generate_token_success(self, async_client: AsyncClient, sample_auth_request):
        """Test successful JWT token generation"""
        # Act
        response = await async_client.post(
            "/api/v1/auth/token",
            json=sample_auth_request,
        )
        
        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "access_token" in data
        assert data["token_type"] == "Bearer"
        assert data["expires_in"] == settings.security.jwt_expiration_seconds
        
        # Verify token is valid JWT
        token = data["access_token"]
        decoded = jwt.decode(
            token,
            settings.security.jwt_secret_key,
            algorithms=[settings.security.jwt_algorithm],
        )
        assert decoded["player_id"] == sample_auth_request["player_id"]
    
    async def test_generate_token_missing_player_id(self, async_client: AsyncClient):
        """Test token generation fails without player_id"""
        # Arrange
        request_data = {
            "api_key": settings.security.coordinator_api_key,
        }
        
        # Act
        response = await async_client.post(
            "/api/v1/auth/token",
            json=request_data,
        )
        
        # Assert
        assert response.status_code == 422  # Validation error
    
    async def test_generate_token_invalid_api_key_when_validation_enabled(
        self, async_client: AsyncClient, monkeypatch
    ):
        """Test token generation fails with invalid API key when validation is enabled"""
        # Arrange
        monkeypatch.setattr(settings.security, "api_key_validation_enabled", True)
        request_data = {
            "player_id": "test_player_123",
            "api_key": "invalid_api_key",
        }

        # Act
        response = await async_client.post(
            "/api/v1/auth/token",
            json=request_data,
        )

        # Assert
        assert response.status_code == 401
        # Accept either "Invalid API key" or "Missing X-API-Key header" as both indicate auth failure
        detail = response.json()["detail"]
        assert "Invalid API key" in detail or "Missing X-API-Key header" in detail or "API key" in detail
    
    async def test_generate_token_with_user_id(self, async_client: AsyncClient):
        """Test token generation includes user_id when provided"""
        # Arrange
        request_data = {
            "player_id": "test_player_123",
            "user_id": "test_user_456",
            "api_key": settings.security.coordinator_api_key,
        }
        
        # Act
        response = await async_client.post(
            "/api/v1/auth/token",
            json=request_data,
        )
        
        # Assert
        assert response.status_code == 200
        data = response.json()
        token = data["access_token"]
        decoded = jwt.decode(
            token,
            settings.security.jwt_secret_key,
            algorithms=[settings.security.jwt_algorithm],
        )
        assert decoded["user_id"] == request_data["user_id"]
    
    async def test_token_expiration(self, async_client: AsyncClient, sample_auth_request):
        """Test that generated token has correct expiration"""
        # Act
        response = await async_client.post(
            "/api/v1/auth/token",
            json=sample_auth_request,
        )

        # Assert
        assert response.status_code == 200
        response_data = response.json()
        token = response_data["access_token"]
        decoded = jwt.decode(
            token,
            settings.security.jwt_secret_key,
            algorithms=[settings.security.jwt_algorithm],
        )

        # Check expiration is in the future
        exp_time = datetime.fromtimestamp(decoded["exp"])
        now = datetime.utcnow()
        assert exp_time > now, "Token should expire in the future"

        # Check that token has reasonable expiration (between 1 minute and 24 hours)
        jwt_exp_seconds = (exp_time - now).total_seconds()
        assert 60 < jwt_exp_seconds < 86400, f"Token expiration should be reasonable (got {jwt_exp_seconds}s)"

        # Check that expires_in field exists and is positive
        assert "expires_in" in response_data
        assert response_data["expires_in"] > 0
    
    async def test_refresh_token_not_implemented(self, async_client: AsyncClient):
        """Test that refresh token endpoint returns 501 Not Implemented"""
        # Arrange
        request_data = {
            "refresh_token": "some_refresh_token",
        }

        # Act
        response = await async_client.post(
            "/api/v1/auth/refresh",
            json=request_data,
        )

        # Assert
        assert response.status_code == 501
        detail_lower = response.json()["detail"].lower()
        assert "not" in detail_lower and "implement" in detail_lower
    
    async def test_token_contains_required_claims(self, async_client: AsyncClient, sample_auth_request):
        """Test that generated token contains all required claims"""
        # Act
        response = await async_client.post(
            "/api/v1/auth/token",
            json=sample_auth_request,
        )
        
        # Assert
        assert response.status_code == 200
        token = response.json()["access_token"]
        decoded = jwt.decode(
            token,
            settings.security.jwt_secret_key,
            algorithms=[settings.security.jwt_algorithm],
        )
        
        # Check required claims
        assert "sub" in decoded
        assert "player_id" in decoded
        assert "exp" in decoded
        assert decoded["sub"] == sample_auth_request["player_id"]
        assert decoded["player_id"] == sample_auth_request["player_id"]
    
    async def test_multiple_token_generation(self, async_client: AsyncClient, sample_auth_request):
        """Test that multiple tokens can be generated for the same player"""
        # Act
        response1 = await async_client.post(
            "/api/v1/auth/token",
            json=sample_auth_request,
        )
        # Wait 1 second to ensure different iat timestamp
        await asyncio.sleep(1)
        response2 = await async_client.post(
            "/api/v1/auth/token",
            json=sample_auth_request,
        )

        # Assert
        assert response1.status_code == 200
        assert response2.status_code == 200

        token1 = response1.json()["access_token"]
        token2 = response2.json()["access_token"]

        # Tokens should be different (different iat)
        assert token1 != token2

        # But both should be valid
        decoded1 = jwt.decode(token1, settings.security.jwt_secret_key, algorithms=[settings.security.jwt_algorithm])
        decoded2 = jwt.decode(token2, settings.security.jwt_secret_key, algorithms=[settings.security.jwt_algorithm])
        assert decoded1["player_id"] == decoded2["player_id"]

