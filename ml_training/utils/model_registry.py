"""
ML Monster AI - PostgreSQL Model Registry
Enhanced ML Monster AI System v2.0

Integration with PostgreSQL ml_models table for tracking trained models
"""

import asyncpg
import logging
from typing import Dict, Any, Optional, List
from datetime import datetime
import json

logger = logging.getLogger(__name__)


class ModelRegistry:
    """
    PostgreSQL-backed model registry
    
    Tracks all trained models in ml_models table:
    - Model metadata
    - Training metrics
    - Deployment status
    - Version history
    """
    
    def __init__(self, db_config: Optional[Dict[str, str]] = None):
        """
        Initialize model registry
        
        Args:
            db_config: Database connection configuration
        """
        self.db_config = db_config or self.get_default_config()
        self.pool = None
        
        logger.info("ModelRegistry initialized")
    
    @staticmethod
    def get_default_config() -> Dict[str, str]:
        """Get default database configuration"""
        return {
            'host': 'localhost',
            'port': 5432,
            'database': 'ragnarok_ml',
            'user': 'ml_user',
            'password': os.getenv('POSTGRES_PASSWORD', '')
        }
    
    async def connect(self):
        """Establish database connection pool"""
        try:
            self.pool = await asyncpg.create_pool(
                host=self.db_config['host'],
                port=self.db_config['port'],
                database=self.db_config['database'],
                user=self.db_config['user'],
                password=self.db_config['password'],
                min_size=5,
                max_size=20,
                command_timeout=10.0
            )
            
            logger.info("PostgreSQL connection pool created")
            
            # Verify ml_models table exists
            await self.ensure_table_exists()
            
            return True
        
        except Exception as e:
            logger.error(f"Failed to connect to PostgreSQL: {e}")
            return False
    
    async def ensure_table_exists(self):
        """Ensure ml_models table exists"""
        create_table_sql = """
        CREATE TABLE IF NOT EXISTS ml_models (
            id SERIAL PRIMARY KEY,
            archetype VARCHAR(50) NOT NULL,
            model_type VARCHAR(50) NOT NULL,
            version VARCHAR(50) NOT NULL,
            file_path TEXT NOT NULL,
            metrics JSONB,
            config JSONB,
            parameters BIGINT,
            size_mb FLOAT,
            created_at TIMESTAMP DEFAULT NOW(),
            deployed_at TIMESTAMP,
            is_deployed BOOLEAN DEFAULT FALSE,
            training_time_hours FLOAT,
            notes TEXT,
            UNIQUE(archetype, model_type, version)
        );
        
        CREATE INDEX IF NOT EXISTS idx_ml_models_archetype ON ml_models(archetype);
        CREATE INDEX IF NOT EXISTS idx_ml_models_deployed ON ml_models(is_deployed);
        CREATE INDEX IF NOT EXISTS idx_ml_models_created ON ml_models(created_at DESC);
        """
        
        async with self.pool.acquire() as conn:
            await conn.execute(create_table_sql)
        
        logger.info("ml_models table verified")
    
    async def register_model(
        self,
        archetype: str,
        model_type: str,
        version: str,
        file_path: str,
        metrics: Dict[str, float],
        config: Dict[str, Any],
        parameters: int,
        size_mb: float,
        training_time_hours: Optional[float] = None,
        notes: Optional[str] = None
    ) -> int:
        """
        Register a trained model
        
        Args:
            archetype: Model archetype
            model_type: Model type
            version: Version string
            file_path: Path to model file
            metrics: Training/validation metrics
            config: Model configuration
            parameters: Number of parameters
            size_mb: Model size in MB
            training_time_hours: Training duration
            notes: Optional notes
        
        Returns:
            Model ID
        """
        query = """
        INSERT INTO ml_models 
        (archetype, model_type, version, file_path, metrics, config, 
         parameters, size_mb, training_time_hours, notes)
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
        ON CONFLICT (archetype, model_type, version) 
        DO UPDATE SET
            file_path = EXCLUDED.file_path,
            metrics = EXCLUDED.metrics,
            config = EXCLUDED.config,
            parameters = EXCLUDED.parameters,
            size_mb = EXCLUDED.size_mb,
            training_time_hours = EXCLUDED.training_time_hours,
            notes = EXCLUDED.notes,
            created_at = NOW()
        RETURNING id
        """
        
        async with self.pool.acquire() as conn:
            model_id = await conn.fetchval(
                query,
                archetype,
                model_type,
                version,
                file_path,
                json.dumps(metrics),
                json.dumps(config),
                parameters,
                size_mb,
                training_time_hours,
                notes
            )
        
        logger.info(f"Model registered: {archetype}/{model_type} v{version} (ID: {model_id})")
        
        return model_id
    
    async def get_latest_model(
        self,
        archetype: str,
        model_type: str
    ) -> Optional[Dict[str, Any]]:
        """
        Get latest model version
        
        Args:
            archetype: Model archetype
            model_type: Model type
        
        Returns:
            Model record or None
        """
        query = """
        SELECT * FROM ml_models
        WHERE archetype = $1 AND model_type = $2
        ORDER BY created_at DESC
        LIMIT 1
        """
        
        async with self.pool.acquire() as conn:
            record = await conn.fetchrow(query, archetype, model_type)
        
        if record:
            return dict(record)
        
        return None
    
    async def get_best_model(
        self,
        archetype: str,
        model_type: str,
        metric: str = 'val_loss',
        higher_is_better: bool = False
    ) -> Optional[Dict[str, Any]]:
        """
        Get best model based on metric
        
        Args:
            archetype: Model archetype
            model_type: Model type
            metric: Metric to optimize
            higher_is_better: Whether higher values are better
        
        Returns:
            Best model record or None
        """
        order = "DESC" if higher_is_better else "ASC"
        
        query = f"""
        SELECT * FROM ml_models
        WHERE archetype = $1 AND model_type = $2
          AND metrics ? $3
        ORDER BY (metrics->>$3)::FLOAT {order}
        LIMIT 1
        """
        
        async with self.pool.acquire() as conn:
            record = await conn.fetchrow(query, archetype, model_type, metric)
        
        if record:
            return dict(record)
        
        return None
    
    async def mark_deployed(
        self,
        archetype: str,
        model_type: str,
        version: str
    ) -> bool:
        """
        Mark model as deployed
        
        Args:
            archetype: Model archetype
            model_type: Model type
            version: Version string
        
        Returns:
            True if successful
        """
        # Unmark any previously deployed models
        query_unmark = """
        UPDATE ml_models
        SET is_deployed = FALSE, deployed_at = NULL
        WHERE archetype = $1 AND model_type = $2 AND is_deployed = TRUE
        """
        
        # Mark new model as deployed
        query_mark = """
        UPDATE ml_models
        SET is_deployed = TRUE, deployed_at = NOW()
        WHERE archetype = $1 AND model_type = $2 AND version = $3
        RETURNING id
        """
        
        async with self.pool.acquire() as conn:
            # Unmark old
            await conn.execute(query_unmark, archetype, model_type)
            
            # Mark new
            model_id = await conn.fetchval(query_mark, archetype, model_type, version)
        
        if model_id:
            logger.info(f"Marked as deployed: {archetype}/{model_type} v{version}")
            return True
        
        logger.warning(f"Failed to mark as deployed: {archetype}/{model_type} v{version}")
        return False
    
    async def get_deployed_models(self) -> List[Dict[str, Any]]:
        """
        Get all currently deployed models
        
        Returns:
            List of deployed model records
        """
        query = """
        SELECT * FROM ml_models
        WHERE is_deployed = TRUE
        ORDER BY archetype, model_type
        """
        
        async with self.pool.acquire() as conn:
            records = await conn.fetch(query)
        
        return [dict(r) for r in records]
    
    async def get_model_history(
        self,
        archetype: str,
        model_type: str,
        limit: int = 10
    ) -> List[Dict[str, Any]]:
        """
        Get version history for a model
        
        Args:
            archetype: Model archetype
            model_type: Model type
            limit: Maximum number of versions
        
        Returns:
            List of model records (newest first)
        """
        query = """
        SELECT * FROM ml_models
        WHERE archetype = $1 AND model_type = $2
        ORDER BY created_at DESC
        LIMIT $3
        """
        
        async with self.pool.acquire() as conn:
            records = await conn.fetch(query, archetype, model_type, limit)
        
        return [dict(r) for r in records]
    
    async def delete_old_versions(
        self,
        archetype: str,
        model_type: str,
        keep_last_n: int = 5
    ) -> int:
        """
        Delete old model versions (keep last N)
        
        Args:
            archetype: Model archetype
            model_type: Model type
            keep_last_n: Number of versions to keep
        
        Returns:
            Number of deleted records
        """
        query = """
        WITH ranked_models AS (
            SELECT id, ROW_NUMBER() OVER (ORDER BY created_at DESC) as rn
            FROM ml_models
            WHERE archetype = $1 AND model_type = $2 AND is_deployed = FALSE
        )
        DELETE FROM ml_models
        WHERE id IN (
            SELECT id FROM ranked_models WHERE rn > $3
        )
        RETURNING id
        """
        
        async with self.pool.acquire() as conn:
            deleted_ids = await conn.fetch(query, archetype, model_type, keep_last_n)
        
        count = len(deleted_ids)
        
        if count > 0:
            logger.info(f"Deleted {count} old versions of {archetype}/{model_type}")
        
        return count
    
    async def get_statistics(self) -> Dict[str, Any]:
        """
        Get registry statistics
        
        Returns:
            Dictionary with statistics
        """
        query = """
        SELECT 
            COUNT(*) as total_models,
            COUNT(DISTINCT archetype) as num_archetypes,
            COUNT(DISTINCT model_type) as num_model_types,
            SUM(CASE WHEN is_deployed THEN 1 ELSE 0 END) as deployed_models,
            AVG(size_mb) as avg_size_mb,
            SUM(size_mb) as total_size_mb,
            AVG(parameters) as avg_parameters,
            SUM(parameters) as total_parameters
        FROM ml_models
        """
        
        async with self.pool.acquire() as conn:
            record = await conn.fetchrow(query)
        
        stats = dict(record)
        
        logger.info(f"Registry Statistics:")
        logger.info(f"  Total Models: {stats['total_models']}")
        logger.info(f"  Deployed: {stats['deployed_models']}")
        logger.info(f"  Total Size: {stats['total_size_mb']:.2f} MB")
        logger.info(f"  Total Parameters: {stats['total_parameters']:,}")
        
        return stats
    
    async def close(self):
        """Close database connection pool"""
        if self.pool:
            await self.pool.close()
            logger.info("PostgreSQL connection pool closed")


# Standalone functions for synchronous usage

def register_model_sync(
    archetype: str,
    model_type: str,
    version: str,
    file_path: str,
    metrics: Dict[str, float],
    db_config: Optional[Dict[str, str]] = None
) -> bool:
    """
    Register model (synchronous wrapper)
    
    Args:
        archetype: Model archetype
        model_type: Model type
        version: Version string
        file_path: Model file path
        metrics: Training metrics
        db_config: Database config
    
    Returns:
        True if successful
    """
    import asyncio
    
    async def _register():
        registry = ModelRegistry(db_config)
        await registry.connect()
        
        try:
            await registry.register_model(
                archetype=archetype,
                model_type=model_type,
                version=version,
                file_path=file_path,
                metrics=metrics,
                config={},
                parameters=0,
                size_mb=0.0
            )
            
            return True
        
        finally:
            await registry.close()
    
    try:
        return asyncio.run(_register())
    except Exception as e:
        logger.error(f"Failed to register model: {e}")
        return False


import os
