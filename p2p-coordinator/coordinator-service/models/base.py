"""
SQLAlchemy Base Model

Base class for all database models using SQLAlchemy 2.0 declarative base.
"""

from sqlalchemy.orm import DeclarativeBase


class Base(DeclarativeBase):
    """Base class for all SQLAlchemy models"""
    pass

