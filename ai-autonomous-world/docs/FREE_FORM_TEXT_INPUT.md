# Free-Form Text Input for Player-NPC Interactions

## Overview

The AI Autonomous World system supports **free-form natural language text input** for player-NPC interactions. Players can type any message to NPCs and receive AI-generated, personality-driven responses.

This document provides a comprehensive guide to the free-form text input feature, including architecture, configuration, usage, and implementation details.

---

## Table of Contents

1. [Current Capabilities](#current-capabilities)
2. [Architecture](#architecture)
3. [Configuration Options](#configuration-options)
4. [Usage Guide](#usage-guide)
5. [API Documentation](#api-documentation)
6. [Implementation Details](#implementation-details)
7. [Troubleshooting](#troubleshooting)

---

## Current Capabilities

### ✅ What's Implemented

The AI service **fully supports** free-form text input with the following features:

- **Natural Language Understanding**: Players can type anything - questions, statements, commands, greetings
- **Context-Aware Responses**: Considers player name, level, class, location, time of day, weather
- **Memory Integration**: Remembers past conversations with each player
- **Personality-Driven Dialogue**: Each NPC has unique personality traits (Big Five model)
- **Multi-Agent Coordination**: Memory Agent + Dialogue Agent + World Agent work together
- **Rate Limiting**: Prevents spam and abuse
- **Profanity Filtering**: Optional content filtering (configurable)
- **Fallback Mechanisms**: Graceful degradation when AI service is unavailable

### ⚠️ Integration Status

| Component | Status | Notes |
|-----------|--------|-------|
| **AI Service Backend** | ✅ Fully Implemented | Accepts any player message |
| **LLM Response Generation** | ✅ Fully Implemented | Personality-driven, context-aware |
| **Memory System** | ✅ Fully Implemented | Per-player conversation history |
| **Bridge Layer API** | ✅ Ready | Endpoint exists, forwards requests |
| **Chat Command Interface** | ✅ Implemented | Python API + rAthena script template |
| **Client Text Input UI** | ❌ Not Implemented | Requires client modification |

---

## Architecture

### System Flow

```
Player Types Message
       ↓
Chat Command (@npc <npc_id> <message>)
       ↓
rAthena Script Handler (ai_chat_handler.txt)
       ↓
Bridge Layer (HTTP POST /ai/chat/command)
       ↓
AI Service Chat Command Router
       ↓
Player Interaction Endpoint
       ↓
Agent Orchestrator
       ↓
┌──────────────┬──────────────┬──────────────┐
│ Memory Agent │ Dialogue Agent│ World Agent  │
│ (Retrieve)   │ (Generate)    │ (Analyze)    │
└──────────────┴──────────────┴──────────────┘
       ↓
LLM (Azure OpenAI / OpenAI / Anthropic / Google)
       ↓
AI-Generated Response
       ↓
Return to Player
```

### Key Components

1. **Chat Command Handler** (`ai_chat_handler.txt`)
   - rAthena NPC script
   - Binds `@npc` command
   - Validates input and enforces rate limiting
   - Builds JSON request

2. **Chat Command Router** (`routers/chat_command.py`)
   - FastAPI endpoint: `POST /ai/chat/command`
   - Rate limiting and validation
   - Forwards to player interaction handler

3. **Player Interaction Handler** (`routers/player.py`)
   - Main interaction processing
   - Orchestrates multiple agents
   - Returns structured response

4. **Agent Orchestrator** (`agents/orchestrator.py`)
   - Coordinates Memory, Dialogue, and World agents
   - Manages conversation flow
   - Stores interaction history

5. **Dialogue Agent** (`agents/dialogue_agent.py`)
   - Generates personality-driven responses
   - Uses LLM for natural language generation
   - Considers context and memory

---

## Configuration Options

All configuration options are in `ai-service/config.py`:

### Free-Form Text Input Settings

```python
# Enable/disable free-form text input
freeform_text_enabled: bool = True

# Input mode: "chat_command", "client_ui", "web_interface"
freeform_text_mode: str = "chat_command"

# Maximum message length (characters)
freeform_text_max_length: int = 500

# Enable rate limiting
freeform_text_rate_limit_enabled: bool = True

# Messages per minute
freeform_text_rate_limit_messages: int = 10

# Enable profanity filter
freeform_text_profanity_filter_enabled: bool = False

# Response timeout (seconds)
freeform_text_response_timeout: int = 30

# Fallback mode: "show_error", "use_buttons", "use_cached"
freeform_text_fallback_mode: str = "show_error"
```

### Chat Command Settings

```python
# Enable chat command interface
chat_command_enabled: bool = True

# Command prefix (default: @npc)
chat_command_prefix: str = "@npc"

# Fallback mode when AI unavailable
chat_command_fallback_mode: str = "show_error"

# Maximum message length
chat_command_max_length: int = 500

# Cooldown between messages (seconds)
chat_command_cooldown: int = 2

# Require proximity to NPC
chat_command_require_npc_proximity: bool = True

# Proximity range (cells)
chat_command_proximity_range: int = 5

# Log all interactions
chat_command_log_all_interactions: bool = True
```

### Environment Variables

You can override settings using environment variables:

```bash
export FREEFORM_TEXT_ENABLED=true
export CHAT_COMMAND_PREFIX="@talk"
export CHAT_COMMAND_COOLDOWN=3
export FREEFORM_TEXT_MAX_LENGTH=1000
```

---

## Usage Guide

### For Players

#### Using Chat Commands

1. **Find an AI-enabled NPC** (look for NPCs with "AI" in their name or description)

2. **Type the chat command**:
   ```
   @npc <npc_id> <your message>
   ```

3. **Examples**:
   ```
   @npc merchant_001 Tell me about the ancient ruins
   @npc guard_prontera What's happening in the city today?
   @npc blacksmith_001 Can you craft me a sword?
   @npc villager_042 Have you seen any monsters nearby?
   ```

4. **Receive AI-generated response**:
   ```
   [Merchant Marcus] Ah, the ancient ruins! I've heard tales of great 
   treasures hidden within, but also of terrible dangers. Many adventurers 
   have ventured there, but few return...
   ```

#### Tips for Better Interactions

- **Be specific**: "Tell me about the haunted forest" is better than "Tell me something"
- **Ask questions**: NPCs respond well to questions
- **Reference context**: "What do you think about the recent monster attacks?"
- **Build relationships**: NPCs remember past conversations

---


## API Documentation

### POST /ai/chat/command

Handle chat command interaction with free-form text input.

**Request Body**:
```json
{
  "player_id": "12345",
  "npc_id": "merchant_001",
  "message": "Tell me about the ancient ruins",
  "player_name": "Adventurer",
  "player_level": 50,
  "player_class": "Knight",
  "map_name": "prontera",
  "x": 150,
  "y": 200,
  "time_of_day": "morning"
}
```

**Response**:
```json
{
  "success": true,
  "npc_response": "Ah, the ancient ruins! I've heard tales of great treasures...",
  "emotion": "curious",
  "error": null
}
```

**Error Response**:
```json
{
  "success": false,
  "npc_response": null,
  "emotion": null,
  "error": "Message too long. Maximum 500 characters."
}
```

### GET /ai/chat/status

Get chat command interface status.

**Response**:
```json
{
  "enabled": true,
  "prefix": "@npc",
  "max_length": 500,
  "cooldown": 2,
  "rate_limiting": true
}
```

---

## Implementation Details

### For Developers

#### Adding Chat Command to rAthena

1. **Copy the chat handler script**:
   ```bash
   cp npc/custom/ai-world/ai_chat_handler.txt npc/scripts_custom.conf
   ```

2. **Add to scripts_custom.conf**:
   ```
   npc: npc/custom/ai-world/ai_chat_handler.txt
   ```

3. **Reload scripts**:
   ```
   @reloadscript
   ```

---

## Troubleshooting

### Common Issues

#### 1. Chat Command Not Working

**Symptom**: `@npc` command not recognized

**Solutions**:
- Check if script is loaded: `@reloadscript`
- Verify script is in `scripts_custom.conf`
- Check server console for script errors

#### 2. No Response from AI

**Symptom**: Message sent but no response received

**Solutions**:
- Check AI service is running: `curl http://localhost:8000/health`
- Check Bridge Layer is running: `curl http://localhost:8888/health`
- Check logs: `tail -f logs/ai-service.log`

---

**Last Updated**: 2025-11-06  
**Version**: 1.0.0
