# AI-Enabled NPC Scripts

This directory contains example NPC scripts that demonstrate integration with the AI Autonomous World System.

## Overview

AI-enabled NPCs use the Bridge Layer to communicate with the AI Service, enabling:
- Dynamic dialogue generation using LLMs
- Personality-driven behavior
- Memory of past interactions
- Adaptive responses to world state
- Emergent behavior patterns

## Example NPCs

### `ai_npc_example.txt` - AI Merchant

**Location**: Prontera (150, 180)  
**NPC ID**: `ai_merchant_001`  
**Personality**: Lawful Good, friendly merchant

**Features**:
- AI-generated dialogue
- Dynamic conversation based on player context
- Personality traits (Big Five model)
- Integration with world state

**Interactions**:
- **Talk**: Engage in AI-driven conversation
- **Trade**: Access dynamic shop (future)
- **Quest**: Receive AI-generated quests (future)

## NPC Registration Flow

1. **OnInit Event**: NPC registers with AI Service on server start
2. **Registration Data**: Sends NPC ID, name, class, level, position, personality
3. **AI Service**: Creates agent and stores state in DragonflyDB
4. **Response**: Returns agent ID for future interactions

## Player Interaction Flow

1. **Player Clicks NPC**: Triggers script execution
2. **Build Context**: Gather player info, location, time, weather
3. **Call Bridge Layer**: Send interaction request to `/ai/player/interaction`
4. **AI Processing**: LLM generates contextual response
5. **Display Response**: Show AI-generated dialogue to player

## Integration Requirements

### Bridge Layer (C++)
- `ai_bridge_controller.cpp` - HTTP client for AI Service
- Endpoints registered in `web.cpp`
- Running on port 8888

### AI Service (Python)
- FastAPI application on port 8000
- DragonflyDB on port 6379
- LLM provider configured (OpenAI/Anthropic/Google)

### rAthena Script Support
Current implementation uses fallback dialogue. Full integration requires:
- HTTP request support in rAthena scripting
- JSON parsing capabilities
- Async request handling (optional)

## Creating New AI NPCs

### Basic Template

```c
prontera,X,Y,DIR	script	NPC Name#ai_id	SPRITE_ID,{
	// Get player context
	.@player_id = getcharid(0);
	.@player_name$ = strcharinfo(0);
	
	// Build interaction request
	.@npc_id$ = "unique_npc_id";
	.@interaction_type$ = "talk";
	
	// Call AI Service (via Bridge Layer)
	// .@response$ = httppost("http://localhost:8888/ai/player/interaction", .@json$);
	
	// Display response
	mes "[NPC Name]";
	mes "AI-generated dialogue here";
	close;

OnInit:
	// Register with AI Service
	.npc_id$ = "unique_npc_id";
	.npc_name$ = "NPC Name";
	.npc_class$ = "class_type";
	
	// Build registration JSON and send
	// httppost("http://localhost:8888/ai/npc/register", .@reg_json$);
	end;
}
```

### Personality Configuration

Use Big Five personality traits (0.0 to 1.0):
- **Openness**: Creativity, curiosity, open to new experiences
- **Conscientiousness**: Organization, dependability, discipline
- **Extraversion**: Sociability, assertiveness, energy
- **Agreeableness**: Compassion, cooperation, trust
- **Neuroticism**: Emotional stability, anxiety, moodiness

**Moral Alignment**: lawful_good, neutral_good, chaotic_good, lawful_neutral, true_neutral, chaotic_neutral, lawful_evil, neutral_evil, chaotic_evil

### Event Types

NPCs can send events to the AI Service:
- `combat` - Combat-related events
- `social` - Social interactions
- `economic` - Trade, shop transactions
- `environmental` - Weather, time changes
- `political` - Faction events

## Testing

### 1. Start Services

```bash
# Start DragonflyDB
dragonfly --port 6379

# Start AI Service
cd ai-autonomous-world
source venv/bin/activate
python ai-service/main.py

# Start rAthena (with Bridge Layer compiled)
./map-server
./char-server
./login-server
```

### 2. Load NPC Script

Add to `npc/scripts_custom.conf`:
```
npc: npc/custom/ai-world/ai_npc_example.txt
```

Reload scripts:
```
@reloadscript
```

### 3. Test Interaction

1. Go to Prontera (150, 180)
2. Click on "AI Merchant"
3. Select interaction type
4. Observe AI-generated responses

## Troubleshooting

### NPC Not Registering
- Check AI Service logs: `ai-service/logs/ai-service.log`
- Verify Bridge Layer is compiled and loaded
- Check DragonflyDB connection

### No AI Responses
- Verify LLM provider API key is configured
- Check network connectivity to AI Service
- Review FastAPI logs for errors

### Script Errors
- Check rAthena map-server console for script errors
- Verify JSON syntax in request building
- Test Bridge Layer endpoints with curl

## Next Steps

1. Implement HTTP request support in rAthena scripting
2. Add JSON parsing utilities
3. Create more example NPCs with different personalities
4. Implement dynamic quest generation
5. Add economic system integration
6. Create faction-aware NPCs

## Resources

- [Architecture Documentation](../../ai-autonomous-world/docs/ARCHITECTURE.md)
- [API Documentation](../../ai-autonomous-world/docs/API.md)
- [Bridge Layer Implementation](../../ai-autonomous-world/BRIDGE_LAYER_IMPLEMENTATION.md)

