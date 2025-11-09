# Interactive AI NPC Conversation System

## 🎮 Overview

The Interactive AI NPC system allows players to have natural, context-aware conversations with NPCs directly in the game's dialogue window. No client modifications required!

## ✨ Features

### 1. **Conversational Dialogue**
- NPCs remember the conversation context
- Each conversation is tracked separately per player
- Conversation history expires after 10 minutes of inactivity

### 2. **Two Ways to Respond**
- **Quick Responses**: Pre-defined personality-specific options
- **Custom Messages**: Type your own message (up to 255 characters)

### 3. **Real-time Feel**
- Fast AI responses
- All conversation stays in the dialogue bubble box
- Smooth, natural flow

### 4. **Personality-Driven**
- Each NPC has unique quick response options
- Responses reflect NPC personality and background

## 🎯 How to Use

### For Players:

1. **Click on an AI NPC** (e.g., Lyra the Explorer or Guard Thorne)
2. **Read the AI greeting** - The NPC will greet you based on their personality
3. **Choose your response**:
   - Select a quick response option (1-3)
   - Choose "Type my own message..." to write custom text
   - Choose "Goodbye" to end the conversation
4. **Continue the conversation** - The NPC remembers what you talked about!
5. **Close the dialogue** when you're done

### Example Conversation Flow:

```
[Lyra the Explorer]
Oh! A fellow adventurer! I've just returned from
the most AMAZING expedition to the ancient ruins!

> Tell me about your adventures!
> Any treasure hunting tips?
> Where should I explore next?
> Type my own message...
> Goodbye

[You select: "Tell me about your adventures!"]

[Lyra the Explorer]
*eyes light up with excitement*
Oh, where do I even begin! Just last week, I discovered
a hidden chamber in the Pyramid...
[AI-generated response based on personality]

> Tell me more about the pyramid!
> Any treasure hunting tips?
> Where should I explore next?
> Type my own message...
> Goodbye
```

## 🤖 Available Interactive NPCs

### 1. **Lyra the Explorer** (prontera 155, 185)
- **Personality**: Cheerful, adventurous, curious
- **Quick Responses**:
  - "Tell me about your adventures!"
  - "Any treasure hunting tips?"
  - "Where should I explore next?"

### 2. **Guard Thorne** (prontera 145, 175)
- **Personality**: Grumpy, suspicious, dutiful
- **Quick Responses**:
  - "Is the city safe?"
  - "Any suspicious activity lately?"
  - "What's your duty here?"

## 🔧 Technical Details

### Architecture

```
Player clicks NPC
    ↓
NPC Script sends request to AI Service
    ↓
AI Service retrieves conversation history from DragonflyDB
    ↓
AI generates contextual response
    ↓
AI Service saves updated conversation history
    ↓
Response displayed in dialogue window
```

### API Endpoint

**POST** `http://192.168.0.100:8000/ai/player/chat`

**Request:**
```json
{
  "npc_id": "ai_explorer_002",
  "player_id": "150000",
  "player_name": "TestAdmin",
  "message": "Tell me about your adventures!"
}
```

**Response:**
```
"*eyes light up with excitement* Oh, where do I even begin! Just last week..."
```

### Conversation History

- **Storage**: DragonflyDB (Redis-compatible)
- **Key Format**: `conversation:{npc_id}:{player_id}`
- **TTL**: 600 seconds (10 minutes)
- **Max Messages**: 50 (automatically trimmed)
- **Retrieval Limit**: Last 20 messages per request

### NPC Script Structure

```javascript
prontera,155,185,4	script	Lyra the Explorer#ai002	4_F_NOVICE,{
    // Get player info
    .@player_id = getcharid(0);
    .@player_name$ = strcharinfo(0);
    
    // Initial greeting (empty message)
    .@greeting$ = callAIService("", "");
    
    // Conversation loop
    while(1) {
        // Show menu with quick responses + custom input
        .@choice = select(...);
        
        // Get message based on choice
        if (custom input) {
            input .@message$;
        }
        
        // Send to AI and display response
        .@response$ = callAIService(.@message$);
        mes .@response$;
    }
}
```

## 📝 Adding New Interactive NPCs

To create a new interactive NPC:

1. **Copy the template** from `prontera_ai_npcs_interactive.txt`
2. **Customize**:
   - NPC ID, name, sprite, position
   - Personality traits (openness, conscientiousness, etc.)
   - Quick response options (3-5 personality-specific choices)
   - Greeting fallback text
3. **Register the NPC** in `OnInit` section
4. **Add to** `npc/scripts_custom.conf`
5. **Reload** map server: `@reloadscript` or restart

## 🎨 Customization Tips

### Quick Response Options

Make them personality-specific:
- **Explorer**: Adventure, discovery, exploration
- **Guard**: Safety, duty, law enforcement
- **Merchant**: Trade, prices, business
- **Scholar**: Knowledge, research, history
- **Healer**: Health, peace, helping others

### Personality Traits

Adjust Big Five traits (0.0 to 1.0):
- **Openness**: Curiosity, creativity
- **Conscientiousness**: Organization, duty
- **Extraversion**: Sociability, energy
- **Agreeableness**: Friendliness, cooperation
- **Neuroticism**: Anxiety, moodiness

## 🐛 Troubleshooting

### NPC shows "I'm still getting to know this world"
- NPC not registered with AI service
- Check AI service is running: `curl http://192.168.0.100:8000/health`
- Reload NPCs: `@reloadscript`

### NPC shows "..."
- AI service error or timeout
- Check AI service logs
- Verify DragonflyDB is running

### Conversation doesn't remember context
- DragonflyDB not running
- Conversation history expired (10 min TTL)
- Check database connection

## 🚀 Future Enhancements

- [ ] Voice tone indicators (whisper, shout, etc.)
- [ ] Emotion animations
- [ ] Quest integration
- [ ] Relationship system
- [ ] Group conversations
- [ ] NPC-to-NPC interactions

