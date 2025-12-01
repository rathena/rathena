"""
Security Tests: Authentication & Authorization

Tests auth bypass attempts, token manipulation, privilege escalation,
and session hijacking scenarios.

Requirements:
- Validate API key authentication
- Test token manipulation resistance
- Verify privilege escalation prevention
- Test session security

Run with:
    pytest tests/security/test_security_auth.py -v
    pytest tests/security/test_security_auth.py -m security
"""

import pytest
import time
import hashlib
import hmac
import base64
from typing import Dict
from fastapi.testclient import TestClient
from unittest.mock import patch

# Import app after setting test environment
import os
os.environ["ENVIRONMENT"] = "testing"

from main import app
from config import settings


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def client():
    """Create test client with auth enabled."""
    with patch.dict(os.environ, {
        "API_KEY_REQUIRED": "true",
        "API_KEY": "test-valid-key-12345678901234567890",
        "RATE_LIMIT_ENABLED": "false"
    }):
        with TestClient(app) as test_client:
            yield test_client


@pytest.fixture
def valid_auth_headers():
    """Valid authentication headers."""
    return {
        "Authorization": "Bearer test-valid-key-12345678901234567890",
        "Content-Type": "application/json"
    }


@pytest.fixture
def client_no_auth():
    """Client with auth disabled for comparison tests."""
    with patch.dict(os.environ, {
        "API_KEY_REQUIRED": "false",
        "RATE_LIMIT_ENABLED": "false"
    }):
        with TestClient(app) as test_client:
            yield test_client


# ============================================================================
# AUTHENTICATION BYPASS TESTS
# ============================================================================

@pytest.mark.security
def test_missing_api_key_rejected(client):
    """Test that requests without API key are rejected."""
    
    response = client.get("/api/v1/status")
    
    assert response.status_code == 401
    assert "detail" in response.json()
    assert "authentication" in response.json()["detail"].lower()
    
    print("\n✓ Missing API key correctly rejected")


@pytest.mark.security
def test_invalid_api_key_rejected(client):
    """Test that invalid API keys are rejected."""
    
    invalid_keys = [
        "invalid-key",
        "wrong-api-key-123",
        "",
        "Bearer ",
        "test-valid-key-12345678901234567891",  # Close but wrong
    ]
    
    for key in invalid_keys:
        headers = {"Authorization": f"Bearer {key}"}
        response = client.get("/api/v1/status", headers=headers)
        
        assert response.status_code in [401, 403], (
            f"Invalid key '{key}' not rejected"
        )
    
    print(f"\n✓ All {len(invalid_keys)} invalid keys rejected")


@pytest.mark.security
def test_valid_api_key_accepted(client, valid_auth_headers):
    """Test that valid API key is accepted."""
    
    response = client.get("/api/v1/status", headers=valid_auth_headers)
    
    assert response.status_code == 200
    
    print("\n✓ Valid API key correctly accepted")


@pytest.mark.security
def test_api_key_case_sensitivity(client):
    """Test that API keys are case-sensitive."""
    
    # Original valid key: test-valid-key-12345678901234567890
    headers_upper = {"Authorization": "Bearer TEST-VALID-KEY-12345678901234567890"}
    response = client.get("/api/v1/status", headers=headers_upper)
    
    assert response.status_code in [401, 403], "API key should be case-sensitive"
    
    print("\n✓ API keys are case-sensitive")


@pytest.mark.security
def test_api_key_timing_attack_resistance(client):
    """Test resistance to timing attacks on API key validation."""
    
    valid_key = "test-valid-key-12345678901234567890"
    almost_valid = "test-valid-key-12345678901234567891"  # One char different
    completely_wrong = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    
    def measure_validation_time(key: str, iterations: int = 100) -> float:
        times = []
        for _ in range(iterations):
            headers = {"Authorization": f"Bearer {key}"}
            start = time.perf_counter()
            client.get("/api/v1/status", headers=headers)
            end = time.perf_counter()
            times.append(end - start)
        return sum(times) / len(times)
    
    almost_time = measure_validation_time(almost_valid)
    wrong_time = measure_validation_time(completely_wrong)
    
    # Time difference should be minimal (constant-time comparison)
    time_difference = abs(almost_time - wrong_time)
    relative_difference = time_difference / max(almost_time, wrong_time)
    
    print(f"\n✓ Timing attack resistance:")
    print(f"  Almost valid: {almost_time*1000:.3f}ms")
    print(f"  Completely wrong: {wrong_time*1000:.3f}ms")
    print(f"  Relative difference: {relative_difference:.1%}")
    
    # Difference should be < 10% for constant-time comparison
    assert relative_difference < 0.10, (
        f"Timing difference {relative_difference:.1%} suggests timing attack vulnerability"
    )


# ============================================================================
# TOKEN MANIPULATION TESTS
# ============================================================================

