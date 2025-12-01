"""
Tests for JSON Utilities

Comprehensive test suite covering:
- JSON serialization/deserialization
- Custom encoders for complex types
- Error handling for malformed JSON
- Performance optimization
- Thread safety
- Edge cases (NaN, Inf, datetime, etc.)
"""

import pytest
import json
from datetime import datetime, date, timedelta
from decimal import Decimal
from enum import Enum
from dataclasses import dataclass
from typing import Optional
from concurrent.futures import ThreadPoolExecutor, as_completed
import time


# =============================================================================
# MOCK CLASSES FOR TESTING
# =============================================================================

class Status(Enum):
    """Test enum"""
    ACTIVE = "active"
    INACTIVE = "inactive"
    PENDING = "pending"


@dataclass
class TestDataClass:
    """Test dataclass"""
    name: str
    value: int
    timestamp: datetime
    optional: Optional[str] = None


class CustomObject:
    """Test custom object"""
    def __init__(self, name: str, value: int):
        self.name = name
        self.value = value
    
    def to_dict(self):
        return {'name': self.name, 'value': self.value}


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def simple_dict():
    """Simple dictionary data"""
    return {
        'string': 'value',
        'number': 42,
        'float': 3.14,
        'boolean': True,
        'null': None,
        'list': [1, 2, 3],
        'nested': {'key': 'value'}
    }


@pytest.fixture
def complex_dict():
    """Complex dictionary with special types"""
    return {
        'datetime': datetime(2024, 1, 1, 12, 0, 0),
        'date': date(2024, 1, 1),
        'decimal': Decimal('10.50'),
        'enum': Status.ACTIVE,
        'bytes': b'binary data',
        'custom': CustomObject('test', 123)
    }


# =============================================================================
# BASIC JSON OPERATIONS TESTS
# =============================================================================

class TestBasicJsonOperations:
    """Test basic JSON serialization/deserialization"""
    
    def test_serialize_simple_dict(self, simple_dict):
        """Test serializing simple dictionary"""
        result = json.dumps(simple_dict)
        assert isinstance(result, str)
        assert 'string' in result
        assert '42' in result
    
    def test_deserialize_simple_json(self):
        """Test deserializing simple JSON"""
        json_str = '{"name": "test", "value": 42}'
        result = json.loads(json_str)
        
        assert result['name'] == 'test'
        assert result['value'] == 42
    
    def test_roundtrip_simple(self, simple_dict):
        """Test serialize-deserialize roundtrip"""
        json_str = json.dumps(simple_dict)
        result = json.loads(json_str)
        
        assert result == simple_dict
    
    def test_serialize_with_indent(self, simple_dict):
        """Test pretty printing with indent"""
        result = json.dumps(simple_dict, indent=2)
        
        assert '\n' in result
        assert '  ' in result  # Indentation
    
    def test_serialize_sort_keys(self):
        """Test serializing with sorted keys"""
        data = {'z': 1, 'a': 2, 'm': 3}
        result = json.dumps(data, sort_keys=True)
        
        # Keys should be alphabetically sorted
        assert result.index('"a"') < result.index('"m"') < result.index('"z"')


# =============================================================================
# CUSTOM ENCODER TESTS
# =============================================================================

