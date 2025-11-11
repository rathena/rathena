# OpenMemory SDK Integration

## Overview

The AI Autonomous World system uses **OpenMemory SDK** for persistent long-term memory storage.

## Architecture

```
AI Service (Python/FastAPI)
    ↓
OpenMemory Python SDK (openmemory-py)
    ↓ HTTP API
OpenMemory Backend (Node.js/TypeScript)
    ↓
PostgreSQL Database (with pgvector)
```

## Components

### 1. OpenMemory Backend Service
- **Location**: `external-references/OpenMemory/backend/`
- **Port**: 8081
- **Technology**: Node.js/TypeScript
- **Database**: PostgreSQL (database: `openmemory`)
- **Embeddings**: Synthetic (no API key required)
- **Tier**: Hybrid (high accuracy, 700-800 QPS)

### 2. OpenMemory Python SDK
- **Package**: `openmemory-py` version 0.3.0
- **Installation**: `pip install /path/to/external-references/OpenMemory/sdk-py/`
- **Type**: HTTP client wrapper (requires backend service)

### 3. AI Service Integration
- **Manager**: `ai-service/memory/openmemory_manager.py`
- **Configuration**: `ai-service/config.py` and `ai-service/.env`
- **Usage**: `ai-service/routers/player.py`

## Configuration

### OpenMemory Backend (.env)
```bash
OM_PORT=8081
OM_TIER=hybrid
OM_METADATA_BACKEND=postgres
OM_VECTOR_BACKEND=postgres
OM_PG_HOST=localhost
OM_PG_PORT=5432
OM_PG_DB=openmemory
OM_PG_USER=ai_world_user
OM_PG_PASSWORD=ai_world_pass_2025
OM_EMBEDDINGS=synthetic
```

### AI Service (.env)
```bash
OPENMEMORY_URL=http://localhost:8081
OPENMEMORY_API_KEY=
```

## Features

### Memory Storage
- **Automatic**: Every player-NPC conversation is automatically stored
- **Rich Context**: Includes player name, NPC name, NPC class, timestamp, conversation type
- **User Isolation**: Each player has separate memory space (user_id: `player_{player_id}`)
- **Sector Classification**: All conversations stored in `episodic` sector
- **High Salience**: Conversations stored with salience=0.8 for importance

### Memory Retrieval
- **Keyword-Based**: Uses keyword overlap scoring for relevance
- **Top-K Selection**: Retrieves top 3 most relevant memories
- **Context Integration**: Retrieved memories included in LLM prompts
- **Fallback**: Gracefully degrades if OpenMemory unavailable

### Memory Persistence
- **PostgreSQL Storage**: All memories persist in database
- **Survives Restarts**: Memories available after service restarts
- **No TTL**: Unlike DragonflyDB cache, memories don't expire

## Implementation Details

### Memory Storage (routers/player.py)
```python
conversation_text = (
    f"Conversation between {player_name} and {npc_name} ({npc_class}):\n"
    f"{player_name}: {message}\n"
    f"{npc_name}: {response_text}"
)

result = openmemory_client.add(
    content=conversation_text,
    tags=["conversation", "npc_interaction", npc_class, player_name, npc_name],
    metadata={...},
    salience=0.8,
    user_id=f"player_{player_id}"
)
```

### Memory Retrieval (routers/player.py)
```python
# Get all memories for player
all_memories = openmemory_client.all(limit=50)
player_memories = [mem for mem in all_memories if mem.get("user_id") == f"player_{player_id}"]

# Keyword-based relevance scoring
keywords = set(message.lower().split())
scored_memories = []
for mem in player_memories:
    matches = sum(1 for keyword in keywords if keyword in mem.get("content", "").lower())
    if matches > 0:
        scored_memories.append((matches, mem))

# Top 3 most relevant
relevant_memories = [mem for score, mem in sorted(scored_memories, reverse=True)[:3]]
```

## Known Limitations

1. **Vector Search**: OpenMemory's synthetic embeddings are unreliable for vector similarity search
   - **Workaround**: Using keyword-based filtering instead
   - **Future**: Consider switching to real embeddings (OpenAI, Gemini, Ollama) for production

2. **User ID Filter**: OpenMemory backend's `user_id` filter in query() doesn't work correctly
   - **Workaround**: Retrieve all memories and filter in Python

3. **Backend Dependency**: Python SDK requires Node.js backend service running
   - **Impact**: Cannot use SDK standalone
   - **Mitigation**: Backend runs as separate service on port 8081

## Testing

### Verify Memory Storage
```bash
curl -X POST "http://192.168.0.100:8000/ai/player/chat" \
  -H "Content-Type: application/json" \
  -d '{"npc_id":"ai_explorer_002","player_id":"150000","player_name":"TestAdmin","message":"Hello!"}'
```

### Check Stored Memories
```python
from openmemory import OpenMemory
om = OpenMemory(base_url="http://localhost:8081")
result = om.all(limit=50)
player_memories = [m for m in result.get('items', []) if m.get('user_id') == 'player_150000']
```

### Verify Persistence
1. Restart AI service: `pkill -f "uvicorn main:app" && uvicorn main:app ...`
2. Send message asking about previous conversation
3. Check logs for "Retrieved X relevant memories from OpenMemory"

## Maintenance

### Start OpenMemory Backend
```bash
cd external-references/OpenMemory/backend
OM_PORT=8081 OM_TIER=hybrid npm run dev
```

### Check Backend Health
```bash
curl http://localhost:8081/health | jq .
```

### Database Maintenance
```bash
# Connect to database
PGPASSWORD=ai_world_pass_2025 psql -U ai_world_user -h localhost -d openmemory

# View memories
SELECT id, content, user_id, primary_sector, created_at FROM memories ORDER BY created_at DESC LIMIT 10;
```

## Status

✅ **Fully Functional**
- Memory storage working
- Memory retrieval working
- Memory persistence working
- Context integration working
- End-to-end testing complete

## Future Improvements

1. **Real Embeddings**: Switch from synthetic to OpenAI/Gemini embeddings for better vector search
2. **Memory Consolidation**: Implement periodic memory summarization to reduce storage
3. **Memory Decay**: Tune decay parameters for different memory types
4. **Graph Associations**: Leverage OpenMemory's graph features for relationship tracking
5. **Multi-Sector**: Use different sectors (semantic, procedural, emotional) for different memory types

