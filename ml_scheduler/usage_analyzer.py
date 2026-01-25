#!/usr/bin/env python3
"""
Usage Pattern Analyzer
Analyzes historical server usage patterns to find optimal training windows
"""

import aiomysql
import asyncpg
import pandas as pd
import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Tuple, Optional
import logging

logger = logging.getLogger(__name__)


class UsagePatternAnalyzer:
    """
    Analyzes historical server usage to find optimal training windows
    
    Features:
    - Query player login patterns from game database
    - Query system metrics from ML database (if available)
    - Calculate usage scores by hour and day of week
    - Identify low-usage windows for training
    """
    
    def __init__(self, config: Dict):
        """
        Initialize usage pattern analyzer
        
        Args:
            config: Database configuration dictionary
        """
        self.config = config
        self.logger = logging.getLogger(__name__)
        self.mariadb_pool: Optional[aiomysql.Pool] = None
        self.postgres_pool: Optional[asyncpg.Pool] = None
    
    async def connect_mariadb(self) -> None:
        """Create MariaDB connection pool for game data"""
        try:
            self.mariadb_pool = await aiomysql.create_pool(
                host=self.config.get('mariadb_host', 'localhost'),
                port=self.config.get('mariadb_port', 3306),
                user=self.config.get('mariadb_user', 'ragnarok'),
                password=self.config.get('mariadb_password', ''),
                db=self.config.get('mariadb_database', 'ragnarok'),
                minsize=1,
                maxsize=5,
                autocommit=True
            )
            self.logger.info("MariaDB connection pool created")
        except Exception as e:
            self.logger.error(f"Failed to connect to MariaDB: {e}")
            raise
    
    async def connect_postgres(self) -> None:
        """Create PostgreSQL connection pool for ML metrics (optional)"""
        try:
            self.postgres_pool = await asyncpg.create_pool(
                host=self.config.get('postgres_host', 'localhost'),
                port=self.config.get('postgres_port', 5432),
                database=self.config.get('postgres_database', 'ragnarok_ml'),
                user=self.config.get('postgres_user', 'ml_user'),
                password=self.config.get('postgres_password', ''),
                min_size=1,
                max_size=5,
                command_timeout=60
            )
            self.logger.info("PostgreSQL connection pool created")
        except Exception as e:
            self.logger.warning(f"PostgreSQL connection failed (optional): {e}")
            self.postgres_pool = None
    
    async def close(self) -> None:
        """Close all database connections"""
        if self.mariadb_pool:
            self.mariadb_pool.close()
            await self.mariadb_pool.wait_closed()
            self.logger.info("MariaDB pool closed")
        
        if self.postgres_pool:
            await self.postgres_pool.close()
            self.logger.info("PostgreSQL pool closed")
    
    async def get_historical_player_counts(self, days: int = 30) -> pd.DataFrame:
        """
        Query historical player activity from character database
        
        Args:
            days: Number of days of history to analyze
        
        Returns:
            DataFrame with hourly player counts
        """
        self.logger.info(f"Querying player activity for last {days} days...")
        
        if not self.mariadb_pool:
            await self.connect_mariadb()
        
        try:
            async with self.mariadb_pool.acquire() as conn:
                async with conn.cursor(aiomysql.DictCursor) as cursor:
                    # Query character online time / last login patterns
                    # Note: RagnarokOnline `char` table typically has last_login timestamp
                    query = """
                    SELECT 
                        DATE_FORMAT(last_login, '%%Y-%%m-%%d %%H:00:00') as hour,
                        COUNT(DISTINCT account_id) as player_count
                    FROM `char`
                    WHERE last_login >= DATE_SUB(NOW(), INTERVAL %s DAY)
                        AND last_login IS NOT NULL
                    GROUP BY DATE_FORMAT(last_login, '%%Y-%%m-%%d %%H:00:00')
                    ORDER BY hour
                    """
                    
                    await cursor.execute(query, (days,))
                    rows = await cursor.fetchall()
                    
                    if not rows:
                        self.logger.warning("No player activity data found, using synthetic data")
                        return self._generate_synthetic_player_data(days)
                    
                    # Convert to DataFrame
                    df = pd.DataFrame(rows)
                    df['hour'] = pd.to_datetime(df['hour'])
                    df['player_count'] = df['player_count'].astype(int)
                    
                    self.logger.info(f"Retrieved {len(df)} hourly player count records")
                    self.logger.info(f"Player count range: {df['player_count'].min()} - {df['player_count'].max()}")
                    
                    return df
        
        except Exception as e:
            self.logger.error(f"Error querying player counts: {e}")
            self.logger.warning("Falling back to synthetic data")
            return self._generate_synthetic_player_data(days)
    
    def _generate_synthetic_player_data(self, days: int) -> pd.DataFrame:
        """
        Generate synthetic player data for testing (when no real data available)
        
        Args:
            days: Number of days to generate
        
        Returns:
            DataFrame with synthetic hourly player counts
        """
        self.logger.info("Generating synthetic player activity patterns...")
        
        # Generate hourly timestamps for the last N days
        end_time = datetime.now().replace(minute=0, second=0, microsecond=0)
        start_time = end_time - timedelta(days=days)
        
        hours = pd.date_range(start=start_time, end=end_time, freq='H')
        
        # Realistic player pattern: lower at night, higher evening/weekend
        player_counts = []
        for dt in hours:
            hour_of_day = dt.hour
            day_of_week = dt.weekday()
            
            # Base count varies by hour (peak at 20:00-22:00)
            if 20 <= hour_of_day <= 22:
                base = 150
            elif 18 <= hour_of_day <= 23:
                base = 120
            elif 12 <= hour_of_day <= 17:
                base = 80
            elif 6 <= hour_of_day <= 11:
                base = 50
            else:  # 0-5 = night (off-peak)
                base = 20
            
            # Weekend multiplier
            if day_of_week >= 5:  # Saturday, Sunday
                base *= 1.5
            
            # Add some randomness
            count = int(base + np.random.normal(0, base * 0.2))
            count = max(5, count)  # Minimum 5 players
            
            player_counts.append(count)
        
        df = pd.DataFrame({
            'hour': hours,
            'player_count': player_counts
        })
        
        self.logger.info(f"Generated {len(df)} synthetic hourly records")
        return df
    
    async def get_system_load_history(self, days: int = 30) -> pd.DataFrame:
        """
        Query TimescaleDB/PostgreSQL for historical system metrics
        
        Args:
            days: Number of days of history
        
        Returns:
            DataFrame with system metrics by hour
        """
        if not self.postgres_pool:
            await self.connect_postgres()
        
        if not self.postgres_pool:
            self.logger.warning("PostgreSQL not available, using synthetic system metrics")
            return self._generate_synthetic_system_data(days)
        
        try:
            async with self.postgres_pool.acquire() as conn:
                # Check if ml_system_health table exists
                table_exists = await conn.fetchval("""
                    SELECT EXISTS (
                        SELECT FROM information_schema.tables 
                        WHERE table_name = 'ml_system_health'
                    )
                """)
                
                if not table_exists:
                    self.logger.warning("ml_system_health table does not exist, using synthetic data")
                    return self._generate_synthetic_system_data(days)
                
                # Query system health metrics
                query = """
                SELECT 
                    DATE_TRUNC('hour', timestamp) as hour,
                    AVG(cpu_percent) as avg_cpu,
                    AVG(memory_percent) as avg_memory,
                    AVG(active_monsters) as avg_monsters
                FROM ml_system_health
                WHERE timestamp >= NOW() - INTERVAL '$1 days'
                GROUP BY DATE_TRUNC('hour', timestamp)
                ORDER BY hour
                """
                
                rows = await conn.fetch(query, days)
                
                if not rows:
                    self.logger.warning("No system health data found, using synthetic data")
                    return self._generate_synthetic_system_data(days)
                
                df = pd.DataFrame([
                    {
                        'hour': row['hour'],
                        'cpu': row['avg_cpu'],
                        'memory': row['avg_memory'],
                        'monsters': row['avg_monsters']
                    }
                    for row in rows
                ])
                
                self.logger.info(f"Retrieved {len(df)} hourly system metric records")
                return df
        
        except Exception as e:
            self.logger.warning(f"Error querying system metrics: {e}, using synthetic data")
            return self._generate_synthetic_system_data(days)
    
    def _generate_synthetic_system_data(self, days: int) -> pd.DataFrame:
        """
        Generate synthetic system metrics for testing
        
        Args:
            days: Number of days to generate
        
        Returns:
            DataFrame with synthetic system metrics
        """
        self.logger.info("Generating synthetic system metrics...")
        
        end_time = datetime.now().replace(minute=0, second=0, microsecond=0)
        start_time = end_time - timedelta(days=days)
        
        hours = pd.date_range(start=start_time, end=end_time, freq='H')
        
        data = []
        for dt in hours:
            hour_of_day = dt.hour
            
            # CPU/Memory correlate with player activity
            if 20 <= hour_of_day <= 22:
                cpu = 60 + np.random.normal(0, 10)
                memory = 70 + np.random.normal(0, 10)
                monsters = 500 + np.random.normal(0, 100)
            elif 0 <= hour_of_day <= 5:
                cpu = 20 + np.random.normal(0, 5)
                memory = 40 + np.random.normal(0, 5)
                monsters = 100 + np.random.normal(0, 20)
            else:
                cpu = 40 + np.random.normal(0, 10)
                memory = 55 + np.random.normal(0, 10)
                monsters = 300 + np.random.normal(0, 50)
            
            data.append({
                'hour': dt,
                'cpu': max(10, min(90, cpu)),
                'memory': max(30, min(85, memory)),
                'monsters': max(50, int(monsters))
            })
        
        df = pd.DataFrame(data)
        self.logger.info(f"Generated {len(df)} synthetic system metric records")
        return df
    
    def find_off_peak_windows(
        self, 
        player_df: pd.DataFrame, 
        system_df: pd.DataFrame,
        window_hours: int = 8,
        top_n: int = 3
    ) -> List[Dict]:
        """
        Identify off-peak time windows for training
        
        Args:
            player_df: Player count DataFrame
            system_df: System metrics DataFrame
            window_hours: Duration of training window in hours
            top_n: Number of best windows to return
        
        Returns:
            List of dictionaries with window info (day, start_hour, end_hour, score)
        """
        self.logger.info(f"Analyzing usage patterns to find {window_hours}-hour off-peak windows...")
        
        # Merge DataFrames on hour
        combined = pd.merge(player_df, system_df, on='hour', how='outer')
        combined = combined.fillna(method='ffill').fillna(0)
        
        # Extract temporal features
        combined['hour_of_day'] = combined['hour'].dt.hour
        combined['day_of_week'] = combined['hour'].dt.dayofweek
        
        # Calculate usage score (lower is better for training)
        # Weights: players (high), CPU (medium), memory (medium), monsters (low)
        combined['usage_score'] = (
            combined['player_count'] * 2.0 +
            combined['cpu'] * 0.5 +
            combined['memory'] * 0.3 +
            combined['monsters'] * 0.2
        )
        
        self.logger.info(f"Usage score range: {combined['usage_score'].min():.1f} - {combined['usage_score'].max():.1f}")
        
        # Group by day of week and hour of day, calculate average usage
        hourly_avg = combined.groupby(['day_of_week', 'hour_of_day']).agg({
            'usage_score': 'mean',
            'player_count': 'mean',
            'cpu': 'mean',
            'memory': 'mean'
        }).reset_index()
        
        # Find continuous low-usage windows for each day of week
        windows = []
        
        for day in range(7):
            day_data = hourly_avg[hourly_avg['day_of_week'] == day].sort_values('hour_of_day')
            
            if len(day_data) < window_hours:
                continue
            
            # Sliding window to find best continuous window
            for start_hour in range(24 - window_hours + 1):
                end_hour = start_hour + window_hours
                
                window_data = day_data[
                    (day_data['hour_of_day'] >= start_hour) & 
                    (day_data['hour_of_day'] < end_hour)
                ]
                
                if len(window_data) < window_hours:
                    continue
                
                avg_score = window_data['usage_score'].mean()
                avg_players = window_data['player_count'].mean()
                avg_cpu = window_data['cpu'].mean()
                avg_memory = window_data['memory'].mean()
                
                windows.append({
                    'day': day,
                    'start_hour': start_hour,
                    'end_hour': end_hour,
                    'score': avg_score,
                    'avg_players': avg_players,
                    'avg_cpu': avg_cpu,
                    'avg_memory': avg_memory
                })
        
        # Sort by score (lowest first = best for training)
        windows_sorted = sorted(windows, key=lambda x: x['score'])
        
        # Get top N best windows
        best_windows = windows_sorted[:top_n]
        
        day_names = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
        
        self.logger.info(f"\nBest {len(best_windows)} training windows identified:")
        for i, w in enumerate(best_windows, 1):
            self.logger.info(
                f"  {i}. {day_names[w['day']]} {w['start_hour']:02d}:00-{w['end_hour']:02d}:00 | "
                f"Score: {w['score']:.1f} | "
                f"Players: {w['avg_players']:.0f} | "
                f"CPU: {w['avg_cpu']:.0f}% | "
                f"Mem: {w['avg_memory']:.0f}%"
            )
        
        return best_windows
    
    def is_in_window(self, windows: List[Dict], dt: Optional[datetime] = None) -> bool:
        """
        Check if current time (or given time) is within an off-peak window
        
        Args:
            windows: List of window dictionaries from find_off_peak_windows()
            dt: DateTime to check (None = now)
        
        Returns:
            True if time is in an off-peak window
        """
        if dt is None:
            dt = datetime.now()
        
        current_day = dt.weekday()
        current_hour = dt.hour
        
        for window in windows:
            if window['day'] == current_day:
                if window['start_hour'] <= current_hour < window['end_hour']:
                    return True
        
        return False


async def main():
    """Test the usage analyzer"""
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    # Test configuration
    config = {
        'mariadb_host': '192.168.0.100',
        'mariadb_port': 3306,
        'mariadb_database': 'ragnarok',
        'mariadb_user': 'ragnarok',
        'mariadb_password': 'ragnarok',
        'postgres_host': 'localhost',
        'postgres_port': 5432,
        'postgres_database': 'ragnarok_ml',
        'postgres_user': 'ml_user',
        'postgres_password': 'ml_secure_password_2026'
    }
    
    analyzer = UsagePatternAnalyzer(config)
    
    try:
        # Get historical data
        player_df = await analyzer.get_historical_player_counts(days=30)
        system_df = await analyzer.get_system_load_history(days=30)
        
        # Find off-peak windows
        windows = analyzer.find_off_peak_windows(player_df, system_df, window_hours=8)
        
        # Check if current time is in window
        is_offpeak = analyzer.is_in_window(windows)
        print(f"\nCurrent time is {'IN' if is_offpeak else 'NOT IN'} an off-peak window")
        
    finally:
        await analyzer.close()


if __name__ == '__main__':
    import asyncio
    asyncio.run(main())