class TestCustomEncoder:
    """Test custom JSON encoder for complex types"""
    
    def test_encode_datetime(self):
        """Test encoding datetime objects"""
        class DateTimeEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, datetime):
                    return obj.isoformat()
                return super().default(obj)
        
        data = {'timestamp': datetime(2024, 1, 1, 12, 0, 0)}
        result = json.dumps(data, cls=DateTimeEncoder)
        
        assert '2024-01-01T12:00:00' in result
    
    def test_encode_date(self):
        """Test encoding date objects"""
        class DateEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, date):
                    return obj.isoformat()
                return super().default(obj)
        
        data = {'date': date(2024, 1, 1)}
        result = json.dumps(data, cls=DateEncoder)
        
        assert '2024-01-01' in result
    
    def test_encode_decimal(self):
        """Test encoding Decimal objects"""
        class DecimalEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, Decimal):
                    return float(obj)
                return super().default(obj)
        
        data = {'price': Decimal('10.50')}
        result = json.dumps(data, cls=DecimalEncoder)
        
        assert '10.5' in result
    
    def test_encode_enum(self):
        """Test encoding Enum objects"""
        class EnumEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, Enum):
                    return obj.value
                return super().default(obj)
        
        data = {'status': Status.ACTIVE}
        result = json.dumps(data, cls=EnumEncoder)
        
        assert '"active"' in result
    
    def test_encode_bytes(self):
        """Test encoding bytes objects"""
        import base64
        
        class BytesEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, bytes):
                    return base64.b64encode(obj).decode('utf-8')
                return super().default(obj)
        
        data = {'data': b'binary'}
        result = json.dumps(data, cls=BytesEncoder)
        
        assert isinstance(result, str)
    
    def test_encode_custom_object(self):
        """Test encoding custom objects with to_dict"""
        class CustomEncoder(json.JSONEncoder):
            def default(self, obj):
                if hasattr(obj, 'to_dict'):
                    return obj.to_dict()
                return super().default(obj)
        
        obj = CustomObject('test', 123)
        result = json.dumps({'obj': obj}, cls=CustomEncoder)
        
        assert '"name": "test"' in result
        assert '"value": 123' in result
    
    def test_universal_encoder(self):
        """Test universal encoder handling multiple types"""
        class UniversalEncoder(json.JSONEncoder):
            def default(self, obj):
                if isinstance(obj, datetime):
                    return obj.isoformat()
                elif isinstance(obj, date):
                    return obj.isoformat()
                elif isinstance(obj, Decimal):
                    return float(obj)
                elif isinstance(obj, Enum):
                    return obj.value
                elif isinstance(obj, bytes):
                    import base64
                    return base64.b64encode(obj).decode('utf-8')
                elif hasattr(obj, 'to_dict'):
                    return obj.to_dict()
                return super().default(obj)
        
        data = {
            'datetime': datetime(2024, 1, 1),
            'decimal': Decimal('10.5'),
            'enum': Status.ACTIVE
        }
        
        result = json.dumps(data, cls=UniversalEncoder)
        assert '2024-01-01' in result
        assert '10.5' in result
        assert 'active' in result


# =============================================================================
# ERROR HANDLING TESTS
# =============================================================================

class TestErrorHandling:
    """Test error handling in JSON operations"""
    
    def test_deserialize_invalid_json(self):
        """Test handling invalid JSON"""
        invalid_json = '{"key": invalid}'
        
        with pytest.raises(json.JSONDecodeError):
            json.loads(invalid_json)
    
    def test_deserialize_incomplete_json(self):
        """Test handling incomplete JSON"""
        incomplete = '{"key": "value"'  # Missing closing brace
        
        with pytest.raises(json.JSONDecodeError):
            json.loads(incomplete)
    
    def test_serialize_unserializable_type(self):
        """Test handling unserializable types"""
        data = {'func': lambda x: x}  # Functions not serializable
        
        with pytest.raises(TypeError):
            json.dumps(data)
    
    def test_safe_json_loads(self):
        """Test safe JSON loads with error handling"""
        def safe_loads(json_str, default=None):
            try:
                return json.loads(json_str)
            except (json.JSONDecodeError, TypeError):
                return default
        
        # Valid JSON
        assert safe_loads('{"key": "value"}') == {'key': 'value'}
        
        # Invalid JSON returns default
        assert safe_loads('invalid', default={}) == {}
    
    def test_safe_json_dumps(self):
        """Test safe JSON dumps with error handling"""
        def safe_dumps(obj, default="{}"):
            try:
                return json.dumps(obj)
            except (TypeError, ValueError):
                return default
        
        # Valid object
        assert safe_dumps({'key': 'value'}) == '{"key": "value"}'
        
        # Invalid object returns default
        assert safe_dumps(lambda x: x, default="{}") == "{}"


# =============================================================================
# SPECIAL VALUES TESTS
# =============================================================================

