"""
Error Handling Decorators

Standardized error handling decorators for API endpoints, database operations,
and LLM calls. Provides consistent error responses and logging.

Features:
- @handle_api_errors - API endpoint error handling
- @handle_db_errors - Database operation error handling
- @handle_llm_errors - LLM call error handling
- Error classification (retriable, fatal, user)
- Correlation ID injection
- Structured logging
"""

import functools
import logging
from typing import Callable, Optional, Type
from enum import Enum

from fastapi import HTTPException, status
from sqlalchemy.exc import SQLAlchemyError, IntegrityError, OperationalError
from pydantic import ValidationError

logger = logging.getLogger(__name__)


class ErrorType(str, Enum):
    """Error classification types."""
    
    USER_ERROR = "user_error"          # User input errors (400)
    NOT_FOUND = "not_found"            # Resource not found (404)
    AUTHENTICATION = "authentication"   # Auth errors (401)
    AUTHORIZATION = "authorization"     # Permission errors (403)
    RATE_LIMIT = "rate_limit"          # Rate limiting (429)
    SERVER_ERROR = "server_error"      # Internal errors (500)
    DATABASE_ERROR = "database_error"  # DB errors (503)
    EXTERNAL_API = "external_api"      # External API errors (502)
    TIMEOUT = "timeout"                # Timeout errors (504)


class RetryableError(Exception):
    """Exception for errors that can be retried."""
    pass


def create_error_response(
    error_type: ErrorType,
    message: str,
    details: Optional[dict] = None,
    correlation_id: Optional[str] = None
) -> dict:
    """
    Create standardized error response.
    
    Args:
        error_type: Type of error
        message: Error message
        details: Additional error details
        correlation_id: Request correlation ID
        
    Returns:
        dict: Standardized error response
    """
    response = {
        'error': {
            'type': error_type.value,
            'message': message,
        }
    }
    
    if details:
        response['error']['details'] = details
    
    if correlation_id:
        response['error']['correlation_id'] = correlation_id
    
    return response


def handle_api_errors(
    include_traceback: bool = False,
    log_errors: bool = True
):
    """
    Decorator for handling API endpoint errors.
    
    Usage:
        @handle_api_errors(include_traceback=False)
        async def my_endpoint():
            # Your code here
            pass
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            correlation_id = kwargs.get('correlation_id', 'unknown')
            
            try:
                return await func(*args, **kwargs)
                
            except HTTPException:
                # Re-raise FastAPI HTTPExceptions unchanged
                raise
                
            except ValidationError as e:
                if log_errors:
                    logger.warning(
                        f"Validation error in {func.__name__}: {str(e)}",
                        extra={'correlation_id': correlation_id}
                    )
                
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=create_error_response(
                        ErrorType.USER_ERROR,
                        "Invalid input data",
                        details={'validation_errors': e.errors()} if include_traceback else None,
                        correlation_id=correlation_id
                    )
                )
                
            except PermissionError as e:
                if log_errors:
                    logger.warning(
                        f"Permission denied in {func.__name__}: {str(e)}",
                        extra={'correlation_id': correlation_id}
                    )
                
                raise HTTPException(
                    status_code=status.HTTP_403_FORBIDDEN,
                    detail=create_error_response(
                        ErrorType.AUTHORIZATION,
                        "Permission denied",
                        correlation_id=correlation_id
                    )
                )
                
            except TimeoutError as e:
                if log_errors:
                    logger.error(
                        f"Timeout in {func.__name__}: {str(e)}",
                        extra={'correlation_id': correlation_id}
                    )
                
                raise HTTPException(
                    status_code=status.HTTP_504_GATEWAY_TIMEOUT,
                    detail=create_error_response(
                        ErrorType.TIMEOUT,
                        "Request timeout",
                        correlation_id=correlation_id
                    )
                )
                
            except Exception as e:
                if log_errors:
                    logger.error(
                        f"Unexpected error in {func.__name__}: {str(e)}",
                        exc_info=True,
                        extra={'correlation_id': correlation_id}
                    )
                
                raise HTTPException(
                    status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                    detail=create_error_response(
                        ErrorType.SERVER_ERROR,
                        "Internal server error",
                        details={'error': str(e)} if include_traceback else None,
                        correlation_id=correlation_id
                    )
                )
        
        return wrapper
    return decorator


def handle_db_errors(
    retry_on_operational: bool = True,
    max_retries: int = 3
):
    """
    Decorator for handling database operation errors.
    
    Usage:
        @handle_db_errors(retry_on_operational=True)
        async def my_db_operation():
            # Your database code here
            pass
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            correlation_id = kwargs.get('correlation_id', 'unknown')
            attempts = 0
            last_error = None
            
            while attempts < max_retries:
                try:
                    return await func(*args, **kwargs)
                    
                except IntegrityError as e:
                    # Don't retry integrity errors (unique constraint, etc.)
                    logger.error(
                        f"Database integrity error in {func.__name__}: {str(e)}",
                        extra={'correlation_id': correlation_id}
                    )
                    raise HTTPException(
                        status_code=status.HTTP_409_CONFLICT,
                        detail=create_error_response(
                            ErrorType.DATABASE_ERROR,
                            "Database constraint violation",
                            correlation_id=correlation_id
                        )
                    )
                    
                except OperationalError as e:
                    attempts += 1
                    last_error = e
                    
                    if attempts < max_retries and retry_on_operational:
                        logger.warning(
                            f"Database operational error in {func.__name__}, "
                            f"retry {attempts}/{max_retries}: {str(e)}",
                            extra={'correlation_id': correlation_id}
                        )
                        continue
                    else:
                        logger.error(
                            f"Database operational error in {func.__name__} "
                            f"after {attempts} attempts: {str(e)}",
                            extra={'correlation_id': correlation_id}
                        )
                        raise HTTPException(
                            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
                            detail=create_error_response(
                                ErrorType.DATABASE_ERROR,
                                "Database temporarily unavailable",
                                correlation_id=correlation_id
                            )
                        )
                        
                except SQLAlchemyError as e:
                    logger.error(
                        f"Database error in {func.__name__}: {str(e)}",
                        exc_info=True,
                        extra={'correlation_id': correlation_id}
                    )
                    raise HTTPException(
                        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                        detail=create_error_response(
                            ErrorType.DATABASE_ERROR,
                            "Database operation failed",
                            correlation_id=correlation_id
                        )
                    )
                    
                except Exception as e:
                    logger.error(
                        f"Unexpected error in database operation {func.__name__}: {str(e)}",
                        exc_info=True,
                        extra={'correlation_id': correlation_id}
                    )
                    raise
        
        return wrapper
    return decorator


