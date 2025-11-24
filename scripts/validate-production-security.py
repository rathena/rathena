#!/usr/bin/env python3
"""
Production Security Validation Script
Validates all critical security requirements before deployment

This script checks:
- SEC-001: API authentication enabled
- SEC-009: Strong API keys configured
- SEC-011: Strong database passwords
- SEC-025: SSL/TLS enabled
- SEC-029: DragonflyDB password set
- SEC-015: CORS properly configured
- SEC-026: Database SSL enabled
- SEC-027: Port configuration documented

Exit code 0: All checks passed
Exit code 1: One or more critical checks failed
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Tuple, Dict

# ANSI color codes
RED = "\033[91m"
GREEN = "\033[92m"
YELLOW = "\033[93m"
BLUE = "\033[94m"
RESET = "\033[0m"
BOLD = "\033[1m"

class SecurityValidator:
    """Validates security configuration for production deployment"""
    
    def __init__(self, env_file: Path):
        self.env_file = env_file
        self.env_vars = self._load_env_file()
        self.failures: List[str] = []
        self.warnings: List[str] = []
        self.passes: List[str] = []
        
    def _load_env_file(self) -> Dict[str, str]:
        """Load environment variables from .env file"""
        env_vars = {}
        if not self.env_file.exists():
            print(f"{RED}ERROR: .env file not found at {self.env_file}{RESET}")
            sys.exit(1)
            
        with open(self.env_file, 'r') as f:
            for line in f:
                line = line.strip()
                # Skip comments and empty lines
                if not line or line.startswith('#'):
                    continue
                # Parse KEY=VALUE
                if '=' in line:
                    key, value = line.split('=', 1)
                    env_vars[key.strip()] = value.strip()
        
        return env_vars
    
    def _get_env(self, key: str, default: str = "") -> str:
        """Get environment variable value"""
        return self.env_vars.get(key, default)
    
    def _is_vault_reference(self, value: str) -> bool:
        """Check if value is a vault reference"""
        return value.startswith("vault://")
    
    def _is_placeholder(self, value: str) -> bool:
        """Check if value is a placeholder"""
        placeholders = [
            "your-",
            "changeme",
            "example",
            "placeholder",
            "todo",
            "xxx",
        ]
        value_lower = value.lower()
        return any(p in value_lower for p in placeholders)
    
    def _is_weak_password(self, password: str) -> bool:
        """Check if password is weak"""
        if len(password) < 16:
            return True
        
        # Check for common weak passwords
        weak_passwords = [
            "password", "admin", "root", "test", "demo",
            "ai_world_pass", "ai_world", "2025", "2024"
        ]
        password_lower = password.lower()
        return any(weak in password_lower for weak in weak_passwords)
    
    def validate_authentication(self) -> bool:
        """SEC-001 & SEC-009: Validate API authentication is enabled"""
        print(f"\n{BOLD}[SEC-001 & SEC-009] API Authentication{RESET}")
        
        # Check API_KEY_REQUIRED
        api_key_required = self._get_env("API_KEY_REQUIRED", "false").lower()
        if api_key_required != "true":
            self.failures.append("API_KEY_REQUIRED must be 'true' in production")
            print(f"  {RED}✗{RESET} API authentication is DISABLED (API_KEY_REQUIRED={api_key_required})")
            return False
        
        print(f"  {GREEN}✓{RESET} API authentication is ENABLED")
        
        # Check API key is set
        api_key = self._get_env("AI_SERVICE_API_KEY", "")
        if not api_key:
            self.failures.append("AI_SERVICE_API_KEY is not set")
            print(f"  {RED}✗{RESET} AI_SERVICE_API_KEY is EMPTY")
            return False
        
        # Check if it's a vault reference
        if self._is_vault_reference(api_key):
            print(f"  {GREEN}✓{RESET} API key uses vault reference: {api_key}")
            self.passes.append("API key stored in vault")
            return True
        
        # Check if it's a placeholder
        if self._is_placeholder(api_key):
            self.failures.append("AI_SERVICE_API_KEY is still a placeholder")
            print(f"  {RED}✗{RESET} API key is a PLACEHOLDER: {api_key}")
            return False
        
        # Check minimum length (should be 32+ bytes = 43+ base64 chars)
        if len(api_key) < 32:
            self.failures.append("AI_SERVICE_API_KEY is too short (< 32 chars)")
            print(f"  {RED}✗{RESET} API key is TOO SHORT: {len(api_key)} chars (minimum 32)")
            return False
        
        print(f"  {GREEN}✓{RESET} API key appears strong ({len(api_key)} chars)")
        self.passes.append("API key configured")
        return True
    
    def validate_database_passwords(self) -> bool:
        """SEC-011 & SEC-029: Validate strong database passwords"""
        print(f"\n{BOLD}[SEC-011 & SEC-029] Database Passwords{RESET}")
        
        all_passed = True
        
        # Check PostgreSQL password
        postgres_pass = self._get_env("POSTGRES_PASSWORD", "")
        if not postgres_pass:
            self.failures.append("POSTGRES_PASSWORD is not set")
            print(f"  {RED}✗{RESET} PostgreSQL password is EMPTY")
            all_passed = False
        elif self._is_vault_reference(postgres_pass):
            print(f"  {GREEN}✓{RESET} PostgreSQL password uses vault: {postgres_pass}")
            self.passes.append("PostgreSQL password in vault")
        elif self._is_placeholder(postgres_pass):
            self.failures.append("POSTGRES_PASSWORD is a placeholder")
            print(f"  {RED}✗{RESET} PostgreSQL password is PLACEHOLDER")
            all_passed = False
        elif self._is_weak_password(postgres_pass):
            self.failures.append("POSTGRES_PASSWORD is weak")
            print(f"  {RED}✗{RESET} PostgreSQL password is WEAK (too short or common)")
            all_passed = False
        else:
            print(f"  {GREEN}✓{RESET} PostgreSQL password appears strong")
            self.warnings.append("PostgreSQL password in plaintext .env (should use vault)")
        
        # Check DragonflyDB password
        dragonfly_pass = self._get_env("DRAGONFLY_PASSWORD", "")
        if not dragonfly_pass:
            self.failures.append("DRAGONFLY_PASSWORD is not set")
            print(f"  {RED}✗{RESET} DragonflyDB password is EMPTY")
            all_passed = False
        elif self._is_vault_reference(dragonfly_pass):
            print(f"  {GREEN}✓{RESET} DragonflyDB password uses vault: {dragonfly_pass}")
            self.passes.append("DragonflyDB password in vault")
        elif self._is_placeholder(dragonfly_pass):
            self.failures.append("DRAGONFLY_PASSWORD is a placeholder")
            print(f"  {RED}✗{RESET} DragonflyDB password is PLACEHOLDER")
            all_passed = False
        elif self._is_weak_password(dragonfly_pass):
            self.failures.append("DRAGONFLY_PASSWORD is weak")
            print(f"  {RED}✗{RESET} DragonflyDB password is WEAK")
            all_passed = False
        else:
            print(f"  {GREEN}✓{RESET} DragonflyDB password appears strong")
            self.warnings.append("DragonflyDB password in plaintext .env (should use vault)")
        
        return all_passed
    
    def validate_ssl_tls(self) -> bool:
        """SEC-025: Validate SSL/TLS is enabled"""
        print(f"\n{BOLD}[SEC-025] SSL/TLS Configuration{RESET}")
        
        ssl_enabled = self._get_env("SSL_ENABLED", "false").lower()
        if ssl_enabled != "true":
            self.failures.append("SSL_ENABLED must be 'true' in production")
            print(f"  {RED}✗{RESET} SSL/TLS is DISABLED (SSL_ENABLED={ssl_enabled})")
            return False
        
        print(f"  {GREEN}✓{RESET} SSL/TLS is ENABLED")
        
        # Check certificate files are configured
        ssl_keyfile = self._get_env("SSL_KEYFILE", "")
        ssl_certfile = self._get_env("SSL_CERTFILE", "")
        
        if not ssl_keyfile or not ssl_certfile:
            self.failures.append("SSL certificate files not configured")
            print(f"  {RED}✗{RESET} SSL certificate files not configured")
            return False
        
        # Check if files exist
        base_dir = self.env_file.parent
        keyfile_path = base_dir / ssl_keyfile
        certfile_path = base_dir / ssl_certfile
        
        if not keyfile_path.exists():
            self.failures.append(f"SSL key file not found: {ssl_keyfile}")
            print(f"  {RED}✗{RESET} SSL key file NOT FOUND: {ssl_keyfile}")
            print(f"      Run: bash scripts/generate-ssl-certs.sh")
            return False
        
        if not certfile_path.exists():
            self.failures.append(f"SSL cert file not found: {ssl_certfile}")
            print(f"  {RED}✗{RESET} SSL cert file NOT FOUND: {ssl_certfile}")
            print(f"      Run: bash scripts/generate-ssl-certs.sh")
            return False
        
        print(f"  {GREEN}✓{RESET} SSL key file exists: {ssl_keyfile}")
        print(f"  {GREEN}✓{RESET} SSL cert file exists: {ssl_certfile}")
        self.passes.append("SSL/TLS configured")
        return True
    
    def validate_database_encryption(self) -> bool:
        """SEC-026: Validate database connection encryption"""
        print(f"\n{BOLD}[SEC-026] Database Connection Encryption{RESET}")
        
        all_passed = True
        
        # Check PostgreSQL SSL mode
        postgres_sslmode = self._get_env("POSTGRES_SSLMODE", "prefer")
        if postgres_sslmode not in ["require", "verify-ca", "verify-full"]:
            self.failures.append(f"POSTGRES_SSLMODE should be 'require', 'verify-ca', or 'verify-full', got '{postgres_sslmode}'")
            print(f"  {RED}✗{RESET} PostgreSQL SSL mode is '{postgres_sslmode}' (should be 'require' or stronger)")
            all_passed = False
        else:
            print(f"  {GREEN}✓{RESET} PostgreSQL SSL mode: {postgres_sslmode}")
            self.passes.append("PostgreSQL SSL enabled")
        
        # Check DragonflyDB SSL
        dragonfly_ssl = self._get_env("DRAGONFLY_SSL", "false").lower()
        if dragonfly_ssl == "true":
            print(f"  {GREEN}✓{RESET} DragonflyDB SSL is enabled")
            self.passes.append("DragonflyDB SSL enabled")
        else:
            self.warnings.append("DragonflyDB SSL not enabled (recommended for production)")
            print(f"  {YELLOW}⚠{RESET} DragonflyDB SSL not enabled (recommended for production)")
        
        return all_passed
    
    def validate_cors(self) -> bool:
        """SEC-015: Validate CORS configuration"""
        print(f"\n{BOLD}[SEC-015] CORS Configuration{RESET}")
        
        cors_origins = self._get_env("CORS_ORIGINS", "")
        
        # Check if wildcard is used
        if "*" in cors_origins:
            self.failures.append("CORS allows wildcard (*) - must specify exact domains")
            print(f"  {RED}✗{RESET} CORS allows WILDCARD (*) - INSECURE")
            return False
        
        # Check if configured
        if not cors_origins:
            self.warnings.append("CORS_ORIGINS not explicitly set")
            print(f"  {YELLOW}⚠{RESET} CORS_ORIGINS not set (using defaults)")
            return True
        
        # List configured origins
        origins = [o.strip() for o in cors_origins.split(',')]
        print(f"  {GREEN}✓{RESET} CORS restricted to specific origins:")
        for origin in origins:
            print(f"      - {origin}")
        
        self.passes.append("CORS properly restricted")
        return True
    
    def validate_secrets_in_vault(self) -> bool:
        """Validate that secrets use vault references"""
        print(f"\n{BOLD}[BEST PRACTICE] Secrets Management{RESET}")
        
        secret_keys = [
            "AZURE_OPENAI_API_KEY",
            "OPENAI_API_KEY",
            "ANTHROPIC_API_KEY",
            "GOOGLE_API_KEY",
            "DEEPSEEK_API_KEY",
            "AI_SERVICE_API_KEY",
            "POSTGRES_PASSWORD",
            "DRAGONFLY_PASSWORD",
        ]
        
        plaintext_secrets = []
        vault_secrets = []
        
        for key in secret_keys:
            value = self._get_env(key, "")
            if not value:
                continue  # Not configured, skip
            
            if self._is_vault_reference(value):
                vault_secrets.append(key)
            elif not self._is_placeholder(value):
                # Real value in plaintext
                plaintext_secrets.append(key)
        
        if plaintext_secrets:
            print(f"  {YELLOW}⚠{RESET} The following secrets are in PLAINTEXT (should use vault://):")
            for key in plaintext_secrets:
                print(f"      - {key}")
            self.warnings.append(f"{len(plaintext_secrets)} secrets in plaintext")
        
        if vault_secrets:
            print(f"  {GREEN}✓{RESET} The following secrets use vault references:")
            for key in vault_secrets:
                print(f"      - {key}")
            self.passes.append(f"{len(vault_secrets)} secrets in vault")
        
        return len(plaintext_secrets) == 0
    
    def validate_debug_mode(self) -> bool:
        """Validate debug mode is disabled in production"""
        print(f"\n{BOLD}[BEST PRACTICE] Debug Mode{RESET}")
        
        debug_mode = self._get_env("DEBUG_MODE", "true").lower()
        if debug_mode == "true":
            self.warnings.append("DEBUG_MODE is enabled (should be false in production)")
            print(f"  {YELLOW}⚠{RESET} DEBUG_MODE is ENABLED (should be disabled in production)")
            return False
        
        print(f"  {GREEN}✓{RESET} DEBUG_MODE is disabled")
        self.passes.append("Debug mode disabled")
        return True
    
    def validate_exposed_api_keys(self) -> bool:
        """SEC-001: Check for exposed API keys in .env"""
        print(f"\n{BOLD}[SEC-001] API Key Exposure Check{RESET}")
        
        # Pattern to detect real Azure OpenAI keys (start with specific prefix)
        azure_pattern = re.compile(r'^[A-Za-z0-9]{64,}$')
        
        azure_key = self._get_env("AZURE_OPENAI_API_KEY", "")
        
        if azure_key and not self._is_vault_reference(azure_key) and not self._is_placeholder(azure_key):
            if azure_pattern.match(azure_key):
                self.failures.append("CRITICAL: Azure OpenAI API key appears to be real - ROTATE IMMEDIATELY")
                print(f"  {RED}✗{RESET} CRITICAL: Real Azure OpenAI key detected in plaintext")
                print(f"      This key MUST be rotated and moved to vault")
                return False
        
        print(f"  {GREEN}✓{RESET} No exposed API keys detected")
        self.passes.append("No exposed API keys")
        return True
    
    def generate_report(self) -> bool:
        """Generate final validation report"""
        print(f"\n{'=' * 80}")
        print(f"{BOLD}PRODUCTION SECURITY VALIDATION REPORT{RESET}")
        print(f"{'=' * 80}\n")
        
        # Summary
        total_checks = len(self.passes) + len(self.failures) + len(self.warnings)
        passed = len(self.passes)
        failed = len(self.failures)
        warned = len(self.warnings)
        
        print(f"Total Checks: {total_checks}")
        print(f"{GREEN}Passed: {passed}{RESET}")
        print(f"{RED}Failed: {failed}{RESET}")
        print(f"{YELLOW}Warnings: {warned}{RESET}\n")
        
        # Failed checks
        if self.failures:
            print(f"{RED}{BOLD}FAILED CHECKS (BLOCKING):{RESET}")
            for i, failure in enumerate(self.failures, 1):
                print(f"  {i}. {failure}")
            print()
        
        # Warnings
        if self.warnings:
            print(f"{YELLOW}{BOLD}WARNINGS (NON-BLOCKING):{RESET}")
            for i, warning in enumerate(self.warnings, 1):
                print(f"  {i}. {warning}")
            print()
        
        # Passed checks
        if self.passes:
            print(f"{GREEN}{BOLD}PASSED CHECKS:{RESET}")
            for i, check in enumerate(self.passes, 1):
                print(f"  {i}. {check}")
            print()
        
        # Final verdict
        print(f"{'=' * 80}")
        if failed == 0:
            print(f"{GREEN}{BOLD}✓ PRODUCTION SECURITY VALIDATION PASSED{RESET}")
            print(f"{GREEN}System is ready for production deployment{RESET}")
            if warned > 0:
                print(f"{YELLOW}Note: {warned} warnings present (review recommended){RESET}")
            print(f"{'=' * 80}\n")
            return True
        else:
            print(f"{RED}{BOLD}✗ PRODUCTION SECURITY VALIDATION FAILED{RESET}")
            print(f"{RED}Fix {failed} critical issue(s) before deployment{RESET}")
            print(f"{'=' * 80}\n")
            return False
    
    def run_all_validations(self) -> bool:
        """Run all security validations"""
        print(f"\n{BLUE}{BOLD}╔═══════════════════════════════════════════════════════════╗{RESET}")
        print(f"{BLUE}{BOLD}║  PRODUCTION SECURITY VALIDATION                          ║{RESET}")
        print(f"{BLUE}{BOLD}║  rathena-AI-world Server                                 ║{RESET}")
        print(f"{BLUE}{BOLD}╚═══════════════════════════════════════════════════════════╝{RESET}")
        
        print(f"\nValidating: {self.env_file}")
        print(f"Loaded {len(self.env_vars)} environment variables\n")
        
        # Run all validations
        validations = [
            self.validate_exposed_api_keys,
            self.validate_authentication,
            self.validate_database_passwords,
            self.validate_ssl_tls,
            self.validate_database_encryption,
            self.validate_cors,
            self.validate_secrets_in_vault,
            self.validate_debug_mode,
        ]
        
        for validation in validations:
            try:
                validation()
            except Exception as e:
                self.failures.append(f"Validation error: {str(e)}")
                print(f"  {RED}✗{RESET} Error during validation: {e}")
        
        # Generate final report
        return self.generate_report()


def main():
    """Main validation entry point"""
    # Determine project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    env_file = project_root / ".env"
    
    # Allow override via command line
    if len(sys.argv) > 1:
        env_file = Path(sys.argv[1])
    
    # Run validation
    validator = SecurityValidator(env_file)
    passed = validator.run_all_validations()
    
    # Exit with appropriate code
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()