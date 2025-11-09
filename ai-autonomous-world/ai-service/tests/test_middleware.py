"""
Tests for middleware components
"""

import pytest
from fastapi import FastAPI
from fastapi.testclient import TestClient
from unittest.mock import AsyncMock, MagicMock, patch

from middleware.request_size import RequestSizeLimitMiddleware
from middleware.rate_limit import RateLimitMiddleware
from middleware.auth import APIKeyMiddleware


class TestRequestSizeLimitMiddleware:
    """Test request size limit middleware"""
    
    def test_request_within_limit(self):
        """Test request within size limit passes through"""
        app = FastAPI()
        app.add_middleware(RequestSizeLimitMiddleware)
        
        @app.post("/test")
        async def test_endpoint(data: dict):
            return {"status": "ok"}
        
        client = TestClient(app)
        
        # Small request should pass
        response = client.post("/test", json={"test": "data"})
        assert response.status_code == 200
    
    def test_request_exceeds_limit(self):
        """Test request exceeding size limit is rejected"""
        app = FastAPI()
        app.add_middleware(RequestSizeLimitMiddleware)
        
        @app.post("/test")
        async def test_endpoint(data: dict):
            return {"status": "ok"}
        
        client = TestClient(app)
        
        # Create a large payload (default limit is 10MB)
        large_data = {"data": "x" * (11 * 1024 * 1024)}  # 11MB
        
        # Should be rejected with 413
        response = client.post("/test", json=large_data)
        assert response.status_code == 413
        assert "Request too large" in response.json()["error"]
    
    def test_request_without_content_length(self):
        """Test request without content-length header passes through"""
        app = FastAPI()
        app.add_middleware(RequestSizeLimitMiddleware)
        
        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}
        
        client = TestClient(app)
        
        # GET request without body should pass
        response = client.get("/test")
        assert response.status_code == 200


class TestRateLimitMiddleware:
    """Test rate limit middleware"""

    @pytest.mark.asyncio
    async def test_rate_limit_not_exceeded(self, mock_database):
        """Test request within rate limit passes through"""
        # Mock rate limit check to return False (not limited)
        mock_database.get.return_value = None

        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}

        client = TestClient(app)

        with patch('middleware.rate_limit.db.client', mock_database):
            response = client.get("/test")
            assert response.status_code == 200

    def test_rate_limit_disabled(self):
        """Test rate limiting when disabled in settings"""
        from config import settings

        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}

        client = TestClient(app)

        # Mock settings to disable rate limiting
        with patch.object(settings, 'rate_limit_enabled', False):
            response = client.get("/test")
            assert response.status_code == 200

    def test_rate_limit_exempt_paths(self):
        """Test exempt paths bypass rate limiting"""
        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/health")
        async def health_endpoint():
            return {"status": "healthy"}

        client = TestClient(app)

        # Exempt paths should not be rate limited
        response = client.get("/health")
        assert response.status_code == 200

    @pytest.mark.asyncio
    async def test_rate_limit_first_request(self, mock_database):
        """Test first request initializes rate limit counter"""
        # Mock first request (no existing counter)
        mock_database.get.return_value = None
        mock_database.setex = AsyncMock()

        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}

        client = TestClient(app)

        with patch('middleware.rate_limit.db.client', mock_database):
            response = client.get("/test")
            assert response.status_code == 200
            # Should have called setex to initialize counter
            mock_database.setex.assert_called_once()

    @pytest.mark.asyncio
    async def test_rate_limit_exceeded(self, mock_database):
        """Test rate limit exceeded returns 429"""
        from config import settings

        # Mock rate limit exceeded (count >= limit)
        mock_database.get.return_value = str(settings.rate_limit_requests)

        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}

        client = TestClient(app)

        with patch('middleware.rate_limit.db.client', mock_database):
            response = client.get("/test")
            assert response.status_code == 429
            assert "Rate limit exceeded" in response.json()["error"]
            assert "Retry-After" in response.headers

    @pytest.mark.asyncio
    async def test_rate_limit_increment(self, mock_database):
        """Test rate limit counter increments"""
        from config import settings

        # Mock existing counter below limit
        mock_database.get.return_value = "5"
        mock_database.incr = AsyncMock()

        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}

        client = TestClient(app)

        with patch('middleware.rate_limit.db.client', mock_database):
            response = client.get("/test")
            assert response.status_code == 200
            # Should have incremented counter
            mock_database.incr.assert_called_once()
            # Should have rate limit headers
            assert "X-RateLimit-Limit" in response.headers
            assert "X-RateLimit-Remaining" in response.headers

    @pytest.mark.asyncio
    async def test_rate_limit_error_handling(self, mock_database):
        """Test rate limit error handling (fail open)"""
        # Mock database error
        mock_database.get.side_effect = Exception("Database connection failed")

        app = FastAPI()
        app.add_middleware(RateLimitMiddleware)

        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}

        client = TestClient(app)

        with patch('middleware.rate_limit.db.client', mock_database):
            # Should allow request to proceed despite error (fail open)
            response = client.get("/test")
            assert response.status_code == 200


class TestAPIKeyMiddleware:
    """Test API key middleware"""

    def test_api_key_middleware_optional(self):
        """Test API key middleware allows requests when auth is optional"""
        app = FastAPI()
        app.add_middleware(APIKeyMiddleware)
        
        @app.get("/test")
        async def test_endpoint():
            return {"status": "ok"}
        
        client = TestClient(app)
        
        # Request without auth should pass (auth is optional by default)
        response = client.get("/test")
        assert response.status_code == 200

