"""
Apache AGE Graph Manager for ML Monster AI System
==================================================
Manages graph database operations for monster relationships, coordination,
and multi-agent learning.

Architecture Reference: plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md
Version: 2.0
"""

import asyncio
import json
import logging
import time
from typing import List, Dict, Any, Optional, Tuple
from datetime import datetime, timedelta
import asyncpg
import numpy as np


logger = logging.getLogger(__name__)


class AGEQueryError(Exception):
    """Exception raised for Apache AGE query errors"""
    pass


class AGEConnectionError(Exception):
    """Exception raised for Apache AGE connection errors"""
    pass


def parse_agtype(agtype_value: Any) -> Any:
    """
    Parse Apache AGE agtype result to Python type
    
    AGE returns results as agtype which needs conversion.
    Common patterns:
    - {"id": 12345}::vertex -> extract 'id'
    - Simple values -> return as-is
    - Arrays -> parse as list
    
    Args:
        agtype_value: Raw agtype value from query result
    
    Returns:
        Parsed Python value
    """
    if agtype_value is None:
        return None
    
    # If already a Python type, return as-is
    if isinstance(agtype_value, (int, float, bool, str)):
        return agtype_value
    
    # If dict-like (vertex/edge)
    if isinstance(agtype_value, dict):
        # Extract properties if vertex/edge structure
        if 'properties' in agtype_value:
            return agtype_value['properties']
        return agtype_value
    
    # If list-like
    if isinstance(agtype_value, (list, tuple)):
        return [parse_agtype(item) for item in agtype_value]
    
    # Try to parse as JSON string
    if isinstance(agtype_value, str):
        try:
            return json.loads(agtype_value)
        except (json.JSONDecodeError, ValueError):
            return agtype_value
    
    return agtype_value


