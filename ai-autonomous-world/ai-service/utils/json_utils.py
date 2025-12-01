"""
JSON Utilities

Safe JSON parsing and serialization with support for complex types.
Eliminates duplicate JSON logic from database.py and other modules.

Features:
- Safe JSON parsing with error handling
- Datetime, Decimal, UUID serialization
- Type coercion for deserialization
- Validation and schema checking
"""

import json
import logging
from typing import Any, Optional, Type, Dict
from datetime import datetime, date
from decimal import Decimal
from uuid import UUID
from enum import Enum

logger = logging.getLogger(__name__)


def safe_json_parse(
    json_str: str,
    default: Any = None,
    raise_on_error: bool = False
) -> Any:
    """
    Safely parse JSON string with error handling.
    
    Args:
        json_str: JSON string to parse
        default: Default value if parsing fails
        raise_on_error: Whether to raise exception on error
        
    Returns:
        Parsed JSON or default value
    """
    if not json_str:
        return default
    
    try:
        return json.loads(json_str)
    except json.JSONDecodeError as e:
        if raise_on_error:
            raise ValueError(f"Invalid JSON: {str(e)}")
        
        logger.warning(f"Failed to parse JSON: {str(e)}")
        return default
    except Exception as e:
        if raise_on_error:
            raise
        
        logger.error(f"Unexpected error parsing JSON: {str(e)}")
        return default


class CustomJSONEncoder(json.JSONEncoder):
    """Custom JSON encoder supporting datetime, Decimal, UUID, and Enum."""
    
    def default(self, obj):
        """Handle special types."""
        if isinstance(obj, datetime):
            return obj.isoformat()
        elif isinstance(obj, date):
            return obj.isoformat()
        elif isinstance(obj, Decimal):
            return float(obj)
        elif isinstance(obj, UUID):
            return str(obj)
        elif isinstance(obj, Enum):
            return obj.value
        elif hasattr(obj, '__dict__'):
            return obj.__dict__
        
        return super().default(obj)


def json_serialize(
    obj: Any,
    pretty: bool = False,
    ensure_ascii: bool = False
) -> str:
    """
    Serialize object to JSON with support for complex types.
    
    Args:
        obj: Object to serialize
        pretty: Whether to pretty-print
        ensure_ascii: Whether to escape non-ASCII characters
        
    Returns:
        str: JSON string
    """
    try:
        if pretty:
            return json.dumps(
                obj,
                cls=CustomJSONEncoder,
                indent=2,
                ensure_ascii=ensure_ascii
            )
        else:
            return json.dumps(
                obj,
                cls=CustomJSONEncoder,
                ensure_ascii=ensure_ascii
            )
    except Exception as e:
        logger.error(f"JSON serialization failed: {str(e)}")
        raise


def json_deserialize(
    json_str: str,
    expected_type: Optional[Type] = None,
    coerce_types: bool = True
) -> Any:
    """
    Deserialize JSON with optional type coercion.
    
    Args:
        json_str: JSON string
        expected_type: Expected type (dict, list, etc.)
        coerce_types: Whether to coerce types
        
    Returns:
        Deserialized object
    """
    obj = safe_json_parse(json_str, raise_on_error=True)
    
    if expected_type and not isinstance(obj, expected_type):
        if coerce_types:
            try:
                obj = expected_type(obj)
            except Exception as e:
                raise ValueError(
                    f"Cannot coerce {type(obj)} to {expected_type}: {str(e)}"
                )
        else:
            raise ValueError(
                f"Expected {expected_type}, got {type(obj)}"
            )
    
    return obj


def merge_json(base: dict, updates: dict, deep: bool = True) -> dict:
    """
    Merge two JSON objects.
    
    Args:
        base: Base dictionary
        updates: Updates to apply
        deep: Whether to deep merge nested dicts
        
    Returns:
        dict: Merged dictionary
    """
    result = base.copy()
    
    for key, value in updates.items():
        if deep and key in result and isinstance(result[key], dict) and isinstance(value, dict):
            result[key] = merge_json(result[key], value, deep=True)
        else:
            result[key] = value
    
    return result


def validate_json_schema(
    data: dict,
    required_fields: list[str],
    optional_fields: Optional[list[str]] = None
) -> tuple[bool, Optional[str]]:
    """
    Validate JSON against simple schema.
    
    Args:
        data: Data to validate
        required_fields: Required field names
        optional_fields: Optional field names
        
    Returns:
        tuple: (is_valid, error_message)
    """
    if not isinstance(data, dict):
        return False, "Data must be a dictionary"
    
    # Check required fields
    missing = [field for field in required_fields if field not in data]
    if missing:
        return False, f"Missing required fields: {', '.join(missing)}"
    
    # Check for unexpected fields
    allowed = set(required_fields + (optional_fields or []))
    unexpected = [field for field in data.keys() if field not in allowed]
    if unexpected:
        return False, f"Unexpected fields: {', '.join(unexpected)}"
    
    return True, None