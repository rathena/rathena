# AI-Driven Autonomous World System - Architecture

## Executive Summary

This document outlines the architecture for transforming rAthena MMORPG into a living, breathing world with AI-driven NPCs and adaptive systems. The design prioritizes:

- **Non-invasive extension architecture** - Minimal modifications to rAthena core
- **Scalability** - Support for hundreds to thousands of autonomous NPCs
- **Flexibility** - Provider-agnostic LLM integration with Azure OpenAI Foundry as default
- **Emergent behavior** - Real-world complexity simulation without artificial constraints
- **Maintainability** - Clean separation of concerns and extensible design

## System Architecture Overview

### High-Level Architecture

The system consists of five major layers:

1. **rAthena Game Server Layer** - Existing MMORPG server (minimal modifications)
2. **Bridge Layer** - Custom extension connecting rAthena to AI services
3. **AI Service Layer** - Python-based autonomous agent orchestration
4. **State Management Layer** - DragonflyDB for shared state and caching
5. **LLM Provider Layer** - Abstracted interface to multiple LLM providers

### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        GAME CLIENTS                              │
│                    (Ragnarok Online Client)                      │
└────────────────────────────┬────────────────────────────────────┘
                             │ Game Protocol
┌────────────────────────────▼────────────────────────────────────┐
│                    rAthena GAME SERVER                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ Map Server   │  │ Char Server  │  │ Login Server │          │
│  └──────┬───────┘  └──────────────┘  └──────────────┘          │
│         │                                                         │
│  ┌──────▼───────────────────────────────────────────┐           │
│  │         Custom NPC Scripts (Event Hooks)         │           │
│  └──────┬───────────────────────────────────────────┘           │
└─────────┼─────────────────────────────────────────────────────┘
          │ HTTP/WebSocket API
┌─────────▼─────────────────────────────────────────────────────┐
│                      BRIDGE LAYER                               │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  rAthena Extension (C++ Plugin / Web API Extension)      │  │
│  │  - Event Listener & Dispatcher                           │  │
│  │  - NPC Action Executor                                   │  │
│  │  - State Synchronizer                                    │  │
│  └──────┬───────────────────────────────────────────────────┘  │
└─────────┼──────────────────────────────────────────────────────┘
          │ REST/gRPC API
┌─────────▼─────────────────────────────────────────────────────┐
│                    AI SERVICE LAYER (Python)                    │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │              CrewAI Multi-Agent Orchestrator             │  │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────┐        │  │
│  │  │ NPC Agents │  │World Agents│  │Event Agents│        │  │
│  │  └────────────┘  └────────────┘  └────────────┘        │  │
│  └──────┬───────────────────────────────────────────────────┘  │
│         │                                                        │
│  ┌──────▼───────────────────────────────────────────────────┐  │
│  │         Memori SDK (Memory & Context Management)         │  │
│  │  - NPC Memories & Personalities                          │  │
│  │  - World State & History                                 │  │
│  │  - Relationship Graphs                                   │  │
│  └──────┬───────────────────────────────────────────────────┘  │
│         │                                                        │
│  ┌──────▼───────────────────────────────────────────────────┐  │
│  │         LLM Provider Abstraction Layer                   │  │
│  │  - Azure OpenAI Foundry (Default)                        │  │
│  │  - OpenAI, DeepSeek, Gemini, Ollama, Claude             │  │
│  └──────┬───────────────────────────────────────────────────┘  │
└─────────┼──────────────────────────────────────────────────────┘
          │ Read/Write Operations
┌─────────▼─────────────────────────────────────────────────────┐
│              STATE MANAGEMENT LAYER (DragonflyDB)              │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  - NPC State & Consciousness Data                        │  │
│  │  - World State (Economy, Politics, Environment)          │  │
│  │  - Event Queue & Action History                          │  │
│  │  - Relationship & Social Graphs                          │  │
│  │  - Caching Layer for LLM Responses                       │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Component Architecture

### 1. rAthena Game Server Layer

**Modifications Required:**
- Minimal - only custom NPC scripts in `/npc/custom/ai-world/`
- No core C++ code changes required
- Leverage existing event system (OnInit, OnTimer, OnTouch, etc.)

**Components:**
- **AI-Enabled NPC Scripts**: Custom scripts that hook into AI service
- **Event Dispatchers**: Scripts that send game events to Bridge Layer
- **Action Receivers**: Scripts that execute AI-decided actions