class TestSpecialValues:
    """Test handling of special JSON values"""
    
    def test_handle_none_values(self):
        """Test handling None/null values"""
        data = {'key': None}
        result = json.dumps(data)
        
        assert 'null' in result
        
        parsed = json.loads(result)
        assert parsed['key'] is None
    
    def test_handle_empty_collections(self):
        """Test handling empty collections"""
        data = {
            'empty_list': [],
            'empty_dict': {},
            'empty_string': ''
        }
        result = json.dumps(data)
        parsed = json.loads(result)
        
        assert parsed['empty_list'] == []
        assert parsed['empty_dict'] == {}
        assert parsed['empty_string'] == ''
    
    def test_handle_unicode(self):
        """Test handling Unicode characters"""
        data = {'text': 'Hello 世界 🌍'}
        result = json.dumps(data, ensure_ascii=False)
        
        assert '世界' in result
        assert '🌍' in result
        
        parsed = json.loads(result)
        assert parsed['text'] == 'Hello 世界 🌍'
    
    def test_handle_escape_characters(self):
        """Test handling escape characters"""
        data = {'text': 'Line1\nLine2\tTabbed'}
        result = json.dumps(data)
        
        assert '\\n' in result
        assert '\\t' in result
        
        parsed = json.loads(result)
        assert parsed['text'] == 'Line1\nLine2\tTabbed'
    
    def test_handle_special_numbers(self):
        """Test handling special numeric values"""
        # JSON doesn't support NaN, Infinity
        # Test that these need special handling
        import math
        
        with pytest.raises((ValueError, OverflowError)):
            json.dumps({'nan': math.nan})
        
        with pytest.raises((ValueError, OverflowError)):
            json.dumps({'inf': math.inf})


# =============================================================================
# PERFORMANCE TESTS
# =============================================================================

class TestPerformance:
    """Test JSON performance"""
    
    @pytest.mark.performance
    def test_serialize_performance(self, simple_dict):
        """Test serialization performance"""
        start = time.perf_counter()
        for _ in range(10000):
            json.dumps(simple_dict)
        duration = time.perf_counter() - start
        
        assert duration < 1.0  # Should complete in < 1 second
    
    @pytest.mark.performance
    def test_deserialize_performance(self):
        """Test deserialization performance"""
        json_str = json.dumps({'key': 'value', 'number': 42})
        
        start = time.perf_counter()
        for _ in range(10000):
            json.loads(json_str)
        duration = time.perf_counter() - start
        
        assert duration < 1.0
    
    @pytest.mark.performance
    def test_large_object_serialization(self):
        """Test serializing large objects"""
        large_dict = {'items': [{'id': i, 'name': f'Item {i}'} for i in range(1000)]}
        
        start = time.perf_counter()
        result = json.dumps(large_dict)
        duration = time.perf_counter() - start
        
        assert duration < 0.1  # Should be fast
        assert len(result) > 10000  # Should be large


# =============================================================================
# THREAD SAFETY TESTS
# =============================================================================

class TestThreadSafety:
    """Test thread safety of JSON operations"""
    
    @pytest.mark.concurrent
    def test_concurrent_serialization(self, simple_dict):
        """Test concurrent JSON serialization"""
        def serialize(i):
            data = simple_dict.copy()
            data['id'] = i
            return json.dumps(data)
        
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(serialize, i) for i in range(100)]
            results = [f.result() for f in as_completed(futures)]
        
        assert len(results) == 100
        assert all(isinstance(r, str) for r in results)
    
    @pytest.mark.concurrent
    def test_concurrent_deserialization(self):
        """Test concurrent JSON deserialization"""
        json_strings = [json.dumps({'id': i, 'value': i * 2}) for i in range(100)]
        
        def deserialize(json_str):
            return json.loads(json_str)
        
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(deserialize, s) for s in json_strings]
            results = [f.result() for f in as_completed(futures)]
        
        assert len(results) == 100
        assert all(isinstance(r, dict) for r in results)


# =============================================================================
# UTILITY FUNCTION TESTS
# =============================================================================

