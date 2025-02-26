# AI Beggar System

This document provides a comprehensive guide to the AI Beggar system, which creates a mysterious Nameless Beggar character that roams cities and interacts with players.

## Overview

The AI Beggar system introduces a unique NPC character known as "The Nameless One" or simply "The Beggar" who wanders between major cities, asking for food from players. This character serves as a mysterious master figure to all the AI Legends, creating a cohesive narrative that ties the AI characters together.

Key features of the Beggar system include:
- Daily appearances in different cities
- Requests for food items from players
- Rewards players with seemingly "useless" items that have hidden value
- Triggers a special event when players collect enough fragments
- Rewards players who consistently feed the Beggar for consecutive days
- Uses advanced AI to create natural, contextual conversations

## The Nameless Beggar

### Character Profile

- **Name:** Unknown (referred to as "The Nameless One" or "Master")
- **Appearance:** Elderly man in tattered robes, carries a wooden staff
- **Behavior:** Wanders between main cities, asking for food from random players
- **Personality:** Mysterious, cryptic, seemingly omniscient
- **Background:** A being of immense power who chooses to live as a beggar to test the kindness and worthiness of others

### Daily Routine

1. **Appearance:** The Beggar appears in one city each day, following either a sequential or random pattern
2. **Movement:** Wanders around the city, approaching random players
3. **Interaction:** Initiates trade with players, asking for food
4. **Reward:** Gives an "Ancient Scroll Fragment" to players who provide food
5. **Disappearance:** Leaves the city after a set duration or at a specific time

## Player Interaction Mechanics

### Food Requests

The Beggar approaches random players in the city and initiates a trade request. If the player accepts, the Beggar will ask for food items. Any food item in the game can be given, and the Beggar will accept it.

Upon receiving food, the Beggar will:
1. Thank the player in his cryptic manner
2. Initiate another trade to give the player an "Ancient Scroll Fragment"
3. Sometimes share cryptic hints about his true nature or the AI Legends

### Ancient Scroll Fragments

These fragments appear to be useless items with the following properties:
- 0 weight
- Cannot be traded to other players
- Cannot be dropped
- Can be stored in storage or cart
- Have a cryptic description

However, these fragments have a hidden purpose. When a player collects 100 fragments, they trigger a special event.

### Feeding Streak

Players who feed the Beggar consistently can earn additional rewards:
1. The player must feed the Beggar at least 3 times per day
2. This must be done for 14 consecutive days
3. If a day is missed, the streak resets (configurable)
4. Upon completing the streak, the player receives a fragment of a special passive skill

### Special Event

When a player collects 100 Ancient Scroll Fragments, a special event is triggered:
1. All AI Legends appear at a designated location, forming multiple parties
2. A grand PK fight is initiated between all AI Legends and their master (the Nameless Beggar)
3. The AI Legends reveal that they are all disciples of the Nameless Beggar
4. The fight is intense and lasts randomly between 8-18 minutes
5. Despite the AI Legends' coordinated efforts, the master wins due to perfect skill combinations from various job classes
6. The player who triggered the event receives a fragment of a special passive skill regardless of the outcome
7. The AI Legends engage in conversation with each other, revealing their relationships and personalities

During this event, the AI Legends demonstrate remarkable teamwork despite their verbal conflicts, showing that they have trained together under the Beggar's tutelage.

After the battle, the AI Legends continue to chat in the global channel for 8-10 minutes, discussing the battle, roasting or supporting each other, and eventually saying goodbye before returning to their individual routines.

## Configuration

The AI Beggar system is highly configurable through the `ai_beggar.conf` file. Key configuration options include:

### Global Settings

```
ai_beggar_enabled: true                 // Enable or disable the Nameless Beggar globally
ai_beggar_provider: "azure_openai"      // AI provider to use (azure_openai, openai, deepseek)
ai_beggar_model: "gpt-4o"               // Model to use for the Nameless Beggar
```

### Appearance and Movement Settings

```
ai_beggar_appearance_chance: 1.0        // Chance of appearing each day (0.0-1.0)
ai_beggar_appearance_time: "06:00"      // Time of day when beggar appears
ai_beggar_disappearance_time: "22:00"   // Time of day when beggar disappears
ai_beggar_stay_duration: 3600           // Duration to stay in one city (in seconds)
ai_beggar_cities: ["prontera", "geffen", "payon", "morocc", "alberta", "izlude", "aldebaran", "comodo", "yuno"] // Cities to visit
ai_beggar_visit_order: "sequential"     // Visit order (sequential, random)
```

### Interaction Settings

```
ai_beggar_max_daily_interactions: 10    // Maximum number of players to interact with per city
ai_beggar_interaction_cooldown: 86400   // Cooldown between interactions with the same player (in seconds)
ai_beggar_food_request_chance: 1.0      // Chance to request food from a player (0.0-1.0)
ai_beggar_food_item_ids: [512, 513, 514, ...] // Food item IDs
```

### Reward Settings

```
ai_beggar_reward_item_id: 25000         // ID of the "Ancient Scroll Fragment" item to give as reward
ai_beggar_reward_amount: 1              // Amount of reward items to give
ai_beggar_reward_weight: 0              // Weight of reward item
ai_beggar_reward_tradable: false        // Whether reward item is tradable
```

### Feeding Streak Settings