**Example NPC Script Structure:**
```c
// npc/custom/ai-world/ai_npc_base.txt
-	script	AI_NPC_Base	-1,{
OnInit:
    // Register NPC with AI service
    callsub(RegisterWithAI);
    end;

OnTimer1000:
    // Periodic AI decision cycle
    callsub(RequestAIDecision);
    end;

OnTouch:
    // Player interaction
    callsub(HandlePlayerInteraction);
    end;
}
```

### 2. Bridge Layer

**Purpose**: Connect rAthena to AI Service without modifying core code

**Implementation Options:**

**Option A: Web API Extension (Recommended)**
- Extend existing rAthena web server (`src/web/`)
- Add new endpoints for AI communication
- Minimal C++ code additions in isolated module

**Option B: Custom Inter-Server Protocol**
- Create new server type similar to char/login/map servers
- More invasive but potentially more performant

**Recommended: Option A - Web API Extension**

**Key Components:**

#### Event Listener & Dispatcher
- Listens to rAthena game events via custom NPC script callbacks
- Translates game events to AI service API calls
- Queues events in DragonflyDB for async processing

#### NPC Action Executor
- Receives AI decisions from AI service
- Translates AI actions to rAthena script commands
- Executes actions via NPC script interface

#### State Synchronizer
- Keeps DragonflyDB synchronized with rAthena game state
- Periodic sync of world state (economy, player positions, etc.)
- Real-time sync of critical events

**API Endpoints:**
```
POST /ai/npc/register          - Register NPC with AI service
POST /ai/npc/event             - Send game event to AI service
GET  /ai/npc/{id}/action       - Get next action for NPC
POST /ai/world/state           - Update world state
GET  /ai/world/state           - Get current world state
POST /ai/player/interaction    - Handle player-NPC interaction
```

### 3. AI Service Layer

**Technology Stack:**
- **Language**: Python 3.11+
- **Framework**: FastAPI (NOT Express.js as per rules)
- **Agent Orchestration**: CrewAI
- **Memory Management**: Memori SDK
- **Async Processing**: asyncio, Celery for background tasks

**Architecture:**

#### CrewAI Multi-Agent Orchestrator

**Agent Types:**

1. **NPC Consciousness Agents** (One per autonomous NPC)
   - Individual personality and decision-making
   - Memory and learning capabilities
   - Goal-oriented behavior
   - Social interaction processing

2. **World System Agents** (Singleton or few instances)
   - Economy Agent: Manages supply/demand, pricing, trade
   - Politics Agent: Manages factions, alliances, conflicts
   - Environment Agent: Weather, seasons, natural events
   - Quest Agent: Dynamic quest generation

3. **Meta Agents** (Orchestration)
   - Event Coordinator: Routes events to appropriate agents
   - Coherence Monitor: Ensures world consistency
   - Performance Optimizer: Manages LLM call frequency

**Agent Communication Flow:**
```
Game Event → Event Coordinator → Relevant Agents → Decision → Action Queue → Bridge Layer
```

#### Memori SDK Integration

**Memory Structures:**

1. **NPC Memory**
   - Short-term: Recent interactions, current goals
   - Long-term: Personality traits, relationships, significant events
   - Episodic: Specific event memories with emotional context
   - Semantic: General knowledge about the world

2. **World Memory**
   - Historical events and their impacts
   - Economic trends and patterns
   - Political history and faction evolution
   - Environmental changes over time

3. **Relationship Graphs**
   - NPC-to-NPC relationships (friendship, rivalry, etc.)
   - NPC-to-Player relationships
   - Faction relationships
   - Dynamic relationship evolution

**Memory Retrieval Strategy:**
- Vector similarity search for relevant memories
- Temporal decay for older memories
- Emotional salience boosting
- Context-aware retrieval

### 4. LLM Provider Abstraction Layer

**Design Pattern**: Strategy Pattern with Factory

**Interface:**
```python
class LLMProvider(ABC):
    @abstractmethod
    async def generate(self, prompt: str, context: dict) -> str:
        pass

    @abstractmethod
    async def generate_structured(self, prompt: str, schema: dict) -> dict:
        pass

    @abstractmethod
    def get_embedding(self, text: str) -> List[float]:
        pass
```

**Implementations:**
- AzureOpenAIProvider (Default)
- OpenAIProvider
- DeepSeekProvider
- GeminiProvider
- OllamaProvider (Local)
- ClaudeProvider

