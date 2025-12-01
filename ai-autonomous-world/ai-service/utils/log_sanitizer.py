"""
Log Sanitization Utility - Fix #8

Prevents sensitive data leaks in logs by automatically redacting:
- Passwords
- API keys
- Tokens
- Session secrets
- Credit card numbers
- Social security numbers
- Email addresses (optional)
- IP addresses (optional)
"""

import re
import logging
from typing import Any, Dict, Union

logger = logging.getLogger(__name__)


class LogSanitizer:
    """
    Sanitize sensitive data from log messages and structured data.
    """
    
    # Patterns for sensitive data
    PATTERNS = {
        # Password patterns
        'password': re.compile(
            r'(password|passwd|pwd)["\s]*[:=]["\s]*[^"\s,}]+',
            re.IGNORECASE
        ),
        
        # API key patterns
        'api_key': re.compile(
            r'(api[_-]?key|apikey|key)["\s]*[:=]["\s]*[A-Za-z0-9+/=]{20,}',
            re.IGNORECASE
        ),
        
        # Azure OpenAI key pattern (specific format)
        'azure_key': re.compile(
            r'AZURE_OPENAI_API_KEY["\s]*[:=]["\s]*[A-Za-z0-9]{32,}',
            re.IGNORECASE
        ),
        
        # Bearer tokens
        'bearer_token': re.compile(
            r'Bearer\s+[A-Za-z0-9\-._~+/]+=*',
            re.IGNORECASE
        ),
        
        # JWT tokens
        'jwt': re.compile(
            r'eyJ[A-Za-z0-9_-]{10,}\.[A-Za-z0-9_-]{10,}\.[A-Za-z0-9_-]{10,}'
        ),
        
        # Generic secrets
        'secret': re.compile(
            r'(secret|token)["\s]*[:=]["\s]*[A-Za-z0-9+/=]{20,}',
            re.IGNORECASE
        ),
        
        # Credit card numbers (basic pattern)
        'credit_card': re.compile(
            r'\b(?:\d{4}[-\s]?){3}\d{4}\b'
        ),
        
        # Social Security Numbers
        'ssn': re.compile(
            r'\b\d{3}-\d{2}-\d{4}\b'
        ),
        
        # Private keys
        'private_key': re.compile(
            r'-----BEGIN (RSA |DSA |EC |OPENSSH )?PRIVATE KEY-----[\s\S]*?-----END (RSA |DSA |EC |OPENSSH )?PRIVATE KEY-----'
        ),
    }
    
    # Optional patterns (enabled by configuration)
    OPTIONAL_PATTERNS = {
        # Email addresses
        'email': re.compile(
            r'\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b'
        ),
        
        # IP addresses
        'ip_address': re.compile(
            r'\b(?:\d{1,3}\.){3}\d{1,3}\b'
        ),
    }
    
    REDACTED_TEXT = "***REDACTED***"
    
    def __init__(self, sanitize_emails: bool = False, sanitize_ips: bool = False):
        """
        Initialize sanitizer.
        
        Args:
            sanitize_emails: Whether to redact email addresses
            sanitize_ips: Whether to redact IP addresses
        """
        self.sanitize_emails = sanitize_emails
        self.sanitize_ips = sanitize_ips
        
        # Combine patterns
        self.active_patterns = self.PATTERNS.copy()
        if sanitize_emails:
            self.active_patterns['email'] = self.OPTIONAL_PATTERNS['email']
        if sanitize_ips:
            self.active_patterns['ip_address'] = self.OPTIONAL_PATTERNS['ip_address']
    
    def sanitize_string(self, text: str) -> str:
        """
        Sanitize a string by replacing sensitive patterns.
        
        Args:
            text: Text to sanitize
            
        Returns:
            Sanitized text
        """
        if not isinstance(text, str):
            return text
        
        sanitized = text
        for pattern_name, pattern in self.active_patterns.items():
            # Replace matches with redacted text
            sanitized = pattern.sub(
                lambda m: self._create_redacted_replacement(m.group(0), pattern_name),
                sanitized
            )
        
        return sanitized
    
    def sanitize_dict(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Recursively sanitize a dictionary.
        
        Args:
            data: Dictionary to sanitize
            
        Returns:
            Sanitized dictionary
        """
        if not isinstance(data, dict):
            return data
        
        sanitized = {}
        for key, value in data.items():
            # Check if key itself indicates sensitive data
            if self._is_sensitive_key(key):
                sanitized[key] = self.REDACTED_TEXT
            elif isinstance(value, dict):
                sanitized[key] = self.sanitize_dict(value)
            elif isinstance(value, list):
                sanitized[key] = [
                    self.sanitize_dict(item) if isinstance(item, dict)
                    else self.sanitize_string(str(item)) if isinstance(item, str)
                    else item
                    for item in value
                ]
            elif isinstance(value, str):
                sanitized[key] = self.sanitize_string(value)
            else:
                sanitized[key] = value
        
        return sanitized
    
    def sanitize_log_record(self, record: logging.LogRecord) -> logging.LogRecord:
        """
        Sanitize a log record before output.
        
        Args:
            record: Log record to sanitize
            
        Returns:
            Sanitized log record
        """
        # Sanitize the message
        if hasattr(record, 'msg') and isinstance(record.msg, str):
            record.msg = self.sanitize_string(record.msg)
        
        # Sanitize args
        if hasattr(record, 'args') and record.args:
            if isinstance(record.args, dict):
                record.args = self.sanitize_dict(record.args)
            elif isinstance(record.args, (list, tuple)):
                record.args = tuple(
                    self.sanitize_string(str(arg)) if isinstance(arg, str)
                    else arg
                    for arg in record.args
                )
        
        return record
    
    def _is_sensitive_key(self, key: str) -> bool:
        """Check if a dictionary key indicates sensitive data"""
        sensitive_keys = {
            'password', 'passwd', 'pwd', 'secret', 'token',
            'api_key', 'apikey', 'private_key', 'access_token',
            'refresh_token', 'session_secret', 'jwt_secret',
            'credit_card', 'ssn', 'social_security'
        }
        
        key_lower = key.lower()
        return any(sensitive in key_lower for sensitive in sensitive_keys)
    
    def _create_redacted_replacement(self, matched_text: str, pattern_name: str) -> str:
        """
        Create a redacted replacement that preserves some context.
        
        Args:
            matched_text: The matched sensitive text
            pattern_name: Name of the pattern that matched
            
        Returns:
            Redacted replacement string
        """
        # Extract key/field name if present
        if '=' in matched_text or ':' in matched_text:
            parts = re.split(r'[=:]', matched_text, 1)
            if len(parts) == 2:
                key_part = parts[0].strip()
                return f"{key_part}={self.REDACTED_TEXT}"
        
        return self.REDACTED_TEXT


# Global sanitizer instance
_sanitizer = None


def get_sanitizer(sanitize_emails: bool = False, sanitize_ips: bool = False) -> LogSanitizer:
    """
    Get or create the global sanitizer instance.
    
    Args:
        sanitize_emails: Whether to redact email addresses
        sanitize_ips: Whether to redact IP addresses
        
    Returns:
        LogSanitizer instance
    """
    global _sanitizer
    if _sanitizer is None:
        _sanitizer = LogSanitizer(
            sanitize_emails=sanitize_emails,
            sanitize_ips=sanitize_ips
        )
    return _sanitizer


def sanitize_for_logging(data: Union[str, Dict, Any]) -> Union[str, Dict, Any]:
    """
    Convenience function to sanitize data for logging.
    
    Args:
        data: Data to sanitize (string, dict, or other)
        
    Returns:
        Sanitized data
    """
    sanitizer = get_sanitizer()
    
    if isinstance(data, str):
        return sanitizer.sanitize_string(data)
    elif isinstance(data, dict):
        return sanitizer.sanitize_dict(data)
    else:
        # Try to convert to string and sanitize
        try:
            return sanitizer.sanitize_string(str(data))
        except Exception:
            return data


class SanitizingLogFilter(logging.Filter):
    """
    Logging filter that sanitizes log records.
    """
    
    def __init__(self, sanitize_emails: bool = False, sanitize_ips: bool = False):
        super().__init__()
        self.sanitizer = LogSanitizer(
            sanitize_emails=sanitize_emails,
            sanitize_ips=sanitize_ips
        )
    
    def filter(self, record: logging.LogRecord) -> bool:
        """
        Filter and sanitize the log record.
        
        Args:
            record: Log record to filter
            
        Returns:
            True (always pass through, but sanitized)
        """
        self.sanitizer.sanitize_log_record(record)
        return True


def configure_sanitized_logging(
    level: str = "INFO",
    format_string: str = None,
    sanitize_emails: bool = False,
    sanitize_ips: bool = False
) -> None:
    """
    Configure logging with automatic sanitization.
    
    Args:
        level: Logging level
        format_string: Log format string
        sanitize_emails: Whether to redact emails
        sanitize_ips: Whether to redact IP addresses
    """
    if format_string is None:
        format_string = (
            '%(asctime)s - %(name)s - %(levelname)s - '
            '[%(filename)s:%(lineno)d] - %(message)s'
        )
    
    # Create sanitizing filter
    sanitizer_filter = SanitizingLogFilter(
        sanitize_emails=sanitize_emails,
        sanitize_ips=sanitize_ips
    )
    
    # Configure root logger
    root_logger = logging.getLogger()
    root_logger.setLevel(level)
    
    # Add filter to all handlers
    for handler in root_logger.handlers:
        handler.addFilter(sanitizer_filter)
    
    # If no handlers, create one
    if not root_logger.handlers:
        handler = logging.StreamHandler()
        handler.setLevel(level)
        handler.setFormatter(logging.Formatter(format_string))
        handler.addFilter(sanitizer_filter)
        root_logger.addHandler(handler)
    
    logger.info("✓ Log sanitization configured")


# Example usage in other modules:
# from utils.log_sanitizer import sanitize_for_logging
# 
# config_dict = {...}  # Contains passwords, API keys, etc.
# logger.info(f"Config loaded: {sanitize_for_logging(config_dict)}")