def handle_llm_errors(
    retry_on_rate_limit: bool = True,
    max_retries: int = 3,
    fallback_provider: Optional[str] = None
):
    """
    Decorator for handling LLM API call errors.
    
    Usage:
        @handle_llm_errors(retry_on_rate_limit=True, fallback_provider='ollama')
        async def call_llm():
            # Your LLM code here
            pass
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            correlation_id = kwargs.get('correlation_id', 'unknown')
            attempts = 0
            
            while attempts < max_retries:
                try:
                    return await func(*args, **kwargs)
                    
                except TimeoutError as e:
                    attempts += 1
                    logger.warning(
                        f"LLM timeout in {func.__name__}, "
                        f"attempt {attempts}/{max_retries}: {str(e)}",
                        extra={'correlation_id': correlation_id}
                    )
                    
                    if attempts >= max_retries:
                        raise HTTPException(
                            status_code=status.HTTP_504_GATEWAY_TIMEOUT,
                            detail=create_error_response(
                                ErrorType.TIMEOUT,
                                "LLM request timeout",
                                correlation_id=correlation_id
                            )
                        )
                    continue
                    
                except RetryableError as e:
                    attempts += 1
                    logger.warning(
                        f"Retriable LLM error in {func.__name__}, "
                        f"attempt {attempts}/{max_retries}: {str(e)}",
                        extra={'correlation_id': correlation_id}
                    )
                    
                    if attempts >= max_retries:
                        if fallback_provider:
                            logger.info(
                                f"Switching to fallback provider: {fallback_provider}",
                                extra={'correlation_id': correlation_id}
                            )
                            # Fallback logic would be handled by caller
                            raise
                        
                        raise HTTPException(
                            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
                            detail=create_error_response(
                                ErrorType.EXTERNAL_API,
                                "LLM service temporarily unavailable",
                                correlation_id=correlation_id
                            )
                        )
                    continue
                    
                except Exception as e:
                    logger.error(
                        f"LLM error in {func.__name__}: {str(e)}",
                        exc_info=True,
                        extra={'correlation_id': correlation_id}
                    )
                    
                    raise HTTPException(
                        status_code=status.HTTP_502_BAD_GATEWAY,
                        detail=create_error_response(
                            ErrorType.EXTERNAL_API,
                            "LLM service error",
                            correlation_id=correlation_id
                        )
                    )
        
        return wrapper
    return decorator


def is_retriable_error(error: Exception) -> bool:
    """
    Determine if an error is retriable.
    
    Args:
        error: Exception to check
        
    Returns:
        bool: True if error can be retried
    """
    retriable_types = (
        TimeoutError,
        ConnectionError,
        OperationalError,
        RetryableError
    )
    
    return isinstance(error, retriable_types)


def classify_http_status(status_code: int) -> ErrorType:
    """
    Classify HTTP status code to ErrorType.
    
    Args:
        status_code: HTTP status code
        
    Returns:
        ErrorType: Classified error type
    """
    if status_code == 400:
        return ErrorType.USER_ERROR
    elif status_code == 401:
        return ErrorType.AUTHENTICATION
    elif status_code == 403:
        return ErrorType.AUTHORIZATION
    elif status_code == 404:
        return ErrorType.NOT_FOUND
    elif status_code == 429:
        return ErrorType.RATE_LIMIT
    elif status_code == 500:
        return ErrorType.SERVER_ERROR
    elif status_code == 502:
        return ErrorType.EXTERNAL_API
    elif status_code == 503:
        return ErrorType.DATABASE_ERROR
    elif status_code == 504:
        return ErrorType.TIMEOUT
    else:
        return ErrorType.SERVER_ERROR