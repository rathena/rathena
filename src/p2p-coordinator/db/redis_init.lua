-- DragonflyDB/Redis Initialization Script
-- Key Patterns, TTLs, GeoHash, Eviction, Cache Warming

-- Key Patterns:
-- player:account:{id}         - Player account cache (hash)
-- player:inventory:{id}       - Player inventory (set or hash)
-- entity:ownership:{entityId} - Entity ownership (string or hash)
-- combat:log:{yyyy-mm}        - Combat logs (list, partitioned by month)
-- worker:load:{workerId}      - Worker load (string)
-- player:state:{id}           - Player state (hash)
-- migration:ownership:{id}    - Ownership migration events (list)
-- peer:reputation:{id}        - Peer reputation (sorted set)
-- cheat:flags:{playerId}      - Cheat flags (set)

-- TTL Configurations (in seconds):
-- Player session: 86400 (1 day)
-- Inventory: 3600 (1 hour)
-- Combat logs: 2592000 (30 days)
-- Worker load: 300 (5 minutes)
-- Player state: 600 (10 minutes)
-- Ownership migrations: 2592000 (30 days)
-- Peer reputation: 604800 (7 days)
-- Cheat flags: 2592000 (30 days)

-- Example: Set TTLs for new keys
-- redis.call('EXPIRE', 'player:account:123', 86400)

-- GeoHash Indexing for spatial queries (e.g., entity locations)
-- GEOADD entity:locations {longitude} {latitude} {entityId}
-- Example:
-- redis.call('GEOADD', 'entity:locations', 101.6869, 3.1390, 'entity-uuid-1')

-- Eviction Policy (to be set in DragonflyDB/Redis config, not script):
-- volatile-lru or allkeys-lru recommended for cache keys

-- Cache Warming Procedure (example: pre-load hot player accounts)
-- for i, id in ipairs(hot_player_ids) do
--   redis.call('GET', 'player:account:' .. id)
-- end

-- Lua script for cache warming (example)
local hot_player_ids = {'1', '2', '3'}
for i, id in ipairs(hot_player_ids) do
  redis.call('GET', 'player:account:' .. id)
end

-- Set up TTLs for demonstration
redis.call('EXPIRE', 'player:account:1', 86400)
redis.call('EXPIRE', 'player:inventory:1', 3600)
redis.call('EXPIRE', 'combat:log:2025-11', 2592000)
redis.call('EXPIRE', 'worker:load:worker-uuid-1', 300)
redis.call('EXPIRE', 'player:state:1', 600)
redis.call('EXPIRE', 'migration:ownership:1', 2592000)
redis.call('EXPIRE', 'peer:reputation:1', 604800)
redis.call('EXPIRE', 'cheat:flags:1', 2592000)

-- Note: For full initialization, run this script with redis-cli or as part of your app's startup.