# Apache AGE Graph Integration for ML Monster AI

## Table of Contents

1. [Overview](#overview)
2. [Apache AGE Setup Requirements](#apache-age-setup-requirements)
3. [Graph Schema Descriptions](#graph-schema-descriptions)
4. [Python API Usage](#python-api-usage)
5. [Common Query Patterns](#common-query-patterns)
6. [Performance Tuning](#performance-tuning)
7. [Troubleshooting](#troubleshooting)
8. [References](#references)

---

## Overview

Apache AGE (A Graph Extension) provides graph database capabilities within PostgreSQL, enabling sophisticated relationship modeling for the ML Monster AI system. This integration supports:

- **9 Graph Schemas**: Comprehensive relationship modeling
- **Multi-Agent Coordination**: Pack tactics and communication
- **Player Pattern Learning**: Track player strategies and adapt
- **Territorial Control**: Monitor and defend territories
- **Skill Dependencies**: Model skill combos and counters
- **Memory Networks**: Share knowledge between monsters

**Architecture Reference**: [`plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md`](../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md)

**Performance Target**: <5ms for graph queries, <10ms total including inference integration

---

## Apache AGE Setup Requirements

### Prerequisites

- **PostgreSQL**: 17.0 or higher (tested on 17.7)
- **Apache AGE**: 1.6.0 or higher
- **Python**: 3.11+
- **Operating System**: Ubuntu 20.04+ or RHEL 8+

### Installation

#### 1. Install PostgreSQL 17

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install postgresql-17 postgresql-server-dev-17

# Verify installation
psql --version
```

#### 2. Install Build Dependencies for Apache AGE

```bash
sudo apt-get install build-essential flex bison
```

#### 3. Clone and Build Apache AGE

```bash
# Clone repository
git clone https://github.com/apache/age.git
cd age

# Build and install
make PG_CONFIG=/usr/bin/pg_config
sudo make PG_CONFIG=/usr/bin/pg_config install
```

#### 4. Configure PostgreSQL

Edit `/etc/postgresql/17/main/postgresql.conf`:

```conf
# Add AGE to shared libraries
shared_preload_libraries = 'age'
```

Restart PostgreSQL:

```bash
sudo systemctl restart postgresql
```

#### 5. Create Database and Load Extension

```bash
# Connect to PostgreSQL
psql -U postgres

# Create database
CREATE DATABASE ragnarok_ml;
\c ragnarok_ml

# Load AGE extension
CREATE EXTENSION IF NOT EXISTS age;

# Verify
SELECT * FROM pg_extension WHERE extname = 'age';
```

#### 6. Run Setup Script

```bash
cd rathena-AI-world/ml_inference_service

# Set environment variables
export POSTGRES_HOST=localhost
export POSTGRES_PORT=5432
export POSTGRES_DB=ragnarok_ml
export POSTGRES_USER=ml_user
export POSTGRES_PASSWORD=your_password

# Run deployment script
./deploy_age.sh
```

Expected output:
```
✓ PostgreSQL connection successful
✓ Apache AGE extension is available
✓ SQL execution completed successfully
✓ Graph 'monster_ai' exists
✓ Nodes created: X total
✓ DEPLOYMENT SUCCESSFUL
```

---

## Graph Schema Descriptions

### Schema 1: Core Entity Schema (Pack Hierarchies)

**Purpose**: Model leader-follower relationships, pack structure, mentoring, and protection duties.

**Node Types**:
- `Monster`: Individual monster instances
  - Properties: `id`, `mob_id`, `name`, `level`, `archetype`, `leadership_score`, `experience`, `spawn_time`, `current_hp_ratio`, `position_x`, `position_y`, `map_id`
- `Team`: Organized packs
  - Properties: `team_id`, `name`, `formation`, `cohesion_score`, `created`

**Edge Types**:
- `LEADS`: Leader commands follower
  - Properties: `strength`, `established`, `challenges`, `last_command`
- `TEAMMATE_OF`: Monster belongs to team
  - Properties: `joined`, `role`, `contribution_score`
- `MENTORS`: Experienced monster teaches newer one
  - Properties: `skill_transfer`, `progress`, `sessions`
- `PROTECTS`: Tank protects vulnerable ally
  - Properties: `priority`, `distance_maintained`, `interventions`
- `ALLIES_WITH`: Inter-pack alliance
  - Properties: `alliance_type`, `formed`, `trust_level`, `shared_kills`

**Use Cases**:
- Determine pack hierarchy before coordinated attack
- Find who to protect when ally HP is low
- Identify learning opportunities from experienced monsters

### Schema 2: Spatial Relationship Schema (Territorial Control)

**Purpose**: Model territories, zones, patrol routes, and spatial navigation.

**Node Types**:
- `Zone`: Map regions
  - Properties: `zone_id`, `map_id`, `name`, `center_x`, `center_y`, `radius`, `danger_level`, `spawn_density`
- `Landmark`: Navigation landmarks
  - Properties: `landmark_id`, `map_id`, `name`, `x`, `y`, `landmark_type`, `visibility`
- `Territory`: Claimed territories
  - Properties: `territory_id`, `map_id`, `center_x`, `center_y`, `radius`, `resource_value`, `danger_level`, `owner_count`
- `PatrolPoint`: Waypoints in patrol routes
  - Properties: `point_id`, `map_id`, `x`, `y`, `importance`, `visit_frequency`

**Edge Types**:
- `WITHIN_ZONE`: Monster is within zone
- `NEAR`: Proximity to landmark
- `PATH_TO`: Navigation path between zones
- `CLAIMS_TERRITORY`: Monster claims territory
- `NEXT_POINT`: Sequential patrol route

**Use Cases**:
- Plan patrol routes
- Detect territory intrusion
- Navigate to strategic positions

### Schema 3: Threat Network Schema (Player Encounters)

**Purpose**: Track player-monster encounters, threat assessment, and combat history.

**Node Types**:
- `Player` (also `ThreatActor`): Player characters
  - Properties: `char_id`, `account_id`, `name`, `job`, `level`, `skill_pattern`, `aggression`, `skill_level`, `threat_rating`, `last_seen`
- `Guild`: Player guilds
  - Properties: `guild_id`, `name`, `avg_level`, `threat_level`, `territory`, `member_count`
- `ThreatEvent`: Combat events
  - Properties: `event_id`, `event_type`, `timestamp`, `severity`, `map_id`, `location_x`, `location_y`

**Edge Types**:
- `ENCOUNTERED`: Player fought monster
  - Properties: `timestamp`, `outcome`, `damage_dealt_to_monster`, `damage_taken_from_monster`, `strategy_used`, `duration_seconds`, `skills_used`, `map_id`
- `THREATENS`: Player is a threat to monster
- `ATTACKED_BY`: Monster was attacked
- `AGGRO_LINK`: Current aggro state
- `MEMBER_OF`: Player belongs to guild

**Use Cases**:
- Predict player strategy based on history
- Assess player/guild threat levels
- Adapt tactics to counter known player patterns

### Schema 4: Resource Competition Schema

**Purpose**: Model competition for resources (spawns, loot, tactical positions).

**Node Types**:
- `Resource`: Contested resources
  - Properties: `resource_id`, `resource_type`, `map_id`, `x`, `y`, `value`, `capacity`, `regeneration_time`

**Edge Types**:
- `COMPETES_FOR`: Monster competes for resource
  - Properties: `priority`, `times_contested`, `times_won`, `times_lost`, `last_attempt`
- `CONTROLS`: Monster controls resource
- `GUARDS`: Monster guards territory

**Use Cases**:
- Find uncontested resources
- Resolve resource conflicts
- Prioritize valuable resources

### Schema 5: Learned Behavior Schema (Skills & Learning)

**Purpose**: Model skill dependencies, combos, and learned behavior patterns.

**Node Types**:
- `Skill`: Monster skills
  - Properties: `skill_id`, `name`, `skill_type`, `cooldown_ms`, `sp_cost`, `cast_time_ms`, `damage_multiplier`, `range`
- `Behavior`: Learned behaviors
  - Properties: `behavior_id`, `name`, `description`, `success_rate`, `times_executed`, `avg_reward`
- `Pattern`: Attack/movement patterns
  - Properties: `pattern_id`, `pattern_type`, `sequence`, `effectiveness`, `context`
- `SkillUse`: Skill usage instances

**Edge Types**:
- `LEARNED_FROM`: Knowledge transfer
- `TRIGGERS`: Behavior triggers pattern
- `COUNTERS`: Skill counters another skill
- `PREREQUISITE`: Skill prerequisite
- `COMBOS_WITH`: Skill combo chains
  - Properties: `effectiveness`, `window_ms`, `damage_bonus`, `success_count`

**Use Cases**:
- Find optimal skill sequences
- Discover skill combos
- Counter enemy skills

### Schema 6: Communication Network Schema

**Purpose**: Inter-monster communication and coordination signals.

**Node Types**:
- `SignalNode`: Communication messages
  - Properties: `signal_id`, `signal_type`, `content`, `timestamp`, `priority`, `expires`, `acknowledged`

**Edge Types**:
- `SIGNALS`: Monster sends/receives signal
- `RESPONDS_TO`: Response to signal
- `COORDINATES_WITH`: Direct coordination

**Use Cases**:
- Pack leader sends formation commands
- Alert allies of danger
- Request assistance

### Schema 7: Temporal Pattern Schema

**Purpose**: Time-based patterns and event sequences.

**Node Types**:
- `TimeWindow`: Time periods
  - Properties: `window_id`, `start_time`, `end_time`, `window_type`, `activity_level`
- `Event`: Significant events
  - Properties: `event_id`, `event_type`, `timestamp`, `participants`, `outcome`, `duration_seconds`

**Edge Types**:
- `OCCURRED_DURING`: Event in time window
- `FOLLOWS`: Sequential events
- `PRECEDES`: Event precedence

**Use Cases**:
- Analyze combat patterns over time
- Predict event sequences
- Identify optimal attack times

### Schema 8: Strategy Schema

**Purpose**: Combat strategies, tactics, and counter-strategies.

**Node Types**:
- `Strategy`: High-level strategies
  - Properties: `strategy_id`, `name`, `description`, `archetype`, `complexity`, `success_rate`, `times_used`, `avg_reward`
- `Tactic`: Specific tactical maneuvers
  - Properties: `tactic_id`, `name`, `description`, `requires_allies`, `effectiveness`
- `Context`: Situational contexts
  - Properties: `context_id`, `name`, `description`, `state_pattern`

**Edge Types**:
- `EMPLOYS`: Strategy employs tactic
- `ADAPTS_TO`: Strategy adapts to context
- `COUNTERED_BY`: Counter-strategy
- `EFFECTIVE_IN`: Strategy effective in context
- `TRANSITIONS_TO`: Strategy transitions

**Use Cases**:
- Select best strategy for current context
- Find counter-strategies for enemy tactics
- Adapt strategy based on situation

### Schema 9: Memory Graph Schema

**Purpose**: Episodic memory and experience sharing.

**Node Types**:
- `MemoryNode`: Significant memories
  - Properties: `memory_id`, `memory_type`, `importance`, `timestamp`, `emotional_valence`, `retrieval_count`, `state_snapshot`
- `Experience`: Episode experiences
  - Properties: `experience_id`, `episode_id`, `monster_id`, `total_reward`, `duration_seconds`, `outcome`, `key_learnings`

**Edge Types**:
- `REMEMBERS`: Monster remembers memory
- `SIMILAR_TO`: Memory associations
- `INFLUENCED_BY`: Memory influences behavior

**Use Cases**:
- Recall similar past situations
- Share successful experiences
- Learn from failures

---

## Python API Usage

### Initialization

```python
from graph_manager import AGEGraphManager, initialize_graph_manager
import asyncpg

# Create connection pool
pool = await asyncpg.create_pool(
    host='localhost',
    port=5432,
    database='ragnarok_ml',
    user='ml_user',
    password='your_password',
    min_size=10,
    max_size=20
)

# Initialize graph manager
graph_mgr = await initialize_graph_manager(pool, graph_name='monster_ai')

# Verify health
health = await graph_mgr.verify_graph_health()
print(f"Graph healthy: {health['healthy']}")
```

### Node Operations

#### Create Monster Node

```python
node_id = await graph_mgr.create_monster_node(
    monster_id=12345,
    properties={
        'mob_id': 1002,
        'name': 'Alpha Poring',
        'level': 10,
        'archetype': 'aggressive',
        'leadership_score': 0.75,
        'experience': 1500,
        'position_x': 150,
        'position_y': 200,
        'map_id': 1
    }
)
```

#### Update Monster Position

```python
await graph_mgr.update_spatial_relationships(
    monster_id=12345,
    position=(155, 205),
    hp_ratio=0.85,
    map_id=1
)
```

#### Get Monster Node

```python
node = await graph_mgr.get_node('Monster', 12345)
print(f"Monster: {node}")
```

### Edge Operations

#### Create Pack Leadership

```python
await graph_mgr.create_pack_leadership(
    leader_id=12345,
    follower_id=12346,
    strength=0.8
)
```

#### Create Custom Edge

```python
await graph_mgr.create_edge(
    from_label='Monster',
    from_id=12345,
    to_label='Monster',
    to_id=12346,
    edge_type='PROTECTS',
    properties={
        'priority': 8,
        'distance_maintained': 3.0,
        'interventions': 0
    }
)
```

### Querying Relationships

#### Get Monster Team

```python
team_info = await graph_mgr.get_monster_team(monster_id=12345)

print(f"Leader ID: {team_info[0]['leader_id']}")
print(f"Role: {team_info[0]['role']}")
print(f"Followers: {team_info[0]['follower_ids']}")
```

#### Get Pack Formation

```python
formation = await graph_mgr.get_pack_formation(leader_id=12345)

print(f"Pack size: {formation['pack_size']}")
print(f"Cohesion: {formation['cohesion_score']:.2f}")
print(f"Avg HP: {formation['avg_hp']:.2%}")
```

#### Get Threat Network

```python
threats = await graph_mgr.get_threat_network(monster_id=12345, radius=2)

for threat_list in threats:
    print(f"Player threats: {threat_list['player_threats']}")
    print(f"Guild threats: {threat_list['guild_threats']}")
```

### Communication

#### Send Signal

```python
await graph_mgr.send_signal(
    sender_id=12345,
    receiver_id=12346,
    signal_type='target_info',
    content={
        'target_id': 150001,
        'target_hp': 0.8,
        'suggested_action': 'attack'
    },
    priority=3
)
```

#### Receive Signals

```python
signals = await graph_mgr.receive_signals(monster_id=12346)

for signal in signals:
    print(f"Signal from {signal['sender_id']}: {signal['signal_type']}")
    print(f"Content: {signal['content']}")
    print(f"Priority: {signal['priority']}")
```

### Bulk Operations

#### Bulk Create Monsters

```python
monsters = [
    {
        'id': 12345 + i,
        'mob_id': 1002,
        'name': f'Monster {i}',
        'level': 5 + i,
        'archetype': 'aggressive',
        'position_x': 100 + i,
        'position_y': 100 + i,
        'map_id': 1
    }
    for i in range(10)
]

created = await graph_mgr.bulk_create_monsters(monsters)
print(f"Created {created} monsters")
```

#### Bulk Update Positions

```python
updates = [
    {
        'monster_id': 12345 + i,
        'x': 150 + i,
        'y': 150 + i,
        'hp_ratio': 0.9,
        'map_id': 1
    }
    for i in range(10)
]

updated = await graph_mgr.bulk_update_positions(updates)
print(f"Updated {updated} positions")
```

### Player Encounter Tracking

#### Record Encounter

```python
await graph_mgr.record_player_encounter(
    monster_id=12345,
    player_char_id=150001,
    encounter_data={
        'outcome': 'monster_won',
        'damage_dealt_to_monster': 350,
        'damage_taken_from_monster': 120,
        'strategy_used': 'rush',
        'duration_seconds': 45.5,
        'map_id': 1
    }
)
```

#### Analyze Player Strategy

```python
strategy = await graph_mgr.get_player_strategy_pattern(
    player_char_id=150001,
    monster_archetype='aggressive',
    lookback_hours=24
)

if strategy:
    print(f"Player's common strategy: {strategy['strategy']}")
    print(f"Usage count: {strategy['usage_count']}")
    print(f"Win rate: {strategy['win_rate']:.2%}")
```

### Learning and Patterns

#### Record Learned Pattern

```python
await graph_mgr.learn_behavior_pattern(
    monster_id=12345,
    pattern_data={
        'pattern_type': 'attack_sequence',
        'effectiveness': 0.78,
        'sequence': ['basic_attack', 'skill_1', 'basic_attack']
    }
)
```

#### Get Learned Behaviors

```python
behaviors = await graph_mgr.get_learned_behaviors(monster_id=12345)

for behavior in behaviors:
    print(f"Learned: {behavior['name']} (proficiency: {behavior['relationship_props']['proficiency']})")
```

### Statistics and Maintenance

#### Get Graph Statistics

```python
stats = await graph_mgr.get_graph_statistics()

print(f"Total Nodes: {stats['total_nodes']}")
print(f"Total Edges: {stats['total_edges']}")
print(f"Avg Query Time: {stats['avg_query_time_ms']:.2f}ms")

for node_type in stats['node_counts']:
    print(f"  {node_type['label']}: {node_type['count']}")
```

#### Cleanup Old Data

```python
cleanup_results = await graph_mgr.cleanup_expired_data(retention_days=30)

print(f"Deleted {cleanup_results['encounters_deleted']} old encounters")
print(f"Deleted {cleanup_results['signals_deleted']} expired signals")
```

---

## Common Query Patterns

### Pattern 1: Pack Coordination Query

**Use Case**: Get all pack members for coordinated attack

```python
async def get_pack_for_coordination(monster_id: int, graph_mgr):
    """Get pack members and their status for coordination"""
    
    # Get team info
    team_info = await graph_mgr.get_monster_team(monster_id)
    
    if not team_info:
        return None
    
    team_data = team_info[0]
    leader_id = team_data.get('leader_id')
    role = team_data.get('role')
    
    # If this monster is leader, get formation
    if role == 'leader':
        formation = await graph_mgr.get_pack_formation(monster_id)
        return {
            'is_leader': True,
            'pack_size': formation['pack_size'],
            'members': formation['members'],
            'cohesion': formation['cohesion_score']
        }
    else:
        # This monster is follower, get leader's formation
        if leader_id:
            formation = await graph_mgr.get_pack_formation(leader_id)
            return {
                'is_leader': False,
                'leader_id': leader_id,
                'pack_size': formation['pack_size'],
                'members': formation['members']
            }
    
    return None
```

### Pattern 2: Threat Assessment Query

**Use Case**: Assess incoming threat before engagement

```python
async def assess_player_threat(player_char_id: int, monster_archetype: str, graph_mgr):
    """Assess player threat based on history"""
    
    # Get player's strategy pattern
    strategy = await graph_mgr.get_player_strategy_pattern(
        player_char_id=player_char_id,
        monster_archetype=monster_archetype,
        lookback_hours=24
    )
    
    if strategy:
        return {
            'threat_level': 'high' if strategy['win_rate'] > 0.7 else 'medium' if strategy['win_rate'] > 0.4 else 'low',
            'common_strategy': strategy['strategy'],
            'win_rate': strategy['win_rate'],
            'experience_count': strategy['usage_count']
        }
    
    return {
        'threat_level': 'unknown',
        'common_strategy': None,
        'win_rate': 0.5,
        'experience_count': 0
    }
```

### Pattern 3: Skill Combo Query

**Use Case**: Find optimal skill combo for current situation

```python
async def find_optimal_skill_combo(available_skills: list, graph_mgr):
    """Find best skill combo from available skills"""
    
    query = f"""
        MATCH (s1:Skill)-[combo:COMBOS_WITH]->(s2:Skill)
        WHERE s1.skill_id IN {available_skills}
          AND s2.skill_id IN {available_skills}
          AND combo.success_count > 10
        RETURN s1.skill_id AS skill1,
               s2.skill_id AS skill2,
               combo.effectiveness AS effectiveness,
               combo.window_ms AS window_ms
        ORDER BY combo.effectiveness DESC
        LIMIT 3
    """
    
    results = await graph_mgr.execute_cypher(
        query,
        returns_schema=[
            ('skill1', 'agtype'),
            ('skill2', 'agtype'),
            ('effectiveness', 'agtype'),
            ('window_ms', 'agtype')
        ]
    )
    
    return results
```

### Pattern 4: Territory Intrusion Detection

**Use Case**: Detect if enemy is intruding on territory

```python
async def detect_territory_intrusion(monster_id: int, enemy_position: tuple, graph_mgr):
    """Check if enemy is in our territory"""
    
    enemy_x, enemy_y = enemy_position
    
    query = f"""
        MATCH (owner:Monster {{id: {monster_id}}})-[:CLAIMS_TERRITORY]->(territory:Territory)
        WHERE point.distance([{enemy_x}, {enemy_y}], [territory.center_x, territory.center_y]) < territory.radius
        RETURN territory.territory_id AS territory_id,
               territory.center_x AS center_x,
               territory.center_y AS center_y,
               territory.radius AS radius
    """
    
    results = await graph_mgr.execute_cypher(
        query,
        returns_schema=[
            ('territory_id', 'agtype'),
            ('center_x', 'agtype'),
            ('center_y', 'agtype'),
            ('radius', 'agtype')
        ]
    )
    
    return len(results) > 0, results
```

---

## Performance Tuning

### Index Optimization

Indexes are critical for <5ms query performance. The setup script creates indexes on:

- `Monster.id` (primary lookup)
- `Monster.archetype` (filtering by type)
- `Monster.leadership_score` (finding leaders)
- `Monster.map_id` (spatial queries)
- `Player.char_id` (player lookups)
- `Skill.skill_id` (skill lookups)
- `Strategy.archetype` (strategy filtering)

#### Create Additional Indexes

```sql
-- Index on Monster HP for finding vulnerable allies
SELECT * FROM cypher('monster_ai', $$
    CREATE INDEX ON :Monster (current_hp_ratio)
$$) AS (result agtype);

-- Index on Player threat rating
SELECT * FROM cypher('monster_ai', $$
    CREATE INDEX ON :Player (threat_rating)
$$) AS (result agtype);

-- PostgreSQL GIN index on properties (for complex queries)
CREATE INDEX idx_monster_properties_gin 
ON monster_ai."Monster" USING gin(properties);
```

### Query Optimization Tips

1. **Limit Path Depth**: Keep path queries to max depth 3
   ```python
   # Good: Limited depth
   path = await graph_mgr.find_shortest_path(..., max_depth=3)
   
   # Bad: Unlimited depth (slow)
   # path = await graph_mgr.find_shortest_path(..., max_depth=100)
   ```

2. **Use Specific Edge Types**: Filter by edge type when possible
   ```python
   # Good: Specific edge type
   neighbors = await graph_mgr.get_neighbors(..., edge_type='LEADS')
   
   # Slower: All edge types
   # neighbors = await graph_mgr.get_neighbors(..., edge_type=None)
   ```

3. **Limit Result Sets**: Always use LIMIT in Cypher queries
   ```cypher
   MATCH (m:Monster)
   WHERE m.level > 50
   RETURN m
   LIMIT 100  -- Always limit results
   ```

4. **Cache Graph Query Results**: Use DragonFlyDB to cache frequently accessed graph data
   ```python
   # Check cache first
   cached = df_client.get(f"graph:pack:{monster_id}")
   
   if not cached:
       # Query graph
       pack = await graph_mgr.get_pack_formation(monster_id)
       # Cache for 5 seconds
       df_client.setex(f"graph:pack:{monster_id}", 5, json.dumps(pack))
   ```

### Connection Pooling

Use connection pooling for optimal performance:

```python
# Configure pool size based on load
pool = await asyncpg.create_pool(
    min_size=10,  # Minimum connections
    max_size=20,  # Maximum connections
    command_timeout=10,  # 10s timeout
    statement_cache_size=200  # Cache prepared statements
)
```

### Batch Operations

Batch operations are significantly faster:

```python
# Slow: Individual updates
for monster in monsters:
    await graph_mgr.update_spatial_relationships(...)

# Fast: Bulk update
await graph_mgr.bulk_update_positions(monsters)
```

---

## Troubleshooting

### Common Issues

#### 1. "Extension 'age' not found"

**Cause**: Apache AGE not installed or not in shared_preload_libraries

**Solution**:
```bash
# Verify AGE is installed
ls /usr/lib/postgresql/17/lib/ | grep age

# Check postgresql.conf
grep shared_preload_libraries /etc/postgresql/17/main/postgresql.conf

# Should contain 'age'
# If not, add it and restart PostgreSQL
sudo systemctl restart postgresql
```

#### 2. "Graph 'monster_ai' does not exist"

**Cause**: Setup SQL not executed

**Solution**:
```bash
# Run setup script
cd rathena-AI-world/ml_inference_service
./deploy_age.sh
```

#### 3. "Permission denied for schema ag_catalog"

**Cause**: User lacks permissions on AGE schema

**Solution**:
```sql
-- Grant permissions to ml_user
GRANT USAGE ON SCHEMA ag_catalog TO ml_user;
GRANT SELECT ON ALL TABLES IN SCHEMA ag_catalog TO ml_user;
GRANT ALL ON GRAPH monster_ai TO ml_user;
```

#### 4. Slow Query Performance (>10ms)

**Cause**: Missing indexes or large graph size

**Solution**:
```sql
-- Analyze tables for query planner
ANALYZE monster_ai."Monster";
ANALYZE monster_ai."Player";

-- Check for missing indexes
SELECT * FROM pg_indexes WHERE schemaname = 'monster_ai';

-- Create additional indexes as needed
```

#### 5. "Connection pool exhausted"

**Cause**: Too many concurrent graph queries

**Solution**:
```yaml
# Increase pool size in inference_config.yaml
database:
  postgresql:
    min_pool_size: 20  # Increase from 10
    max_pool_size: 40  # Increase from 20
```

#### 6. Memory Issues with Large Graph

**Cause**: Graph grown too large, queries consuming too much memory

**Solution**:
```python
# Run cleanup more frequently
await graph_mgr.cleanup_expired_data(retention_days=7)  # Shorter retention

# Or configure PostgreSQL work_mem
# In postgresql.conf:
work_mem = 256MB  # Increase for complex queries
```

### Debugging

#### Enable Debug Logging

```python
import logging
logging.getLogger('graph_manager').setLevel(logging.DEBUG)
```

#### Check Query Execution Plan

```sql
-- Use EXPLAIN to analyze query performance
EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM cypher('monster_ai', $$
    MATCH (m:Monster)-[:LEADS]->(f:Monster)
    WHERE m.id = 12345
    RETURN f.name
$$) AS (name agtype);
```

#### Monitor Graph Size

```python
stats = await graph_mgr.get_graph_statistics()
print(f"Total nodes: {stats['total_nodes']}")
print(f"Total edges: {stats['total_edges']}")

# If graph is too large (>1M nodes), consider partitioning or cleanup
```

### Performance Benchmarks

**Target Performance**:
- Simple node lookup: <1ms
- Pack hierarchy query: <5ms
- Path finding (depth 3): <10ms
- Bulk updates (100 monsters): <50ms

**Measuring Performance**:

```python
import time

# Measure query time
start = time.perf_counter()
result = await graph_mgr.get_node('Monster', 12345)
elapsed_ms = (time.perf_counter() - start) * 1000

print(f"Query time: {elapsed_ms:.2f}ms")

# Get aggregate metrics
perf = graph_mgr.get_performance_metrics()
print(f"Avg query time: {perf['avg_query_time_ms']:.2f}ms")
print(f"Error rate: {perf['error_rate']:.2%}")
```

---

## Integration with ML Inference Pipeline

### Before Inference: Query Graph Context

```python
async def enrich_request_with_graph_context(request, graph_mgr):
    """Add graph context to inference request"""
    
    monster_id = request['monster_id']
    
    # Get pack coordination data
    team_info = await graph_mgr.get_monster_team(monster_id)
    
    # Get threat network
    threats = await graph_mgr.get_threat_network(monster_id, radius=2)
    
    # Add to request
    request['graph_context'] = {
        'team_info': team_info,
        'threats': threats,
        'has_pack': len(team_info) > 0 and team_info[0].get('role') != 'independent'
    }
    
    return request
```

### After Inference: Update Graph State

```python
async def update_graph_after_inference(request, response, graph_mgr):
    """Update graph with new state after inference"""
    
    monster_id = request['monster_id']
    
    # Update position and HP
    await graph_mgr.update_spatial_relationships(
        monster_id=monster_id,
        position=(request['position_x'], request['position_y']),
        hp_ratio=request['hp_ratio'],
        map_id=request['map_id']
    )
    
    # If coordination action, send signals
    if response.get('coordination_action'):
        # Determine pack members
        team_info = await graph_mgr.get_monster_team(monster_id)
        
        if team_info and team_info[0].get('follower_ids'):
            for follower_id in team_info[0]['follower_ids']:
                await graph_mgr.send_signal(
                    sender_id=monster_id,
                    receiver_id=follower_id,
                    signal_type='formation_command',
                    content={'action': response['coordination_action']},
                    priority=2
                )
```

---

## Configuration Reference

### inference_config.yaml

```yaml
graph:
  enabled: true
  graph_name: "monster_ai"
  use_for_coordination: true      # Use graph for pack coordination
  use_for_spatial: true            # Update spatial relationships
  use_for_threat_tracking: true   # Track player threats
  cache_duration: 5                # Cache graph results for 5 seconds
  max_query_depth: 3               # Maximum path depth
  retry_attempts: 3                # Query retry attempts
  retry_delay: 0.1                 # Initial retry delay (seconds)
  query_timeout: 10                # Query timeout (seconds)
```

### Environment Variables

```bash
# Database connection
export POSTGRES_HOST=localhost
export POSTGRES_PORT=5432
export POSTGRES_DB=ragnarok_ml
export POSTGRES_USER=ml_user
export POSTGRES_PASSWORD=your_secure_password

# Python path (if needed)
export PYTHONPATH=/opt/ml_monster_ai:$PYTHONPATH
```

---

## Testing

### Run All Tests

```bash
# With pytest
pytest test_graph_integration.py -v

# Standalone
python test_graph_integration.py

# Specific test class
pytest test_graph_integration.py::TestNodeCRUD -v

# With coverage
pytest test_graph_integration.py --cov=graph_manager --cov-report=html
```

### Manual Testing

```bash
# Test graph manager directly
python -m graph_manager

# Test deployment script
./deploy_age.sh --check-only

# Full deployment
./deploy_age.sh
```

### Verify Integration

```bash
# Start inference service
python main.py

# Check logs for graph initialization
tail -f /opt/ml_monster_ai/logs/inference_service.log | grep -i graph

# Expected output:
# INFO - Initializing Apache AGE graph manager...
# INFO - ✓ Graph manager initialized: X nodes, Y edges, Z.ZZms avg query time
# INFO - Graph enabled: True
```

---

## Best Practices

### 1. Error Handling

Always handle graph errors gracefully - they should not block inference:

```python
try:
    team_info = await graph_mgr.get_monster_team(monster_id)
except Exception as e:
    logger.warning(f"Graph query failed (non-critical): {e}")
    team_info = None  # Continue without graph data
```

### 2. Caching

Cache frequently accessed graph data:

```python
# Cache pack formation for 5 seconds
cache_key = f"pack_formation:{leader_id}"
cached = df_client.get(cache_key)

if cached:
    formation = json.loads(cached)
else:
    formation = await graph_mgr.get_pack_formation(leader_id)
    df_client.setex(cache_key, 5, json.dumps(formation))
```

### 3. Cleanup

Run cleanup regularly to prevent graph bloat:

```python
# In main service loop, run cleanup daily
if current_time - last_cleanup > 86400:  # 24 hours
    await graph_mgr.cleanup_expired_data(retention_days=30)
    last_cleanup = current_time
```

### 4. Monitoring

Monitor graph query performance:

```python
# Log slow queries
if query_time_ms > 10:
    logger.warning(f"Slow graph query: {query_time_ms:.2f}ms")

# Track metrics
perf = graph_mgr.get_performance_metrics()
if perf['avg_query_time_ms'] > 10:
    logger.warning(f"Graph queries averaging {perf['avg_query_time_ms']:.2f}ms")
```

### 5. Transactions

Use transactions for multi-step operations:

```python
# Create monster and establish relationships atomically
await graph_mgr.execute_in_transaction([
    ("CREATE (m:Monster {id: 12345, name: 'Test'})", None),
    ("MATCH (m1:Monster {id: 12345}), (m2:Monster {id: 12346}) CREATE (m1)-[:LEADS]->(m2)", None)
])
```

---

## References

### Documentation

- **Apache AGE Official Docs**: https://age.apache.org/
- **OpenCypher Query Language**: https://opencypher.org/
- **Architecture Document**: [`plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md`](../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md)
- **asyncpg Documentation**: https://magicstack.github.io/asyncpg/

### Files

- **Setup SQL**: [`setup_age_graphs.sql`](setup_age_graphs.sql)
- **Graph Manager**: [`graph_manager.py`](graph_manager.py)
- **Main Service**: [`main.py`](main.py)
- **Configuration**: [`inference_config.yaml`](inference_config.yaml)
- **Deployment Script**: [`deploy_age.sh`](deploy_age.sh)
- **Tests**: [`test_graph_integration.py`](test_graph_integration.py)

### Quick Reference

#### Common Cypher Patterns

```cypher
-- Create node
CREATE (n:Label {property: value})

-- Match node
MATCH (n:Label {id: $id})

-- Create edge
MATCH (a:Label1 {id: $id1}), (b:Label2 {id: $id2})
CREATE (a)-[:EDGE_TYPE {prop: val}]->(b)

-- Path query
MATCH path = (a:Label1)-[:EDGE*1..3]->(b:Label2)
RETURN path

-- Update properties
MATCH (n:Label {id: $id})
SET n.prop1 = $val1, n.prop2 = $val2

-- Delete with relationships
MATCH (n:Label {id: $id})
DETACH DELETE n
```

#### Python API Quick Reference

```python
# Node operations
await graph_mgr.create_monster_node(id, props)
await graph_mgr.update_node(label, id, props)
await graph_mgr.delete_node(label, id, detach=True)
await graph_mgr.get_node(label, id)

# Edge operations
await graph_mgr.create_edge(from_label, from_id, to_label, to_id, edge_type, props)
await graph_mgr.get_edges(from_label, from_id, edge_type)

# Queries
await graph_mgr.get_neighbors(label, id, direction, edge_type)
await graph_mgr.find_shortest_path(from_label, from_id, to_label, to_id)
await graph_mgr.get_monster_team(monster_id)
await graph_mgr.get_threat_network(monster_id, radius)

# Communication
await graph_mgr.send_signal(sender, receiver, type, content, priority)
await graph_mgr.receive_signals(monster_id)

# Maintenance
await graph_mgr.get_graph_statistics()
await graph_mgr.cleanup_expired_data(retention_days)
```

---

## Support

For issues, questions, or contributions:

1. Check this documentation
2. Review test examples in [`test_graph_integration.py`](test_graph_integration.py)
3. Check logs: `/opt/ml_monster_ai/logs/inference_service.log`
4. Consult architecture document: [`plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md`](../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md)

---

**Document Version**: 2.0  
**Last Updated**: 2026-01-25  
**Maintained By**: ML Monster AI Team