**Configuration:**
```yaml
llm:
  default_provider: azure_openai
  providers:
    azure_openai:
      endpoint: ${AZURE_OPENAI_ENDPOINT}
      api_key: ${AZURE_OPENAI_KEY}
      deployment: gpt-4
      embedding_deployment: text-embedding-ada-002
    openai:
      api_key: ${OPENAI_API_KEY}
      model: gpt-4-turbo
    ollama:
      endpoint: http://localhost:11434
      model: llama3
```

**Provider Selection Strategy:**
- Default provider for most operations
- Fallback chain for reliability
- Cost-based routing (cheap models for simple tasks)
- Latency-based routing (local models for real-time needs)

### 5. State Management Layer (DragonflyDB)

**Why DragonflyDB over Redis:**
- Better performance for high-throughput scenarios
- Lower memory footprint
- Better multi-threading support
- Redis-compatible API

**Data Structures:**

#### NPC State
```
Key: npc:{npc_id}
Type: Hash
Fields:
  - id, name, class, level
  - position (map, x, y)
  - personality_vector (embedding)
  - current_goal, current_action
  - moral_alignment (good/evil/neutral score)
  - faction_id, faction_loyalty
  - economic_status (wealth, inventory)
  - last_decision_timestamp
```

#### NPC Memory
```
Key: npc:{npc_id}:memory:{memory_id}
Type: Hash + Vector
Fields:
  - timestamp, event_type
  - description, emotional_valence
  - participants, location
  - importance_score
  - embedding (for similarity search)
```

#### World State
```
Key: world:economy
Type: Hash
Fields:
  - item_prices (JSON)
  - supply_demand (JSON)
  - trade_volume
  - inflation_rate

Key: world:politics
Type: Hash
Fields:
  - faction_relations (JSON)
  - territory_control (JSON)
  - active_conflicts (JSON)

Key: world:environment
Type: Hash
Fields:
  - current_weather, season
  - natural_events (JSON)
  - resource_availability (JSON)
```

#### Event Queue
```
Key: events:pending
Type: Sorted Set
Score: timestamp
Members: event_id

Key: event:{event_id}
Type: Hash
Fields:
  - type, source, target
  - data (JSON)
  - priority, status
```

#### Relationship Graph
```
Key: relationships:{npc_id}
Type: Hash
Fields:
  - {other_npc_id}: relationship_score (-100 to 100)

Key: relationship:{npc_id}:{other_npc_id}
Type: Hash
Fields:
  - score, type (friend/enemy/neutral)
  - history (JSON array of interactions)
  - last_interaction_timestamp
```

**Caching Strategy:**
- LLM responses cached by prompt hash (TTL: 1 hour)
- NPC decisions cached for similar contexts (TTL: 5 minutes)
- World state cached with invalidation on updates
- Memory retrieval results cached per query

## Data Flow and Communication Protocols

### Event Flow: Player Interacts with NPC

```
1. Player clicks NPC in game
2. rAthena triggers OnTouch event in NPC script
3. NPC script calls Bridge API: POST /ai/player/interaction
4. Bridge Layer:
   - Validates request
   - Enriches with game state context
   - Publishes to DragonflyDB event queue
5. AI Service (Event Coordinator):
   - Consumes event from queue
   - Routes to appropriate NPC Agent
6. NPC Agent (CrewAI):
   - Retrieves NPC state and memories from DragonflyDB
   - Retrieves relevant memories via Memori SDK
   - Constructs prompt with context
   - Calls LLM Provider for decision
   - Updates NPC state and memories
   - Publishes action to action queue
7. Bridge Layer:
   - Consumes action from queue
   - Translates to rAthena script commands
   - Executes via NPC script callback
8. rAthena executes action (dialogue, movement, etc.)
9. Player sees NPC response
```

### Event Flow: Autonomous NPC Decision Cycle

```
1. Timer triggers in AI Service (every 5-30 seconds per NPC)
2. NPC Agent:
   - Retrieves current state and goals
   - Analyzes environment (nearby NPCs, players, events)
   - Retrieves relevant memories
   - Decides next action based on personality and goals
3. Action types:
   - Movement (wander, go to location)
   - Social (talk to another NPC, form relationship)
   - Economic (buy, sell, trade)
   - Combat (attack, defend, flee)
   - Goal pursuit (work towards objective)
4. Action published to queue
5. Bridge Layer executes action in rAthena
6. Result fed back to NPC Agent for learning
```

### Communication Protocols

#### Bridge ↔ AI Service

**Protocol**: REST API over HTTP/HTTPS
**Format**: JSON
**Authentication**: API Key + JWT tokens