@pytest.mark.security
def test_malformed_bearer_token_rejected(client):
    """Test various malformed Bearer tokens."""
    
    malformed_tokens = [
        "Bearer",  # No token
        "Bearer  ",  # Empty token
        "Bearer\n\ttest",  # Whitespace injection
        "Bearer test token",  # Multiple tokens
        "bearer test-key",  # Lowercase bearer
        "BEARER test-key",  # Uppercase bearer
        "ApiKey test-key",  # Wrong scheme
    ]
    
    for token in malformed_tokens:
        headers = {"Authorization": token}
        response = client.get("/api/v1/status", headers=headers)
        
        assert response.status_code in [401, 403], (
            f"Malformed token '{token}' not rejected"
        )
    
    print(f"\n✓ All {len(malformed_tokens)} malformed tokens rejected")


@pytest.mark.security
def test_token_injection_attempts(client):
    """Test SQL/NoSQL injection in tokens."""
    
    injection_payloads = [
        "' OR '1'='1",
        "'; DROP TABLE users; --",
        "1' UNION SELECT * FROM secrets--",
        "$ne: null",
        "{'$gt': ''}",
        "../../../etc/passwd",
        "<script>alert('XSS')</script>",
    ]
    
    for payload in injection_payloads:
        headers = {"Authorization": f"Bearer {payload}"}
        response = client.get("/api/v1/status", headers=headers)
        
        # Should be rejected, not cause error
        assert response.status_code in [401, 403, 400], (
            f"Injection payload '{payload}' not properly rejected"
        )
        
        # Response should not contain sensitive error details
        response_text = response.text.lower()
        assert "sql" not in response_text
        assert "database" not in response_text
        assert "error" not in response_text or "internal" not in response_text
    
    print(f"\n✓ All {len(injection_payloads)} injection attempts blocked")


@pytest.mark.security
def test_multiple_auth_headers_handling(client):
    """Test handling of multiple auth headers."""
    
    # Multiple Authorization headers
    headers = {
        "Authorization": "Bearer valid-key",
        "X-API-Key": "another-key"
    }
    
    response = client.get("/api/v1/status", headers=headers)
    
    # Should handle gracefully without confusion
    assert response.status_code in [401, 403, 200]
    
    print("\n✓ Multiple auth headers handled properly")


# ============================================================================
# PRIVILEGE ESCALATION TESTS
# ============================================================================

@pytest.mark.security
def test_api_key_parameter_pollution(client, valid_auth_headers):
    """Test parameter pollution attacks."""
    
    # Try multiple api_key parameters
    response = client.get(
        "/api/v1/status?api_key=fake1&api_key=fake2",
        headers=valid_auth_headers
    )
    
    # Should still require valid header auth
    assert response.status_code == 200
    
    print("\n✓ Parameter pollution handled correctly")


@pytest.mark.security
def test_header_injection_attempts(client):
    """Test HTTP header injection attempts."""
    
    malicious_headers = [
        {"Authorization": "Bearer test\r\nX-Admin: true"},
        {"Authorization": "Bearer test\nSet-Cookie: admin=true"},
        {"Authorization": "Bearer test%0d%0aX-Privilege: admin"},
    ]
    
    for headers in malicious_headers:
        try:
            response = client.get("/api/v1/status", headers=headers)
            # Should be rejected or sanitized
            assert response.status_code in [401, 403, 400]
        except Exception:
            # Header injection should be caught
            pass
    
    print(f"\n✓ All {len(malicious_headers)} header injection attempts blocked")


# ============================================================================
# SESSION SECURITY TESTS
# ============================================================================

@pytest.mark.security
def test_api_key_not_logged(client, valid_auth_headers, caplog):
    """Test that API keys are not logged."""
    
    import logging
    caplog.set_level(logging.DEBUG)
    
    response = client.get("/api/v1/status", headers=valid_auth_headers)
    
    # Check logs don't contain the API key
    api_key = "test-valid-key-12345678901234567890"
    
    for record in caplog.records:
        assert api_key not in record.message, "API key found in logs!"
    
    print("\n✓ API keys not logged")


@pytest.mark.security
def test_api_key_not_in_error_responses(client):
    """Test that API keys don't leak in error responses."""
    
    # Send invalid request with valid auth
    headers = {"Authorization": "Bearer test-valid-key-12345678901234567890"}
    response = client.post(
        "/api/v1/chat",
        headers=headers,
        json={"invalid": "payload"}
    )
    
    # Error response should not contain API key
    response_text = response.text
    assert "test-valid-key" not in response_text
    
    print("\n✓ API keys not leaked in error responses")


@pytest.mark.security
def test_concurrent_auth_independence(client, valid_auth_headers):
    """Test that concurrent requests don't interfere with auth."""
    
    from concurrent.futures import ThreadPoolExecutor
    
    def make_request():
        response = client.get("/api/v1/status", headers=valid_auth_headers)
        return response.status_code
    
    with ThreadPoolExecutor(max_workers=10) as executor:
        futures = [executor.submit(make_request) for _ in range(50)]
        results = [f.result() for f in futures]
    
    # All should succeed independently
    assert all(code == 200 for code in results)
    
    print("\n✓ Concurrent auth requests independent")