class TestUtilityFunctions:
    """Test JSON utility functions"""
    
    def test_json_equals(self):
        """Test comparing JSON strings for equality"""
        json1 = '{"a": 1, "b": 2}'
        json2 = '{"b": 2, "a": 1}'  # Different order
        
        # Should be equal when parsed
        assert json.loads(json1) == json.loads(json2)
    
    def test_json_merge(self):
        """Test merging JSON objects"""
        obj1 = {'a': 1, 'b': 2}
        obj2 = {'b': 3, 'c': 4}
        
        merged = {**obj1, **obj2}
        assert merged == {'a': 1, 'b': 3, 'c': 4}
    
    def test_json_path_access(self):
        """Test accessing nested JSON values"""
        data = {
            'user': {
                'profile': {
                    'name': 'John'
                }
            }
        }
        
        # Safe nested access
        name = data.get('user', {}).get('profile', {}).get('name')
        assert name == 'John'
        
        # Non-existent path
        missing = data.get('user', {}).get('settings', {}).get('theme')
        assert missing is None
    
    def test_json_filter_keys(self):
        """Test filtering JSON keys"""
        data = {'a': 1, 'b': 2, 'c': 3, 'd': 4}
        allowed_keys = ['a', 'c']
        
        filtered = {k: v for k, v in data.items() if k in allowed_keys}
        assert filtered == {'a': 1, 'c': 3}
    
    def test_json_flatten(self):
        """Test flattening nested JSON"""
        nested = {
            'user': {
                'name': 'John',
                'age': 30
            },
            'active': True
        }
        
        def flatten_dict(d, parent_key='', sep='.'):
            items = []
            for k, v in d.items():
                new_key = f"{parent_key}{sep}{k}" if parent_key else k
                if isinstance(v, dict):
                    items.extend(flatten_dict(v, new_key, sep=sep).items())
                else:
                    items.append((new_key, v))
            return dict(items)
        
        flattened = flatten_dict(nested)
        assert flattened['user.name'] == 'John'
        assert flattened['user.age'] == 30
        assert flattened['active'] is True


# =============================================================================
# VALIDATION TESTS
# =============================================================================

class TestValidation:
    """Test JSON validation"""
    
    def test_validate_json_string(self):
        """Test validating JSON string"""
        def is_valid_json(json_str):
            try:
                json.loads(json_str)
                return True
            except (json.JSONDecodeError, TypeError):
                return False
        
        assert is_valid_json('{"key": "value"}')
        assert not is_valid_json('invalid')
        assert not is_valid_json(None)
    
    def test_validate_json_structure(self):
        """Test validating JSON structure"""
        def validate_structure(data, required_keys):
            if not isinstance(data, dict):
                return False
            return all(key in data for key in required_keys)
        
        data = {'name': 'test', 'value': 42}
        assert validate_structure(data, ['name', 'value'])
        assert not validate_structure(data, ['name', 'missing'])
    
    def test_validate_json_types(self):
        """Test validating JSON value types"""
        def validate_types(data, type_spec):
            for key, expected_type in type_spec.items():
                if key not in data:
                    return False
                if not isinstance(data[key], expected_type):
                    return False
            return True
        
        data = {'name': 'test', 'value': 42, 'active': True}
        spec = {'name': str, 'value': int, 'active': bool}
        
        assert validate_types(data, spec)


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    def test_deeply_nested_structure(self):
        """Test handling deeply nested structures"""
        # Create deeply nested dict
        nested = {'level': 0}
        current = nested
        for i in range(1, 100):
            current['nested'] = {'level': i}
            current = current['nested']
        
        # Should be able to serialize/deserialize
        json_str = json.dumps(nested)
        parsed = json.loads(json_str)
        
        assert parsed['level'] == 0
    
    def test_very_long_strings(self):
        """Test handling very long strings"""
        long_string = 'a' * 1000000  # 1 million characters
        data = {'text': long_string}
        
        json_str = json.dumps(data)
        parsed = json.loads(json_str)
        
        assert len(parsed['text']) == 1000000
    
    def test_many_keys(self):
        """Test handling objects with many keys"""
        many_keys = {f'key_{i}': i for i in range(10000)}
        
        json_str = json.dumps(many_keys)
        parsed = json.loads(json_str)
        
        assert len(parsed) == 10000
        assert parsed['key_9999'] == 9999
    
    def test_mixed_types_in_array(self):
        """Test arrays with mixed types"""
        mixed = [1, 'string', True, None, {'nested': 'object'}, [1, 2, 3]]
        
        json_str = json.dumps(mixed)
        parsed = json.loads(json_str)
        
        assert len(parsed) == 6
        assert parsed[0] == 1
        assert parsed[1] == 'string'
        assert parsed[2] is True
        assert parsed[3] is None
        assert isinstance(parsed[4], dict)
        assert isinstance(parsed[5], list)
    
    def test_empty_json(self):
        """Test handling empty JSON"""
        assert json.loads('{}') == {}
        assert json.loads('[]') == []
        assert json.dumps({}) == '{}'
        assert json.dumps([]) == '[]'
    
    def test_single_value_json(self):
        """Test single value JSON (non-object/array)"""
        assert json.loads('null') is None
        assert json.loads('true') is True
        assert json.loads('false') is False
        assert json.loads('42') == 42
        assert json.loads('"string"') == 'string'