**Request Example:**
```json
POST /ai/player/interaction
{
  "npc_id": "npc_12345",
  "player_id": "player_67890",
  "interaction_type": "talk",
  "context": {
    "player_name": "Adventurer",
    "player_level": 50,
    "player_class": "Knight",
    "location": {"map": "prontera", "x": 150, "y": 200},
    "time_of_day": "morning",
    "weather": "sunny"
  }
}
```

**Response Example:**
```json
{
  "action": "dialogue",
  "data": {
    "text": "Greetings, brave Knight! The morning sun brings hope to Prontera.",
    "emotion": "friendly",
    "next_actions": ["offer_quest", "trade", "end_conversation"]
  },
  "npc_state_update": {
    "last_interaction": "2024-01-15T10:30:00Z",
    "relationship_change": {"player_67890": +5}
  }
}
```

#### AI Service ↔ DragonflyDB

**Protocol**: Redis Protocol (RESP)
**Client**: redis-py with async support

**Operations:**
- GET/SET for simple state
- HGET/HSET for structured data
- ZADD/ZRANGE for sorted sets (event queues)
- PUBLISH/SUBSCRIBE for real-time events
- Vector operations for memory similarity search

#### AI Service ↔ LLM Providers

**Protocol**: Provider-specific (HTTP REST, gRPC)
**Format**: JSON
**Rate Limiting**: Token bucket algorithm per provider

## Scalability and Performance Considerations

### Horizontal Scaling

**AI Service Layer:**
- Stateless design allows multiple instances
- Load balancer distributes requests
- Sticky sessions for NPC agents (same NPC → same instance)
- Shared state via DragonflyDB

**DragonflyDB:**
- Single instance for small deployments (< 1000 NPCs)
- Cluster mode for large deployments (> 1000 NPCs)
- Read replicas for read-heavy workloads

**Bridge Layer:**
- Multiple instances behind load balancer
- Connection pooling to DragonflyDB
- Async request handling

### Performance Optimizations

#### LLM Call Reduction
1. **Caching**: Cache similar prompts and responses
2. **Batching**: Batch multiple NPC decisions in single LLM call
3. **Tiering**: Use cheaper/faster models for simple decisions
4. **Lazy Evaluation**: Only call LLM when significant change occurs
5. **Pre-computation**: Pre-generate common responses

#### Decision Cycle Optimization
- **Variable Frequency**: Important NPCs decide more often
- **Event-Driven**: Trigger decisions on significant events, not just timers
- **Proximity-Based**: NPCs near players decide more frequently
- **Sleep Mode**: Inactive NPCs enter low-power mode

#### Memory Management
- **Hierarchical Memory**: Hot/warm/cold tiers
- **Compression**: Compress old memories
- **Pruning**: Remove low-importance memories
- **Summarization**: Summarize old episodic memories

### Estimated Performance

**Assumptions:**
- 1000 autonomous NPCs
- Average 1 decision per NPC per 10 seconds
- Average LLM call: 500ms latency, 1000 tokens

**Calculations:**
- Decisions per second: 1000 / 10 = 100 DPS
- With 50% cache hit rate: 50 LLM calls/second
- With batching (10 NPCs per call): 5 LLM calls/second
- Total latency: ~500ms per decision (acceptable)

**Bottlenecks:**
1. LLM API rate limits (mitigated by caching and batching)
2. DragonflyDB throughput (mitigated by clustering)
3. Bridge Layer throughput (mitigated by horizontal scaling)

## Deployment Architecture

### Development Environment

```
┌─────────────────────────────────────────────────────────────┐
│                    Developer Machine                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   rAthena    │  │  AI Service  │  │ DragonflyDB  │      │
│  │  (Docker)    │  │   (Local)    │  │  (Docker)    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                                               │
│  LLM Provider: Ollama (local) or Azure OpenAI (cloud)       │
└─────────────────────────────────────────────────────────────┘
```

### Production Environment

