"""
Authentication API Endpoints

JWT token generation and validation for P2P clients.

NOTE: Authentication is OPTIONAL and can be disabled via configuration.
      P2P-DLL has graceful fallback if authentication is not available.
"""

from datetime import datetime, timedelta, timezone
from typing import Optional
from fastapi import APIRouter, HTTPException, status, Depends, Header
from pydantic import BaseModel, Field
from loguru import logger
import jwt

from config import settings


router = APIRouter(prefix="/api/v1/auth", tags=["auth"])


# Request/Response Models
class TokenRequest(BaseModel):
    """JWT token request"""
    player_id: str = Field(..., description="Player identifier")
    user_id: Optional[str] = Field(None, description="User identifier (optional)")
    api_key: Optional[str] = Field(None, description="API key for validation (optional)")


class TokenResponse(BaseModel):
    """JWT token response"""
    access_token: str = Field(..., description="JWT access token")
    refresh_token: Optional[str] = Field(None, description="JWT refresh token (optional)")
    token_type: str = Field(default="Bearer", description="Token type")
    expires_in: int = Field(..., description="Token expiration time in seconds")


class TokenRefreshRequest(BaseModel):
    """Token refresh request"""
    refresh_token: str = Field(..., description="Refresh token")


# Helper Functions
def create_access_token(data: dict, expires_delta: Optional[timedelta] = None) -> str:
    """
    Create a JWT access token
    
    Args:
        data: Data to encode in the token
        expires_delta: Token expiration time (default: 1 hour)
    
    Returns:
        Encoded JWT token
    """
    to_encode = data.copy()

    now = datetime.now(timezone.utc)
    if expires_delta:
        expire = now + expires_delta
    else:
        expire = now + timedelta(hours=1)

    to_encode.update({
        "exp": expire,
        "iat": now,
        "type": "access"
    })
    
    encoded_jwt = jwt.encode(
        to_encode,
        settings.security.jwt_secret_key,
        algorithm=settings.security.jwt_algorithm
    )
    
    return encoded_jwt


def verify_api_key(api_key: Optional[str]) -> bool:
    """
    Verify API key (optional validation)
    
    Args:
        api_key: API key to verify
    
    Returns:
        True if valid or validation is disabled, False otherwise
    """
    # If API key validation is disabled, always return True
    if not settings.security.api_key_validation_enabled:
        return True
    
    # If no API key provided but validation is required, return False
    if not api_key:
        return False
    
    # Verify against configured API key
    return api_key == settings.security.coordinator_api_key


# API Endpoints
@router.post("/token", response_model=TokenResponse, status_code=status.HTTP_200_OK)
async def generate_token(
    request: TokenRequest,
) -> TokenResponse:
    """
    Generate JWT access token for P2P client
    
    This endpoint generates a JWT token that can be used for authenticated requests.
    
    **NOTE**: Authentication is OPTIONAL. If JWT validation is disabled in configuration,
    tokens are still generated but not validated on subsequent requests.
    
    **API Key Validation**: If enabled, requires valid X-API-Key header or api_key in request body.
    """
    logger.info(f"Token request for player_id: {request.player_id}, user_id: {request.user_id}")
    
    # Verify API key if validation is enabled
    if not verify_api_key(request.api_key):
        logger.warning(f"Invalid API key for player_id: {request.player_id}")
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid API key",
        )
    
    # Create token payload
    token_data = {
        "sub": request.player_id,
        "player_id": request.player_id,
        "type": "access",
    }

    if request.user_id:
        token_data["user_id"] = request.user_id

    # Generate access token
    access_expires_delta = timedelta(seconds=settings.security.jwt_expiration_seconds)
    access_token = create_access_token(token_data, access_expires_delta)

    # Generate refresh token (valid for 7 days)
    refresh_token_data = {
        "player_id": request.player_id,
        "type": "refresh",
    }
    refresh_expires_delta = timedelta(days=7)
    refresh_token = create_access_token(refresh_token_data, refresh_expires_delta)

    logger.info(f"Access and refresh tokens generated for player_id: {request.player_id}")

    return TokenResponse(
        access_token=access_token,
        refresh_token=refresh_token,
        token_type="Bearer",
        expires_in=settings.security.jwt_expiration_seconds,
    )


@router.post("/refresh", response_model=TokenResponse, status_code=status.HTTP_200_OK)
async def refresh_token(
    request: TokenRefreshRequest,
) -> TokenResponse:
    """
    Refresh JWT access token using a refresh token

    This endpoint validates the refresh token and issues a new access token.
    Refresh tokens have a longer expiration time than access tokens.

    Args:
        request: Token refresh request containing the refresh token

    Returns:
        New access token with expiration time

    Raises:
        HTTPException: If refresh token is invalid or expired
    """
    try:
        # Decode and validate refresh token
        payload = jwt.decode(
            request.refresh_token,
            settings.security.jwt_secret_key,
            algorithms=[settings.security.jwt_algorithm]
        )

        # Extract player_id from refresh token
        player_id: int = payload.get("player_id")
        if player_id is None:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid refresh token: missing player_id",
            )

        # Verify this is a refresh token (not an access token)
        token_type: str = payload.get("type", "access")
        if token_type != "refresh":
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid token type: expected refresh token",
            )

        # Create new access token
        access_token_expires = timedelta(seconds=settings.security.jwt_expiration_seconds)
        access_token = create_access_token(
            data={"player_id": player_id, "type": "access"},
            expires_delta=access_token_expires
        )

        logger.info(f"Access token refreshed for player_id: {player_id}")

        return TokenResponse(
            access_token=access_token,
            token_type="Bearer",
            expires_in=settings.security.jwt_expiration_seconds,
        )

    except jwt.ExpiredSignatureError:
        logger.warning("Refresh token expired")
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Refresh token has expired. Please login again.",
        )
    except jwt.InvalidTokenError as e:
        logger.warning(f"Invalid refresh token: {e}")
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid refresh token",
        )