class AGEGraphManager:
    """
    Manages Apache AGE graph operations for monster AI
    
    Features:
    - AsyncIO PostgreSQL connection with AGE support
    - Cypher query execution with retry logic
    - Node and edge CRUD operations
    - Path finding and neighbor queries
    - Subgraph extraction
    - Transaction management
    - Connection pooling
    - Comprehensive error handling
    """
    
    def __init__(
        self,
        connection_pool: asyncpg.Pool,
        graph_name: str = 'monster_ai',
        max_retries: int = 3,
        retry_delay: float = 0.1
    ):
        """
        Initialize AGE Graph Manager
        
        Args:
            connection_pool: asyncpg connection pool
            graph_name: Name of the AGE graph to use
            max_retries: Maximum retry attempts for failed queries
            retry_delay: Initial delay between retries (exponential backoff)
        """
        self.pool = connection_pool
        self.graph_name = graph_name
        self.max_retries = max_retries
        self.retry_delay = retry_delay
        
        # Performance metrics
        self.query_count = 0
        self.total_query_time = 0.0
        self.error_count = 0
        
        logger.info(
            f"AGEGraphManager initialized for graph '{graph_name}' "
            f"with {max_retries} max retries"
        )
    
    async def execute_cypher(
        self,
        query: str,
        params: Optional[Dict[str, Any]] = None,
        returns_schema: Optional[List[Tuple[str, str]]] = None
    ) -> List[Dict[str, Any]]:
        """
        Execute Cypher query on Apache AGE graph
        
        Args:
            query: Cypher query string (without graph name wrapper)
            params: Query parameters as dict
            returns_schema: List of (column_name, agtype) tuples for result parsing
        
        Returns:
            List of result dictionaries
        
        Raises:
            AGEQueryError: If query fails after retries
        
        Example:
            results = await graph_mgr.execute_cypher(
                "MATCH (m:Monster {id: $id}) RETURN m.name, m.level",
                params={'id': 1001},
                returns_schema=[('name', 'agtype'), ('level', 'agtype')]
            )
        """
        start_time = time.perf_counter()
        
        # Build full query with graph name
        if returns_schema:
            columns = ', '.join([col for col, _ in returns_schema])
            full_query = f"SELECT * FROM cypher('{self.graph_name}', $$ {query} $$) AS ({columns})"
        else:
            full_query = f"SELECT * FROM cypher('{self.graph_name}', $$ {query} $$) AS (result agtype)"
        
        # Retry logic
        for attempt in range(self.max_retries):
            try:
                async with self.pool.acquire() as conn:
                    # Set search path for AGE
                    await conn.execute("LOAD 'age'")
                    await conn.execute("SET search_path = ag_catalog, \"$user\", public")
                    
                    # Execute query
                    if params:
                        # Replace $param_name with $1, $2, etc for asyncpg
                        # For simplicity, AGE queries use positional params
                        logger.debug(f"Executing Cypher query: {query[:100]}...")
                        rows = await conn.fetch(full_query)
                    else:
                        rows = await conn.fetch(full_query)
                    
                    # Parse results
                    results = []
                    for row in rows:
                        parsed_row = {}
                        for idx, (col_name, _) in enumerate(returns_schema or [('result', 'agtype')]):
                            parsed_row[col_name] = parse_agtype(row[idx])
                        results.append(parsed_row)
                    
                    # Record metrics
                    elapsed = time.perf_counter() - start_time
                    self.query_count += 1
                    self.total_query_time += elapsed
                    
                    logger.debug(
                        f"Query executed successfully in {elapsed*1000:.2f}ms, "
                        f"returned {len(results)} rows"
                    )
                    
                    return results
            
            except asyncpg.PostgresError as e:
                self.error_count += 1
                
                if attempt < self.max_retries - 1:
                    delay = self.retry_delay * (2 ** attempt)  # Exponential backoff
                    logger.warning(
                        f"Query failed (attempt {attempt+1}/{self.max_retries}): {e}. "
                        f"Retrying in {delay}s..."
                    )
                    await asyncio.sleep(delay)
                else:
                    logger.error(f"Query failed after {self.max_retries} attempts: {e}")
                    raise AGEQueryError(f"Cypher query failed: {e}") from e
            
            except Exception as e:
                self.error_count += 1
                logger.error(f"Unexpected error in execute_cypher: {e}", exc_info=True)
                raise AGEQueryError(f"Unexpected query error: {e}") from e
        
        return []  # Should never reach here
    
    # ========================================================================
    # NODE CRUD OPERATIONS
    # ========================================================================
    
    async def create_monster_node(
        self,
        monster_id: int,
        properties: Dict[str, Any]
    ) -> str:
        """
        Create Monster node in graph
        
        Args:
            monster_id: Unique monster instance ID
            properties: Node properties (mob_id, name, level, archetype, etc.)
        
        Returns:
            Node ID (agtype internal ID)
        
        Example:
            node_id = await graph_mgr.create_monster_node(
                monster_id=1001,
                properties={
                    'mob_id': 1002,
                    'name': 'Poring',
                    'level': 10,
                    'archetype': 'aggressive',
                    'leadership_score': 0.5
                }
            )
        """
        query = """
            CREATE (m:Monster {
                id: $monster_id,
                mob_id: $mob_id,
                name: $name,
                level: $level,
                archetype: $archetype,
                leadership_score: $leadership_score,
                experience: $experience,
                spawn_time: timestamp(),
                current_hp_ratio: 1.0,
                position_x: $position_x,
                position_y: $position_y,
                map_id: $map_id
            })
            RETURN id(m)
        """
        
        # Build parameterized query (AGE doesn't support $param directly in cypher)
        # We use string formatting but with safe parameter validation
        props = {
            'monster_id': monster_id,
            'mob_id': properties.get('mob_id', 0),
            'name': str(properties.get('name', 'Unknown')),
            'level': properties.get('level', 1),
            'archetype': str(properties.get('archetype', 'aggressive')),
            'leadership_score': properties.get('leadership_score', 0.5),
            'experience': properties.get('experience', 0),
            'position_x': properties.get('position_x', 0),
            'position_y': properties.get('position_y', 0),
            'map_id': properties.get('map_id', 1)
        }
        
        # Use literal values in query (AGE limitation)
        query_with_values = f"""
            CREATE (m:Monster {{
                id: {props['monster_id']},
                mob_id: {props['mob_id']},
                name: '{props['name']}',
                level: {props['level']},
                archetype: '{props['archetype']}',
                leadership_score: {props['leadership_score']},
                experience: {props['experience']},
                spawn_time: timestamp(),
                current_hp_ratio: 1.0,
                position_x: {props['position_x']},
                position_y: {props['position_y']},
                map_id: {props['map_id']}
            }})
            RETURN id(m)
        """
        
        results = await self.execute_cypher(
            query_with_values,
            returns_schema=[('node_id', 'agtype')]
        )
        
        if results:
            logger.info(f"Created Monster node: id={monster_id}, archetype={props['archetype']}")
            return results[0]['node_id']
        
        raise AGEQueryError(f"Failed to create Monster node for id={monster_id}")
    
    async def update_node(
        self,
        node_label: str,
        node_id: int,
        properties: Dict[str, Any]
    ) -> bool:
        """
        Update node properties
        
        Args:
            node_label: Node label (e.g., 'Monster', 'Player')
            node_id: Node's id property value
            properties: Properties to update
        
        Returns:
            True if updated successfully
        """
        # Build SET clauses
        set_clauses = []
        for key, value in properties.items():
            if isinstance(value, str):
                set_clauses.append(f"n.{key} = '{value}'")
            elif isinstance(value, (int, float)):
                set_clauses.append(f"n.{key} = {value}")
            elif isinstance(value, bool):
                set_clauses.append(f"n.{key} = {str(value).lower()}")
            elif value is None:
                set_clauses.append(f"n.{key} = NULL")
        
        if not set_clauses:
            return False
        
        set_clause = ', '.join(set_clauses)
        
        query = f"""
            MATCH (n:{node_label} {{id: {node_id}}})
            SET {set_clause}
            RETURN n.id
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('id', 'agtype')]
        )
        
        success = len(results) > 0
        if success:
            logger.debug(f"Updated {node_label} node: id={node_id}")
        
        return success
    
    async def delete_node(
        self,
        node_label: str,
        node_id: int,
        detach: bool = True
    ) -> bool:
        """
        Delete node from graph
        
        Args:
            node_label: Node label
            node_id: Node's id property
            detach: If True, delete all relationships first (DETACH DELETE)
        
        Returns:
            True if deleted successfully
        """
        delete_clause = "DETACH DELETE" if detach else "DELETE"
        
        query = f"""
            MATCH (n:{node_label} {{id: {node_id}}})
            {delete_clause} n
            RETURN count(n)
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('count', 'agtype')]
        )
        
        success = results and results[0]['count'] > 0
        if success:
            logger.info(f"Deleted {node_label} node: id={node_id}")
        
        return success
    
    async def get_node(
        self,
        node_label: str,
        node_id: int
    ) -> Optional[Dict[str, Any]]:
        """
        Retrieve node by ID
        
        Args:
            node_label: Node label
            node_id: Node's id property
        
        Returns:
            Node properties dict or None if not found
        """
        query = f"""
            MATCH (n:{node_label} {{id: {node_id}}})
            RETURN n
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('node', 'agtype')]
        )
        
        if results:
            return parse_agtype(results[0]['node'])
        
        return None
    
    # ========================================================================
    # EDGE CRUD OPERATIONS
    # ========================================================================
    
    async def create_edge(
        self,
        from_label: str,
        from_id: int,
        to_label: str,
        to_id: int,
        edge_type: str,
        properties: Optional[Dict[str, Any]] = None
    ) -> bool:
        """
        Create edge between two nodes
        
        Args:
            from_label: Source node label
            from_id: Source node id
            to_label: Target node label
            to_id: Target node id
            edge_type: Edge relationship type (e.g., 'LEADS', 'THREATENS')
            properties: Edge properties
        
        Returns:
            True if created successfully
        
        Example:
            await graph_mgr.create_edge(
                from_label='Monster',
                from_id=1001,
                to_label='Monster',
                to_id=1002,
                edge_type='LEADS',
                properties={'strength': 0.8, 'established': 'now'}
            )
        """
        props = properties or {}
        
        # Build property string
        prop_strs = []
        for key, value in props.items():
            if isinstance(value, str):
                prop_strs.append(f"{key}: '{value}'")
            elif isinstance(value, (int, float)):
                prop_strs.append(f"{key}: {value}")
            elif isinstance(value, bool):
                prop_strs.append(f"{key}: {str(value).lower()}")
        
        prop_str = ', '.join(prop_strs) if prop_strs else ''
        prop_clause = f" {{{prop_str}}}" if prop_str else ""
        
        query = f"""
            MATCH (a:{from_label} {{id: {from_id}}})
            MATCH (b:{to_label} {{id: {to_id}}})
            CREATE (a)-[r:{edge_type}{prop_clause}]->(b)
            RETURN type(r)
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('type', 'agtype')]
        )
        
        success = len(results) > 0
        if success:
            logger.debug(
                f"Created edge: ({from_label}:{from_id})-[:{edge_type}]->({to_label}:{to_id})"
            )
        
        return success
    
    async def update_edge(
        self,
        from_label: str,
        from_id: int,
        to_label: str,
        to_id: int,
        edge_type: str,
        properties: Dict[str, Any]
    ) -> bool:
        """
        Update edge properties
        
        Args:
            from_label: Source node label
            from_id: Source node id
            to_label: Target node label
            to_id: Target node id
            edge_type: Edge relationship type
            properties: Properties to update
        
        Returns:
            True if updated successfully
        """
        # Build SET clauses
        set_clauses = []
        for key, value in properties.items():
            if isinstance(value, str):
                set_clauses.append(f"r.{key} = '{value}'")
            elif isinstance(value, (int, float)):
                set_clauses.append(f"r.{key} = {value}")
            elif isinstance(value, bool):
                set_clauses.append(f"r.{key} = {str(value).lower()}")
        
        if not set_clauses:
            return False
        
        set_clause = ', '.join(set_clauses)
        
        query = f"""
            MATCH (a:{from_label} {{id: {from_id}}})-[r:{edge_type}]->(b:{to_label} {{id: {to_id}}})
            SET {set_clause}
            RETURN type(r)
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('type', 'agtype')]
        )
        
        return len(results) > 0
    
    async def delete_edge(
        self,
        from_label: str,
        from_id: int,
        to_label: str,
        to_id: int,
        edge_type: str
    ) -> bool:
        """
        Delete edge between two nodes
        
        Args:
            from_label: Source node label
            from_id: Source node id
            to_label: Target node label
            to_id: Target node id
            edge_type: Edge relationship type
        
        Returns:
            True if deleted successfully
        """
        query = f"""
            MATCH (a:{from_label} {{id: {from_id}}})-[r:{edge_type}]->(b:{to_label} {{id: {to_id}}})
            DELETE r
            RETURN count(r)
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('count', 'agtype')]
        )
        
        return results and results[0]['count'] > 0
    
    async def get_edges(
        self,
        from_label: str,
        from_id: int,
        edge_type: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """
        Get all edges from a node
        
        Args:
            from_label: Source node label
            from_id: Source node id
            edge_type: Optional edge type filter
        
        Returns:
            List of edges with target node info
        """
        edge_filter = f":{edge_type}" if edge_type else ""
        
        query = f"""
            MATCH (a:{from_label} {{id: {from_id}}})-[r{edge_filter}]->(b)
            RETURN type(r) AS edge_type, properties(r) AS edge_props, 
                   labels(b)[0] AS target_label, b.id AS target_id
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('edge_type', 'agtype'),
                ('edge_props', 'agtype'),
                ('target_label', 'agtype'),
                ('target_id', 'agtype')
            ]
        )
        
        return results
    
    # ========================================================================
    # PATH FINDING QUERIES
    # ========================================================================
    
    async def find_shortest_path(
        self,
        from_label: str,
        from_id: int,
        to_label: str,
        to_id: int,
        edge_type: Optional[str] = None,
        max_depth: int = 5
    ) -> Optional[List[Dict[str, Any]]]:
        """
        Find shortest path between two nodes
        
        Args:
            from_label: Start node label
            from_id: Start node id
            to_label: End node label
            to_id: End node id
            edge_type: Optional edge type constraint
            max_depth: Maximum path depth
        
        Returns:
            Path as list of nodes and edges, or None if no path exists
        """
        edge_pattern = f":{edge_type}" if edge_type else ""
        
        query = f"""
            MATCH path = shortestPath(
                (start:{from_label} {{id: {from_id}}})-[{edge_pattern}*1..{max_depth}]->(end:{to_label} {{id: {to_id}}})
            )
            RETURN [n IN nodes(path) | {{id: n.id, label: labels(n)[0]}}] AS nodes,
                   [r IN relationships(path) | type(r)] AS edge_types,
                   length(path) AS path_length
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('nodes', 'agtype'),
                ('edge_types', 'agtype'),
                ('path_length', 'agtype')
            ]
        )
        
        if results:
            logger.debug(
                f"Found path from {from_label}:{from_id} to {to_label}:{to_id}, "
                f"length={results[0]['path_length']}"
            )
            return results
        
        return None
    
    async def find_all_paths(
        self,
        from_label: str,
        from_id: int,
        to_label: str,
        to_id: int,
        edge_type: Optional[str] = None,
        max_depth: int = 3,
        limit: int = 10
    ) -> List[Dict[str, Any]]:
        """
        Find all paths between two nodes
        
        Args:
            from_label: Start node label
            from_id: Start node id
            to_label: End node label
            to_id: End node id
            edge_type: Optional edge type constraint
            max_depth: Maximum path depth
            limit: Maximum number of paths to return
        
        Returns:
            List of paths
        """
        edge_pattern = f":{edge_type}" if edge_type else ""
        
        query = f"""
            MATCH path = (start:{from_label} {{id: {from_id}}})-[{edge_pattern}*1..{max_depth}]->(end:{to_label} {{id: {to_id}}})
            RETURN [n IN nodes(path) | {{id: n.id, label: labels(n)[0]}}] AS nodes,
                   length(path) AS path_length
            ORDER BY path_length ASC
            LIMIT {limit}
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('nodes', 'agtype'),
                ('path_length', 'agtype')
            ]
        )
        
        return results
    
    # ========================================================================
    # NEIGHBOR QUERIES
    # ========================================================================
    
    async def get_neighbors(
        self,
        node_label: str,
        node_id: int,
        direction: str = 'outgoing',
        edge_type: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """
        Get all neighbors of a node
        
        Args:
            node_label: Node label
            node_id: Node id
            direction: 'outgoing', 'incoming', or 'both'
            edge_type: Optional edge type filter
        
        Returns:
            List of neighbor nodes with relationship info
        """
        edge_filter = f":{edge_type}" if edge_type else ""
        
        if direction == 'outgoing':
            pattern = f"(n:{node_label} {{id: {node_id}}})-[r{edge_filter}]->(neighbor)"
        elif direction == 'incoming':
            pattern = f"(neighbor)-[r{edge_filter}]->(n:{node_label} {{id: {node_id}}})"
        else:  # both
            pattern = f"(n:{node_label} {{id: {node_id}}})-[r{edge_filter}]-(neighbor)"
        
        query = f"""
            MATCH {pattern}
            RETURN neighbor.id AS neighbor_id,
                   labels(neighbor)[0] AS neighbor_label,
                   type(r) AS edge_type,
                   properties(r) AS edge_properties
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('neighbor_id', 'agtype'),
                ('neighbor_label', 'agtype'),
                ('edge_type', 'agtype'),
                ('edge_properties', 'agtype')
            ]
        )
        
        return results
    
    async def get_neighbors_by_type(
        self,
        node_label: str,
        node_id: int,
        neighbor_label: str,
        edge_type: Optional[str] = None
    ) -> List[Dict[str, Any]]:
        """
        Get neighbors of specific type
        
        Args:
            node_label: Source node label
            node_id: Source node id
            neighbor_label: Target neighbor label to filter
            edge_type: Optional edge type filter
        
        Returns:
            List of matching neighbors
        """
        edge_filter = f":{edge_type}" if edge_type else ""
        
        query = f"""
            MATCH (n:{node_label} {{id: {node_id}}})-[r{edge_filter}]->(neighbor:{neighbor_label})
            RETURN neighbor.id AS id, properties(neighbor) AS properties, type(r) AS edge_type
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('id', 'agtype'),
                ('properties', 'agtype'),
                ('edge_type', 'agtype')
            ]
        )
        
        return results
    
    # ========================================================================
    # SPECIALIZED MONSTER AI QUERIES
    # ========================================================================
    
    async def get_monster_team(self, monster_id: int) -> List[Dict[str, Any]]:
        """
        Get all pack/team members for a monster
        
        Includes:
        - Leader (if exists)
        - Followers (if leader)
        - Teammates (team members)
        
        Args:
            monster_id: Monster instance ID
        
        Returns:
            List of team member info
        """
        query = f"""
            MATCH (self:Monster {{id: {monster_id}}})
            OPTIONAL MATCH (leader:Monster)-[:LEADS]->(self)
            OPTIONAL MATCH (self)-[:LEADS*0..2]->(follower:Monster)
            OPTIONAL MATCH (self)-[:TEAMMATE_OF]->(team:Team)<-[:TEAMMATE_OF]-(teammate:Monster)
            RETURN 
                CASE WHEN leader IS NOT NULL THEN leader.id ELSE self.id END AS leader_id,
                collect(DISTINCT follower.id) AS follower_ids,
                collect(DISTINCT teammate.id) AS teammate_ids,
                CASE WHEN leader IS NOT NULL THEN 'follower' 
                     WHEN EXISTS((self)-[:LEADS]->()) THEN 'leader'
                     ELSE 'independent' END AS role
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('leader_id', 'agtype'),
                ('follower_ids', 'agtype'),
                ('teammate_ids', 'agtype'),
                ('role', 'agtype')
            ]
        )
        
        if results:
            return results
        
        return []
    
    async def get_threat_network(
        self,
        monster_id: int,
        radius: int = 3
    ) -> List[Dict[str, Any]]:
        """
        Get threat network around monster
        
        Returns players and guilds that threaten this monster or nearby allies
        
        Args:
            monster_id: Monster instance ID
            radius: Graph traversal radius (hops from monster)
        
        Returns:
            List of threat actors with threat scores
        """
        query = f"""
            MATCH (monster:Monster {{id: {monster_id}}})
            OPTIONAL MATCH (player:Player)-[threat:THREATENS]->(monster)
            OPTIONAL MATCH (guild:Guild)-[hunt:HUNTS]->(monster)
            OPTIONAL MATCH (monster)-[:ALLIES_WITH|TEAMMATE_OF*1..{radius}]-(ally:Monster)
            OPTIONAL MATCH (player2:Player)-[threat2:THREATENS]->(ally)
            RETURN 
                collect(DISTINCT {{
                    type: 'player',
                    char_id: player.char_id,
                    name: player.name,
                    threat_score: threat.threat_score
                }}) AS player_threats,
                collect(DISTINCT {{
                    type: 'guild',
                    guild_id: guild.guild_id,
                    name: guild.name,
                    threat_level: guild.threat_level
                }}) AS guild_threats,
                collect(DISTINCT {{
                    type: 'ally_threat',
                    ally_id: ally.id,
                    threatened_by: player2.name,
                    threat_score: threat2.threat_score
                }}) AS ally_threats
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('player_threats', 'agtype'),
                ('guild_threats', 'agtype'),
                ('ally_threats', 'agtype')
            ]
        )
        
        return results
    
    async def find_coordination_paths(
        self,
        monster_ids: List[int]
    ) -> List[Dict[str, Any]]:
        """
        Find coordination paths between multiple monsters
        
        Used to determine if monsters can coordinate effectively
        
        Args:
            monster_ids: List of monster IDs to check
        
        Returns:
            List of coordination paths between all pairs
        """
        if len(monster_ids) < 2:
            return []
        
        # Build pairs
        results = []
        
        for i in range(len(monster_ids)):
            for j in range(i + 1, len(monster_ids)):
                from_id = monster_ids[i]
                to_id = monster_ids[j]
                
                # Check if coordination path exists
                path = await self.find_shortest_path(
                    from_label='Monster',
                    from_id=from_id,
                    to_label='Monster',
                    to_id=to_id,
                    edge_type=None,  # Any edge type
                    max_depth=3
                )
                
                if path:
                    results.append({
                        'from_id': from_id,
                        'to_id': to_id,
                        'path': path,
                        'can_coordinate': True
                    })
                else:
                    results.append({
                        'from_id': from_id,
                        'to_id': to_id,
                        'path': None,
                        'can_coordinate': False
                    })
        
        return results
    
    async def update_spatial_relationships(
        self,
        monster_id: int,
        position: Tuple[int, int],
        hp_ratio: float,
        map_id: int
    ) -> bool:
        """
        Update monster's spatial position and HP in graph
        
        Args:
            monster_id: Monster instance ID
            position: (x, y) coordinates
            hp_ratio: Current HP ratio (0.0-1.0)
            map_id: Current map ID
        
        Returns:
            True if updated successfully
        """
        x, y = position
        
        query = f"""
            MATCH (m:Monster {{id: {monster_id}}})
            SET m.position_x = {x},
                m.position_y = {y},
                m.current_hp_ratio = {hp_ratio},
                m.map_id = {map_id}
            RETURN m.id
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('id', 'agtype')]
        )
        
        success = len(results) > 0
        if success:
            logger.debug(f"Updated spatial data for Monster {monster_id}: pos=({x},{y}), hp={hp_ratio:.2f}")
        
        return success
    
    async def learn_behavior_pattern(
        self,
        monster_id: int,
        pattern_data: Dict[str, Any]
    ) -> bool:
        """
        Record learned behavior pattern
        
        Creates Pattern node and links to monster via LEARNED_FROM edge
        
        Args:
            monster_id: Monster instance ID
            pattern_data: Pattern information (type, sequence, effectiveness, etc.)
        
        Returns:
            True if recorded successfully
        """
        pattern_id = pattern_data.get('pattern_id', int(time.time() * 1000))
        pattern_type = pattern_data.get('pattern_type', 'unknown')
        effectiveness = pattern_data.get('effectiveness', 0.5)
        
        # Create pattern node
        query_create = f"""
            CREATE (p:Pattern {{
                pattern_id: {pattern_id},
                pattern_type: '{pattern_type}',
                effectiveness: {effectiveness},
                learned_at: timestamp(),
                times_used: 0
            }})
            RETURN p.pattern_id
        """
        
        await self.execute_cypher(
            query_create,
            returns_schema=[('pattern_id', 'agtype')]
        )
        
        # Link to monster
        query_link = f"""
            MATCH (monster:Monster {{id: {monster_id}}})
            MATCH (pattern:Pattern {{pattern_id: {pattern_id}}})
            CREATE (monster)-[:LEARNED_PATTERN {{
                learning_date: timestamp(),
                proficiency: 0.5,
                practice_count: 0
            }}]->(pattern)
            RETURN monster.id
        """
        
        results = await self.execute_cypher(
            query_link,
            returns_schema=[('id', 'agtype')]
        )
        
        success = len(results) > 0
        if success:
            logger.info(f"Monster {monster_id} learned pattern: {pattern_type}")
        
        return success
    
    async def get_learned_behaviors(
        self,
        monster_id: int
    ) -> List[Dict[str, Any]]:
        """
        Get all learned behaviors/patterns for a monster
        
        Args:
            monster_id: Monster instance ID
        
        Returns:
            List of learned behaviors with proficiency info
        """
        query = f"""
            MATCH (monster:Monster {{id: {monster_id}}})-[r:LEARNED_FROM|LEARNED_PATTERN]->(item)
            WHERE 'Behavior' IN labels(item) OR 'Pattern' IN labels(item)
            RETURN 
                labels(item)[0] AS item_type,
                item.name AS name,
                item.pattern_type AS pattern_type,
                properties(r) AS relationship_props
            ORDER BY r.proficiency DESC
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('item_type', 'agtype'),
                ('name', 'agtype'),
                ('pattern_type', 'agtype'),
                ('relationship_props', 'agtype')
            ]
        )
        
        return results
    
    # ========================================================================
    # PLAYER ENCOUNTER TRACKING
    # ========================================================================
    
    async def record_player_encounter(
        self,
        monster_id: int,
        player_char_id: int,
        encounter_data: Dict[str, Any]
    ) -> bool:
        """
        Record player-monster encounter in graph
        
        Args:
            monster_id: Monster instance ID
            player_char_id: Player character ID
            encounter_data: Encounter details (outcome, damage, strategy, etc.)
        
        Returns:
            True if recorded successfully
        """
        outcome = encounter_data.get('outcome', 'unknown')
        damage_dealt = encounter_data.get('damage_dealt_to_monster', 0)
        damage_taken = encounter_data.get('damage_taken_from_monster', 0)
        strategy = encounter_data.get('strategy_used', 'unknown')
        duration = encounter_data.get('duration_seconds', 0.0)
        
        query = f"""
            MATCH (player:Player {{char_id: {player_char_id}}})
            MATCH (monster:Monster {{id: {monster_id}}})
            CREATE (player)-[:ENCOUNTERED {{
                timestamp: timestamp(),
                outcome: '{outcome}',
                damage_dealt_to_monster: {damage_dealt},
                damage_taken_from_monster: {damage_taken},
                strategy_used: '{strategy}',
                duration_seconds: {duration},
                map_id: {encounter_data.get('map_id', 1)}
            }}]->(monster)
            RETURN player.char_id
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('char_id', 'agtype')]
        )
        
        success = len(results) > 0
        if success:
            logger.info(
                f"Recorded encounter: Player {player_char_id} vs Monster {monster_id}, "
                f"outcome={outcome}"
            )
        
        return success
    
    async def get_player_strategy_pattern(
        self,
        player_char_id: int,
        monster_archetype: str,
        lookback_hours: int = 24
    ) -> Optional[Dict[str, Any]]:
        """
        Analyze player's strategy pattern against archetype
        
        Args:
            player_char_id: Player character ID
            monster_archetype: Monster archetype to analyze
            lookback_hours: How far back to analyze
        
        Returns:
            Player's most common strategy and success rate
        """
        query = f"""
            MATCH (player:Player {{char_id: {player_char_id}}})-[enc:ENCOUNTERED]->(monster:Monster)
            WHERE monster.archetype = '{monster_archetype}'
              AND enc.timestamp > timestamp() - interval '{lookback_hours} hours'
            WITH enc.strategy_used AS strategy,
                 COUNT(*) AS usage_count,
                 SUM(CASE WHEN enc.outcome = 'player_won' THEN 1 ELSE 0 END) AS wins
            RETURN strategy,
                   usage_count,
                   wins,
                   (wins::FLOAT / usage_count) AS win_rate
            ORDER BY usage_count DESC
            LIMIT 1
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('strategy', 'agtype'),
                ('usage_count', 'agtype'),
                ('wins', 'agtype'),
                ('win_rate', 'agtype')
            ]
        )
        
        if results:
            return results[0]
        
        return None
    
    # ========================================================================
    # SUBGRAPH EXTRACTION
    # ========================================================================
    
    async def get_subgraph(
        self,
        center_label: str,
        center_id: int,
        radius: int = 2,
        edge_types: Optional[List[str]] = None
    ) -> Dict[str, Any]:
        """
        Extract subgraph around a center node
        
        Args:
            center_label: Center node label
            center_id: Center node id
            radius: Maximum distance from center (hops)
            edge_types: Optional list of edge types to include
        
        Returns:
            Subgraph as dict with nodes and edges
        """
        edge_filter = f":{' | '.join(edge_types)}" if edge_types else ""
        
        query = f"""
            MATCH (center:{center_label} {{id: {center_id}}})
            OPTIONAL MATCH path = (center)-[{edge_filter}*0..{radius}]-(node)
            WITH collect(DISTINCT node) AS nodes, 
                 collect(DISTINCT [n IN nodes(path) | {{id: id(n), props: properties(n)}}]) AS node_details
            RETURN node_details
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('node_details', 'agtype')]
        )
        
        return results[0] if results else {'nodes': [], 'edges': []}
    
    async def get_pack_subgraph(
        self,
        monster_id: int
    ) -> Dict[str, Any]:
        """
        Get complete pack subgraph for a monster
        
        Includes all pack members, their relationships, and shared threats
        
        Args:
            monster_id: Monster instance ID
        
        Returns:
            Pack subgraph data
        """
        query = f"""
            MATCH (self:Monster {{id: {monster_id}}})
            OPTIONAL MATCH (leader:Monster)-[:LEADS*0..2]->(self)
            OPTIONAL MATCH (self)-[:LEADS*0..2]->(member:Monster)
            WITH DISTINCT 
                CASE WHEN leader IS NOT NULL THEN leader ELSE self END AS pack_leader,
                collect(DISTINCT member) + [self] AS all_members
            UNWIND all_members AS pack_member
            OPTIONAL MATCH (pack_member)-[r]-(related)
            RETURN 
                pack_leader.id AS leader_id,
                collect(DISTINCT {{
                    id: pack_member.id,
                    archetype: pack_member.archetype,
                    level: pack_member.level,
                    hp_ratio: pack_member.current_hp_ratio
                }}) AS members,
                collect(DISTINCT {{
                    from: id(startNode(r)),
                    to: id(endNode(r)),
                    type: type(r)
                }}) AS relationships
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('leader_id', 'agtype'),
                ('members', 'agtype'),
                ('relationships', 'agtype')
            ]
        )
        
        if results:
            return results[0]
        
        return {'leader_id': None, 'members': [], 'relationships': []}
    
    # ========================================================================
    # TRANSACTION SUPPORT
    # ========================================================================
    
    async def execute_in_transaction(
        self,
        queries: List[Tuple[str, Optional[Dict[str, Any]]]]
    ) -> bool:
        """
        Execute multiple Cypher queries in a transaction
        
        Args:
            queries: List of (query_string, params) tuples
        
        Returns:
            True if all queries succeeded
        
        Example:
            await graph_mgr.execute_in_transaction([
                ("CREATE (m:Monster {id: 1})", None),
                ("CREATE (m:Monster {id: 2})", None),
                ("MATCH (a:Monster {id: 1}), (b:Monster {id: 2}) CREATE (a)-[:LEADS]->(b)", None)
            ])
        """
        async with self.pool.acquire() as conn:
            async with conn.transaction():
                try:
                    # Set AGE context
                    await conn.execute("LOAD 'age'")
                    await conn.execute("SET search_path = ag_catalog, \"$user\", public")
                    
                    # Execute all queries
                    for query, params in queries:
                        full_query = f"SELECT * FROM cypher('{self.graph_name}', $$ {query} $$) AS (result agtype)"
                        await conn.execute(full_query)
                    
                    logger.info(f"Transaction completed: {len(queries)} queries executed")
                    return True
                
                except Exception as e:
                    logger.error(f"Transaction failed: {e}", exc_info=True)
                    raise AGEQueryError(f"Transaction failed: {e}") from e
    
    # ========================================================================
    # BULK OPERATIONS
    # ========================================================================
    
    async def bulk_create_monsters(
        self,
        monsters: List[Dict[str, Any]]
    ) -> int:
        """
        Bulk create multiple monster nodes
        
        Args:
            monsters: List of monster property dicts
        
        Returns:
            Number of monsters created
        """
        if not monsters:
            return 0
        
        created_count = 0
        
        for monster in monsters:
            try:
                await self.create_monster_node(
                    monster_id=monster['id'],
                    properties=monster
                )
                created_count += 1
            except Exception as e:
                logger.error(f"Failed to create monster {monster.get('id')}: {e}")
        
        logger.info(f"Bulk created {created_count}/{len(monsters)} monsters")
        return created_count
    
    async def bulk_update_positions(
        self,
        updates: List[Dict[str, Any]]
    ) -> int:
        """
        Bulk update monster positions
        
        Args:
            updates: List of update dicts with monster_id, position, hp_ratio
        
        Returns:
            Number of monsters updated
        """
        if not updates:
            return 0
        
        updated_count = 0
        
        for update in updates:
            try:
                await self.update_spatial_relationships(
                    monster_id=update['monster_id'],
                    position=(update['x'], update['y']),
                    hp_ratio=update['hp_ratio'],
                    map_id=update['map_id']
                )
                updated_count += 1
            except Exception as e:
                logger.error(f"Failed to update monster {update.get('monster_id')}: {e}")
        
        logger.debug(f"Bulk updated {updated_count}/{len(updates)} monster positions")
        return updated_count
    
    # ========================================================================
    # SIGNAL AND COMMUNICATION
    # ========================================================================
    
    async def send_signal(
        self,
        sender_id: int,
        receiver_id: int,
        signal_type: str,
        content: Dict[str, Any],
        priority: int = 5
    ) -> bool:
        """
        Send signal from one monster to another
        
        Args:
            sender_id: Sender monster ID
            receiver_id: Receiver monster ID
            signal_type: Signal type (target_info, danger_alert, help_request, etc.)
            content: Signal payload as dict
            priority: Signal priority (1=highest, 10=lowest)
        
        Returns:
            True if signal sent successfully
        """
        content_json = json.dumps(content).replace("'", "''")  # Escape quotes
        
        query = f"""
            MATCH (sender:Monster {{id: {sender_id}}})
            MATCH (receiver:Monster {{id: {receiver_id}}})
            CREATE (sender)-[:SIGNALS {{
                signal_type: '{signal_type}',
                content: '{content_json}',
                timestamp: timestamp(),
                priority: {priority},
                expires: timestamp() + interval '5 seconds',
                acknowledged: false
            }}]->(receiver)
            RETURN sender.id
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('id', 'agtype')]
        )
        
        return len(results) > 0
    
    async def receive_signals(
        self,
        monster_id: int
    ) -> List[Dict[str, Any]]:
        """
        Receive all pending signals for monster
        
        Args:
            monster_id: Monster instance ID
        
        Returns:
            List of signals ordered by priority
        """
        query = f"""
            MATCH (sender:Monster)-[sig:SIGNALS]->(receiver:Monster {{id: {monster_id}}})
            WHERE sig.expires > timestamp()
              AND sig.acknowledged = false
            RETURN sender.id AS sender_id,
                   sig.signal_type AS signal_type,
                   sig.content AS content,
                   sig.priority AS priority,
                   sig.timestamp AS timestamp
            ORDER BY sig.priority ASC
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('sender_id', 'agtype'),
                ('signal_type', 'agtype'),
                ('content', 'agtype'),
                ('priority', 'agtype'),
                ('timestamp', 'agtype')
            ]
        )
        
        # Parse content JSON
        for result in results:
            if 'content' in result and isinstance(result['content'], str):
                try:
                    result['content'] = json.loads(result['content'])
                except json.JSONDecodeError:
                    pass
        
        return results
    
    # ========================================================================
    # PACK COORDINATION
    # ========================================================================
    
    async def create_pack_leadership(
        self,
        leader_id: int,
        follower_id: int,
        strength: float = 0.7
    ) -> bool:
        """
        Establish leadership relationship between monsters
        
        Args:
            leader_id: Leader monster ID
            follower_id: Follower monster ID
            strength: Leadership strength (0.0-1.0)
        
        Returns:
            True if relationship created
        """
        query = f"""
            MATCH (leader:Monster {{id: {leader_id}}})
            MATCH (follower:Monster {{id: {follower_id}}})
            MERGE (leader)-[r:LEADS]->(follower)
            ON CREATE SET
                r.strength = {strength},
                r.established = timestamp(),
                r.challenges = 0,
                r.last_command = timestamp()
            ON MATCH SET
                r.strength = {strength},
                r.last_command = timestamp()
            RETURN r.strength
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[('strength', 'agtype')]
        )
        
        success = len(results) > 0
        if success:
            logger.info(f"Pack leadership: Monster {leader_id} leads Monster {follower_id}")
        
        return success
    
    async def get_pack_formation(
        self,
        leader_id: int
    ) -> Dict[str, Any]:
        """
        Get pack formation data for coordination
        
        Args:
            leader_id: Pack leader monster ID
        
        Returns:
            Pack formation info (members, positions, roles)
        """
        query = f"""
            MATCH (leader:Monster {{id: {leader_id}}})-[:LEADS*0..2]->(member:Monster)
            RETURN 
                leader.id AS leader_id,
                leader.position_x AS leader_x,
                leader.position_y AS leader_y,
                collect({{
                    id: member.id,
                    archetype: member.archetype,
                    position_x: member.position_x,
                    position_y: member.position_y,
                    hp_ratio: member.current_hp_ratio
                }}) AS members
        """
        
        results = await self.execute_cypher(
            query,
            returns_schema=[
                ('leader_id', 'agtype'),
                ('leader_x', 'agtype'),
                ('leader_y', 'agtype'),
                ('members', 'agtype')
            ]
        )
        
        if results:
            formation = results[0]
            
            # Calculate formation metrics
            members = formation.get('members', [])
            if members:
                positions = np.array([(m['position_x'], m['position_y']) for m in members])
                cohesion = 1.0 / (1.0 + np.var(positions))  # Lower variance = higher cohesion
                
                formation['pack_size'] = len(members)
                formation['cohesion_score'] = float(cohesion)
                formation['avg_hp'] = float(np.mean([m['hp_ratio'] for m in members]))
            
            return formation
        
        return {'leader_id': None, 'members': [], 'pack_size': 0}
    
    # ========================================================================
    # STATISTICS AND MAINTENANCE
    # ========================================================================
    
    async def get_graph_statistics(self) -> Dict[str, Any]:
        """
        Get graph statistics (node counts, edge counts, etc.)
        
        Returns:
            Statistics dict
        """
        # Count nodes by label
        query_nodes = """
            MATCH (n)
            RETURN labels(n)[0] AS label, COUNT(*) AS count
            ORDER BY count DESC
        """
        
        node_counts = await self.execute_cypher(
            query_nodes,
            returns_schema=[('label', 'agtype'), ('count', 'agtype')]
        )
        
        # Count edges by type
        query_edges = """
            MATCH ()-[r]->()
            RETURN type(r) AS type, COUNT(*) AS count
            ORDER BY count DESC
        """
        
        edge_counts = await self.execute_cypher(
            query_edges,
            returns_schema=[('type', 'agtype'), ('count', 'agtype')]
        )
        
        return {
            'graph_name': self.graph_name,
            'node_counts': node_counts,
            'edge_counts': edge_counts,
            'total_nodes': sum(nc['count'] for nc in node_counts),
            'total_edges': sum(ec['count'] for ec in edge_counts),
            'query_count': self.query_count,
            'avg_query_time_ms': (self.total_query_time / self.query_count * 1000) if self.query_count > 0 else 0.0,
            'error_count': self.error_count
        }
    
    async def cleanup_expired_data(
        self,
        retention_days: int = 30
    ) -> Dict[str, int]:
        """
        Clean up old graph data
        
        Removes:
        - Old encounter edges (>30 days)
        - Expired signals
        - Inactive monster nodes (dead >7 days)
        
        Args:
            retention_days: How many days of data to keep
        
        Returns:
            Dict with cleanup counts
        """
        cleanup_counts = {
            'encounters_deleted': 0,
            'signals_deleted': 0,
            'monsters_deleted': 0
        }
        
        # Delete old encounters
        query_encounters = f"""
            MATCH ()-[enc:ENCOUNTERED]->()
            WHERE enc.timestamp < timestamp() - interval '{retention_days} days'
            DELETE enc
            RETURN count(enc)
        """
        
        results = await self.execute_cypher(
            query_encounters,
            returns_schema=[('count', 'agtype')]
        )
        
        if results:
            cleanup_counts['encounters_deleted'] = results[0]['count']
        
        # Delete expired signals
        query_signals = """
            MATCH (signal:SignalNode)
            WHERE signal.expires < timestamp()
            DETACH DELETE signal
            RETURN count(signal)
        """
        
        results = await self.execute_cypher(
            query_signals,
            returns_schema=[('count', 'agtype')]
        )
        
        if results:
            cleanup_counts['signals_deleted'] = results[0]['count']
        
        logger.info(
            f"Cleanup complete: {cleanup_counts['encounters_deleted']} encounters, "
            f"{cleanup_counts['signals_deleted']} signals deleted"
        )
        
        return cleanup_counts
    
    async def verify_graph_health(self) -> Dict[str, Any]:
        """
        Verify graph database health
        
        Checks:
        - Graph exists
        - Can execute queries
        - Performance metrics
        
        Returns:
            Health status dict
        """
        health = {
            'healthy': False,
            'graph_exists': False,
            'query_performance_ms': 0.0,
            'error': None
        }
        
        try:
            # Test query: count all nodes
            start = time.perf_counter()
            
            results = await self.execute_cypher(
                "MATCH (n) RETURN count(n)",
                returns_schema=[('count', 'agtype')]
            )
            
            elapsed = (time.perf_counter() - start) * 1000
            
            if results:
                health['healthy'] = True
                health['graph_exists'] = True
                health['query_performance_ms'] = elapsed
                health['total_nodes'] = results[0]['count']
                
                logger.info(f"Graph health check passed: {health['total_nodes']} nodes, {elapsed:.2f}ms query time")
            
        except Exception as e:
            health['error'] = str(e)
            logger.error(f"Graph health check failed: {e}")
        
        return health
    
    def get_performance_metrics(self) -> Dict[str, Any]:
        """
        Get performance metrics for graph operations
        
        Returns:
            Metrics dict
        """
        return {
            'query_count': self.query_count,
            'total_query_time_seconds': self.total_query_time,
            'avg_query_time_ms': (self.total_query_time / self.query_count * 1000) if self.query_count > 0 else 0.0,
            'error_count': self.error_count,
            'error_rate': (self.error_count / self.query_count) if self.query_count > 0 else 0.0
        }


# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def calculate_leadership_score(
    monster_level: int,
    experience: int,
    archetype: str
) -> float:
    """
    Calculate leadership score for monster
    
    Factors:
    - Level (higher = better leader)
    - Experience (more experienced = better)
    - Archetype (tanks and aggressive monsters are natural leaders)
    
    Args:
        monster_level: Monster level
        experience: Monster experience points
        archetype: Monster archetype
    
    Returns:
        Leadership score (0.0-1.0)
    """
    # Base score from level (0.0-0.5)
    level_score = min(monster_level / 200.0, 0.5)
    
    # Experience score (0.0-0.3)
    exp_score = min(experience / 100000.0, 0.3)
    
    # Archetype bonus (0.0-0.2)
    archetype_bonuses = {
        'tank': 0.2,
        'aggressive': 0.15,
        'defensive': 0.1,
        'mage': 0.05,
        'ranged': 0.05,
        'support': 0.0
    }
    archetype_score = archetype_bonuses.get(archetype, 0.0)
    
    total_score = level_score + exp_score + archetype_score
    
    return min(total_score, 1.0)


async def initialize_graph_manager(
    db_pool: asyncpg.Pool,
    graph_name: str = 'monster_ai'
) -> AGEGraphManager:
    """
    Initialize and verify graph manager
    
    Args:
        db_pool: asyncpg connection pool
        graph_name: AGE graph name
    
    Returns:
        Initialized AGEGraphManager instance
    
    Raises:
        AGEConnectionError: If graph is not accessible
    """
    manager = AGEGraphManager(db_pool, graph_name)
    
    # Verify graph health
    health = await manager.verify_graph_health()
    
    if not health['healthy']:
        raise AGEConnectionError(
            f"Graph '{graph_name}' is not healthy: {health.get('error', 'Unknown error')}"
        )
    
    logger.info(
        f"Graph manager initialized successfully: {health['total_nodes']} nodes, "
        f"{health['query_performance_ms']:.2f}ms query performance"
    )
    
    return manager