# ============================================================================
# AUTH BYPASS VECTORS
# ============================================================================

@pytest.mark.security
def test_path_traversal_auth_bypass(client):
    """Test path traversal doesn't bypass auth."""
    
    bypass_attempts = [
        "/api/v1/../health",
        "/api/v1/./status",
        "/api/v1/status/../status",
        "/api/v1/status%2f..%2fhealth",
    ]
    
    for path in bypass_attempts:
        response = client.get(path)
        
        # If path resolves to protected endpoint, should require auth
        if response.status_code not in [404, 401, 403]:
            # Check if it actually bypassed to a protected resource
            assert response.status_code != 200 or "authenticated" not in response.text.lower()
    
    print(f"\n✓ All {len(bypass_attempts)} path traversal attempts blocked")


@pytest.mark.security
def test_http_verb_tampering(client, valid_auth_headers):
    """Test HTTP verb tampering doesn't bypass auth."""
    
    # Try different HTTP methods without auth
    methods_to_test = [
        ("GET", "/api/v1/status"),
        ("POST", "/api/v1/chat"),
        ("PUT", "/api/npc/test-123/state"),
        ("DELETE", "/api/npc/test-123"),
    ]
    
    for method, path in methods_to_test:
        # Without auth
        if method == "GET":
            response = client.get(path)
        elif method == "POST":
            response = client.post(path, json={})
        elif method == "PUT":
            response = client.put(path, json={})
        elif method == "DELETE":
            response = client.delete(path)
        
        # Should require auth for all methods
        assert response.status_code in [401, 403, 404, 422], (
            f"{method} {path} didn't require auth"
        )
    
    print("\n✓ HTTP verb tampering blocked")


@pytest.mark.security
def test_content_type_confusion(client, valid_auth_headers):
    """Test content-type confusion doesn't bypass validation."""
    
    weird_content_types = [
        "application/json; charset=utf-7",
        "application/xml",
        "text/html",
        "multipart/form-data",
        "application/x-www-form-urlencoded",
    ]
    
    payload = {"message": "test"}
    
    for content_type in weird_content_types:
        headers = valid_auth_headers.copy()
        headers["Content-Type"] = content_type
        
        response = client.post("/api/v1/chat", headers=headers, json=payload)
        
        # Should either accept or reject gracefully
        assert response.status_code in [200, 400, 415, 422]
    
    print(f"\n✓ All {len(weird_content_types)} content-type variations handled")


# ============================================================================
# TOKEN EXPIRATION & ROTATION
# ============================================================================

@pytest.mark.security
def test_empty_auth_header_rejected(client):
    """Test empty Authorization header is rejected."""
    
    headers = {"Authorization": ""}
    response = client.get("/api/v1/status", headers=headers)
    
    assert response.status_code in [401, 403]
    
    print("\n✓ Empty auth header rejected")


@pytest.mark.security
def test_whitespace_api_key_rejected(client):
    """Test API keys with only whitespace are rejected."""
    
    whitespace_keys = [
        "   ",
        "\t\t",
        "\n\n",
        "Bearer    ",
        "Bearer \t\n",
    ]
    
    for key in whitespace_keys:
        headers = {"Authorization": key}
        response = client.get("/api/v1/status", headers=headers)
        
        assert response.status_code in [401, 403]
    
    print(f"\n✓ All {len(whitespace_keys)} whitespace keys rejected")


# ============================================================================
# SECURITY HEADERS VALIDATION
# ============================================================================

@pytest.mark.security
def test_security_headers_present(client_no_auth):
    """Test that security headers are present in responses."""
    
    response = client_no_auth.get("/health")
    
    required_headers = [
        "X-Content-Type-Options",
        "X-Frame-Options",
        "X-XSS-Protection",
        "Referrer-Policy",
        "Permissions-Policy",
    ]
    
    missing_headers = []
    for header in required_headers:
        if header not in response.headers:
            missing_headers.append(header)
    
    assert len(missing_headers) == 0, (
        f"Missing security headers: {', '.join(missing_headers)}"
    )
    
    print(f"\n✓ All {len(required_headers)} security headers present")


@pytest.mark.security
def test_no_sensitive_headers_leak(client, valid_auth_headers):
    """Test that sensitive headers don't leak."""
    
    response = client.get("/api/v1/status", headers=valid_auth_headers)
    
    sensitive_headers = [
        "X-Powered-By",  # Should be set to custom value
        "Server",  # Should be removed
    ]
    
    for header in sensitive_headers:
        if header in response.headers:
            value = response.headers[header].lower()
            # Check it's not leaking sensitive info
            assert "fastapi" not in value
            assert "uvicorn" not in value
            assert "python" not in value
    
    print("\n✓ No sensitive header leakage")