```
ai_beggar_required_feeding_days: 14     // Number of consecutive days required to feed the beggar
ai_beggar_required_feeding_count: 3     // Number of food items required per day
ai_beggar_streak_reset_on_miss: true    // Whether to reset streak if player misses a day
ai_beggar_streak_reward_skill_id: 8100  // ID of the skill fragment to reward after completing streak
```

### Special Event Settings

```
ai_beggar_fragment_threshold: 100       // Number of fragments needed to trigger special event
ai_beggar_event_map: "prontera"         // Map where special event occurs
ai_beggar_event_x: 150                  // X coordinate for special event
ai_beggar_event_y: 150                  // Y coordinate for special event
ai_beggar_event_duration: 1800          // Duration of special event (in seconds)
```

### Memory and Context Settings

```
ai_beggar_use_langchain: true           // Use LangChain for memory management
ai_beggar_memory_vector_store: "chroma" // Vector store to use for memory
ai_beggar_memory_embedding_model: "text-embedding-3-small" // Embedding model to use
```

## Database Schema

The AI Beggar system uses several database tables to store information:

- `ai_beggar_appearances`: Records of Beggar appearances in cities
- `ai_beggar_interactions`: Records of player interactions with the Beggar
- `ai_beggar_feeding_streaks`: Tracking of player feeding streaks
- `ai_beggar_fragments`: Tracking of player fragment collections
- `ai_beggar_events`: Records of special events triggered by players
- `ai_beggar_memory`: Memory entries for the Beggar using LangChain
- `ai_beggar_conversations`: Conversation history between the Beggar and players
- `ai_beggar_logs`: Logs of Beggar activities
- `ai_beggar_statistics`: Statistics about Beggar activities
- `ai_beggar_skill_fragments`: Skill fragments given to players

## Implementation Details

### AI Integration

The Beggar uses advanced language models (Azure OpenAI, OpenAI, or DeepSeek) to generate realistic and contextually appropriate responses. The system includes:

- **Persistent Memory**: Using LangChain for context management, the Beggar remembers past interactions with players
- **Contextual Responses**: Responses are generated based on the current situation, player history, and conversation context
- **Fallback Mechanism**: Local fallback provider for offline operation

### Map Server Integration

The Beggar system integrates with the map server to:

- Spawn and despawn the Beggar character
- Handle player movement and proximity detection
- Process trade requests and item exchanges
- Manage special event spawning and coordination

### Database Integration

The system uses the database to:

- Track Beggar appearances and movements
- Record player interactions and rewards
- Manage feeding streaks and fragment collections
- Store memory and conversation history

## Player Experience

From the player's perspective, the Beggar system creates a mysterious and intriguing experience:

1. **Discovery**: Players encounter a strange old man asking for food
2. **Curiosity**: The cryptic responses and seemingly useless rewards create curiosity
3. **Collection**: Players begin collecting the fragments, unsure of their purpose
4. **Consistency**: Some players feed the Beggar daily, developing a routine
5. **Revelation**: The special event reveals the connection between the Beggar and the AI Legends
6. **Reward**: Players receive unique skill fragments that enhance their characters

The system creates a sense of mystery and discovery, encouraging players to interact with the Beggar and, by extension, the AI Legends.

## Narrative Integration

The Beggar serves as a narrative lynchpin for the entire AI Legends system:

1. **Master Figure**: All AI Legends are revealed to be disciples of the Nameless Beggar
2. **Testing Mechanism**: The Beggar tests players' kindness and persistence
3. **Mysterious Past**: The Beggar's true identity and powers remain mysterious
4. **Unifying Element**: The Beggar connects all the disparate AI Legends into a cohesive story

This narrative integration creates a more immersive and coherent world, where the AI characters have relationships and histories that players can discover over time.

## Technical Considerations

### Performance Optimization

- **Caching**: Cache AI responses to reduce API calls
- **Asynchronous Processing**: Process AI requests asynchronously
- **Local Fallback**: Use local provider when API is unavailable
- **Memory Management**: Efficiently manage memory usage
- **Database Optimization**: Optimize database queries

### Security Considerations

- **API Key Encryption**: Encrypt API keys in memory and database
- **Input Sanitization**: Sanitize input before sending to AI
- **Output Filtering**: Filter output for inappropriate content
- **Rate Limiting**: Limit the number of requests per user
- **Access Control**: Control access to AI functionality

## Troubleshooting

### Common Issues

1. **Beggar Not Appearing**
   - Check if `ai_beggar_enabled` is set to `true` in `ai_beggar.conf`
   - Verify that the appearance time and chance are properly configured
   - Check the logs for any errors

2. **AI Responses Are Slow**
   - Consider using a faster AI provider
   - Reduce the context window size
   - Enable caching for faster responses

3. **Memory Issues**
   - Check if LangChain is properly configured
   - Verify that the vector store is accessible
   - Adjust memory parameters for better performance

4. **Database Issues**
   - Check database connection settings
   - Verify that all required tables exist
   - Check for database errors in the logs

### Logs and Debugging

The system logs various events to help with debugging:

- **Beggar Logs**: Stored in the `ai_beggar_logs` table
- **Server Logs**: Check the server logs for AI-related messages
- **Debug Mode**: Enable debug mode for more detailed logging

## Conclusion

The AI Beggar system creates a unique and engaging gameplay experience by introducing a mysterious character that connects with players on a personal level and ties together the narrative of the AI Legends. Through careful configuration and customization, server administrators can create a balanced and enjoyable experience that keeps players engaged and excited to explore the world and interact with this enigmatic figure.