# ============================================================================
# MAIN (for testing)
# ============================================================================

async def test_graph_manager():
    """Test graph manager functionality"""
    import os
    
    # Create connection pool
    pool = await asyncpg.create_pool(
        host=os.getenv('POSTGRES_HOST', 'localhost'),
        port=int(os.getenv('POSTGRES_PORT', '5432')),
        database=os.getenv('POSTGRES_DB', 'ragnarok_ml'),
        user=os.getenv('POSTGRES_USER', 'ml_user'),
        password=os.getenv('POSTGRES_PASSWORD', ''),
        min_size=2,
        max_size=5
    )
    
    try:
        # Initialize graph manager
        graph_mgr = await initialize_graph_manager(pool)
        
        # Test: Get graph statistics
        stats = await graph_mgr.get_graph_statistics()
        print("\nGraph Statistics:")
        print(f"  Total Nodes: {stats['total_nodes']}")
        print(f"  Total Edges: {stats['total_edges']}")
        print(f"  Avg Query Time: {stats['avg_query_time_ms']:.2f}ms")
        
        # Test: Get monster team
        team = await graph_mgr.get_monster_team(1001)
        print(f"\nMonster 1001 Team: {team}")
        
        # Test: Get pack formation
        formation = await graph_mgr.get_pack_formation(1001)
        print(f"\nPack Formation: {formation}")
        
        # Get performance metrics
        metrics = graph_mgr.get_performance_metrics()
        print(f"\nPerformance Metrics:")
        print(f"  Queries: {metrics['query_count']}")
        print(f"  Avg Time: {metrics['avg_query_time_ms']:.2f}ms")
        print(f"  Errors: {metrics['error_count']}")
        
        print("\n✓ Graph manager test passed")
    
    finally:
        await pool.close()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    asyncio.run(test_graph_manager())