```
┌─────────────────────────────────────────────────────────────┐
│                      Load Balancer                           │
└────────┬────────────────────────────────────────────────────┘
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼────┐
│rAthena│ │rAthena│  (Game Servers)
│Server1│ │Server2│
└───┬───┘ └──┬────┘
    │        │
    └────┬───┘
         │
    ┌────▼────────────────────────────────────────────────────┐
    │              Bridge Layer (API Gateway)                  │
    │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │
    │  │Instance 1│  │Instance 2│  │Instance 3│             │
    │  └──────────┘  └──────────┘  └──────────┘             │
    └────┬────────────────────────────────────────────────────┘
         │
    ┌────▼────────────────────────────────────────────────────┐
    │           AI Service Layer (Kubernetes)                  │
    │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │
    │  │  Pod 1   │  │  Pod 2   │  │  Pod 3   │             │
    │  │(NPC 1-   │  │(NPC 334- │  │(NPC 667- │             │
    │  │  333)    │  │  666)    │  │  1000)   │             │
    │  └──────────┘  └──────────┘  └──────────┘             │
    └────┬────────────────────────────────────────────────────┘
         │
    ┌────▼────────────────────────────────────────────────────┐
    │         DragonflyDB Cluster (3 nodes)                    │
    │  ┌──────────┐  ┌──────────┐  ┌──────────┐             │
    │  │  Master  │  │ Replica1 │  │ Replica2 │             │
    │  └──────────┘  └──────────┘  └──────────┘             │
    └─────────────────────────────────────────────────────────┘
         │
    ┌────▼────────────────────────────────────────────────────┐
    │              LLM Provider (Cloud)                        │
    │         Azure OpenAI Foundry (Default)                   │
    └─────────────────────────────────────────────────────────┘
```

### Infrastructure Requirements

**Minimum (100 NPCs):**
- rAthena Server: 2 CPU, 4GB RAM
- Bridge Layer: 2 CPU, 2GB RAM
- AI Service: 4 CPU, 8GB RAM
- DragonflyDB: 2 CPU, 4GB RAM
- Total: 10 CPU, 18GB RAM

**Recommended (1000 NPCs):**
- rAthena Servers: 2x (4 CPU, 8GB RAM each)
- Bridge Layer: 3x (2 CPU, 4GB RAM each)
- AI Service: 5x (8 CPU, 16GB RAM each)
- DragonflyDB Cluster: 3x (4 CPU, 8GB RAM each)
- Total: 66 CPU, 136GB RAM

## Technology Stack Summary

### rAthena Layer
- **Language**: C++
- **Build System**: CMake
- **Database**: MySQL/MariaDB
- **Scripting**: Custom rAthena script language

### Bridge Layer
- **Language**: C++ (extension to rAthena web server)
- **HTTP Library**: httplib (already in rAthena)
- **JSON Library**: nlohmann/json (already in rAthena)

### AI Service Layer
- **Language**: Python 3.11+
- **Web Framework**: FastAPI 0.109+
- **Agent Framework**: CrewAI (latest)
- **Memory SDK**: Memori SDK (latest)
- **Async**: asyncio, aiohttp
- **Task Queue**: Celery + Redis (for background tasks)
- **LLM Libraries**:
  - openai (OpenAI, Azure OpenAI)
  - anthropic (Claude)
  - google-generativeai (Gemini)
  - ollama (local models)

### State Management
- **Database**: DragonflyDB (latest)
- **Client**: redis-py with async support
- **Vector Search**: DragonflyDB native vector support

### DevOps
- **Containerization**: Docker, Docker Compose
- **Orchestration**: Kubernetes (production)
- **Monitoring**: Prometheus + Grafana
- **Logging**: ELK Stack (Elasticsearch, Logstash, Kibana)
- **Tracing**: OpenTelemetry

### Development Tools
- **Version Control**: Git
- **CI/CD**: GitHub Actions
- **Testing**: pytest, unittest (Python), Google Test (C++)
- **Code Quality**: pylint, black, mypy (Python), clang-format (C++)

## Next Steps

1. **Phase 1: Foundation** (Weeks 1-2)
   - Set up development environment
   - Implement Bridge Layer basic structure
   - Implement AI Service skeleton with FastAPI
   - Set up DragonflyDB

2. **Phase 2: Core Systems** (Weeks 3-6)
   - Implement LLM Provider abstraction
   - Integrate CrewAI for agent orchestration
   - Integrate Memori SDK for memory management
   - Implement basic NPC consciousness model

3. **Phase 3: World Systems** (Weeks 7-10)
   - Implement economy system
   - Implement politics/faction system
   - Implement environment system
   - Implement quest generation system

4. **Phase 4: Integration & Testing** (Weeks 11-12)
   - End-to-end integration testing
   - Performance testing and optimization
   - Load testing with 100+ NPCs
   - Bug fixes and refinements

5. **Phase 5: Deployment** (Weeks 13-14)
   - Production deployment setup
   - Monitoring and alerting setup
   - Documentation and training
   - Initial world bootstrap

---

**Document Version**: 1.0
**Last Updated**: 2024-01-15
**Author**: AI Architecture Team

