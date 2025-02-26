# AI Legends System

This document provides a comprehensive guide to the AI Legends system, which creates intelligent AI-controlled characters that roam the world and interact with players.

## Overview

The AI Legends system introduces powerful AI-controlled characters representing each final job class in Ragnarok Online. These characters have unique personalities, backstories, and behaviors, and can interact with players in various ways, including conversations, PvP challenges, and party assistance.

Each AI Legend:
- Represents a final job class at maximum level
- Has a unique MBTI personality that influences behavior
- Roams the world, never staying in one place for too long
- Can appear at MVP battles to assist weaker parties
- May challenge players of the same class to PvP
- Can teach secret skills to players who meet specific conditions
- Has a backstory that is revealed piece by piece through interactions
- May give rare rewards to players who meet specific criteria

## Key Features

### Dynamic AI-Powered Characters

AI Legends use advanced language models (Azure OpenAI, OpenAI, or DeepSeek) to generate realistic and contextually appropriate responses and behaviors. Each character has:

- **Persistent Memory**: Using LangChain for context management, AI Legends remember past interactions with players
- **Unique Personality**: MBTI personality types that are opposite to what's expected for their class
- **Adaptive Behavior**: Responses and actions that evolve based on player interactions
- **Progressive Growth**: Characters develop and change over time based on their experiences

### Balanced Combat Abilities

AI Legends are designed to be challenging but not impossible to defeat:

- Always 8% stronger than the current strongest player on the server
- Use optimal equipment, skills, and strategies based on their class
- Can adapt to different combat situations
- Never take loot from defeated monsters

### Social Interactions

AI Legends can engage in various social interactions:

- **Conversations**: Chat with players using natural language
- **Party Assistance**: Join the weakest party in MVP battles
- **PvP Challenges**: Challenge players of the same class to duels
- **Backstory Revelation**: Share pieces of their backstory through interactions
- **Gift Giving**: Provide rare items to players who meet specific criteria

### Secret Skills System

Each AI Legend can teach a unique secret skill to players:

- Skills are only available through special quests
- Quests are triggered by meeting specific conditions
- Conditions often involve unconventional gameplay
- Skills provide unique abilities not available through normal progression

## Configuration

The AI Legends system is highly configurable through the `ai_legends.conf` file. Key configuration options include:

### Global Settings

```
ai_legends_enabled: true                // Enable or disable AI Legends globally
ai_legends_provider: "azure_openai"     // AI provider to use (azure_openai, openai, deepseek)
ai_legends_model: "gpt-4o"              // Model to use for AI Legends
ai_legends_power_advantage: 8           // Percentage stronger than the strongest player (%)
```

### Movement Settings

```
ai_legends_roam_enabled: true           // Enable roaming behavior
ai_legends_max_stay_time: 300           // Maximum time to stay in one place (in seconds)
ai_legends_min_stay_time: 60            // Minimum time to stay in one place (in seconds)
```

### Interaction Settings

```
ai_legends_interaction_cooldown: 60     // Cooldown between interactions with the same player (in seconds)
ai_legends_max_daily_pvp: 1             // Maximum daily PvP challenges per legend
ai_legends_gift_chance: 0.05            // Chance of giving a gift during interaction (0.0-1.0)
```

### Memory Settings

```
ai_legends_use_langchain: true          // Use LangChain for memory management
ai_legends_memory_vector_store: "chroma" // Vector store to use for memory
ai_legends_memory_embedding_model: "text-embedding-3-small" // Embedding model to use
```

### Individual Character Settings

Each AI Legend has its own configuration section that can override global settings:

```
dragon_knight: {
    enabled: true                       // Enable this legend
    name: "Siegfried Dragonheart"       // Character name
    class_id: 4060                      // Class ID (Dragon Knight)
    base_level: 200                     // Base level
    job_level: 70                       // Job level
    mbti: "INTJ"                        // MBTI personality type
    appearance_maps: ["gef_fild10", "gl_cas02", "abyss_03"] // Maps where this legend can appear
    preferred_mvps: ["EDDGA", "DRAKE", "DETALE"] // Preferred MVP monsters
    backstory_pieces: 7                 // Number of backstory pieces to collect
    secret_skill_id: 8001               // ID of the secret skill this legend teaches
    secret_skill_name: "Dragon's Breath" // Name of the secret skill
    secret_skill_condition: "Defeat 10 dragon-type monsters without using any potions" // Condition to learn the skill
    custom_script: "npc/custom/legends/dragon_knight.txt" // Custom script for this legend
}
```

## Database Schema

The AI Legends system uses several database tables to store information:

- `ai_legends`: Basic information about each AI Legend
- `ai_legend_equipment`: Equipment used by AI Legends
- `ai_legend_skills`: Skills used by AI Legends
- `ai_legend_backstory`: Backstory pieces for each AI Legend
- `ai_legend_secret_skills`: Secret skills taught by AI Legends
- `ai_legend_interactions`: Record of interactions between AI Legends and players
- `ai_legend_relationships`: Relationship status between AI Legends and players
- `ai_legend_memory`: Memory entries for AI Legends using LangChain
- `ai_legend_conversations`: Conversation history between AI Legends and players
- `ai_legend_pvp_history`: Record of PvP battles between AI Legends and players
- `ai_legend_party_history`: Record of parties joined by AI Legends
- `ai_legend_gifts`: Gifts given by AI Legends to players
- `player_secret_skills`: Secret skills learned by players
- `secret_skill_quests`: Quests for learning secret skills
- `ai_legend_logs`: Logs of AI Legend activities
- `ai_legend_statistics`: Statistics about AI Legend activities

## Player Interaction Guide

### Meeting AI Legends

AI Legends can be encountered in various ways:

1. **Random Encounters**: They roam the world and may be encountered anywhere
2. **MVP Battles**: They often appear during MVP battles to assist weaker parties
3. **PvP Challenges**: They may challenge players of the same class to duels
4. **Special Events**: They may appear during special server events

### Conversation Tips

When conversing with AI Legends:

1. **Be Respectful**: AI Legends respond better to respectful communication
2. **Ask About Their Past**: Inquire about their backstory to learn more
3. **Show Interest**: Show interest in their class, abilities, and experiences
4. **Seek Advice**: Ask for advice related to their class or specialization
5. **Share Experiences**: Share your own experiences to build rapport

### Learning Secret Skills

To learn a secret skill from an AI Legend:

1. **Meet the Legend**: You must first encounter the AI Legend
2. **Build Relationship**: Develop a positive relationship through interactions
3. **Discover the Condition**: Learn the condition for the secret skill quest
4. **Complete the Condition**: Fulfill the specific condition for the skill
5. **Receive the Quest**: The AI Legend will offer the secret skill quest
6. **Complete the Quest**: Follow the quest steps to learn the skill

### PvP Challenges

When challenged to PvP by an AI Legend:

1. **Accept the Challenge**: You can accept or decline the 1v1 duel challenge
2. **Prepare for Battle**: The AI Legend will be 8% stronger than the strongest player in a direct 1v1 duel
3. **Use Strategy**: AI Legends have optimal builds but can be defeated with the right strategy
4. **Learn from Defeat**: If defeated in the duel, you can learn from the experience
5. **Earn Respect**: Performing well in PvP can earn the AI Legend's respect

### MVP Assistance

When an AI Legend joins your party for an MVP battle:

1. **Coordinate Strategy**: The AI Legend will adapt to your party's strategy
2. **Follow Their Lead**: They often have insights about the MVP's weaknesses
3. **Protect Weaker Members**: They prioritize protecting weaker party members
4. **No Loot Competition**: They never take loot from defeated monsters
5. **Express Gratitude**: Thanking them can improve your relationship

## AI Legend Characters

The system includes AI Legend characters for all final job classes:

### Swordsman Branch
- **Dragon Knight**: Siegfried Dragonheart (INTJ)
- **Imperial Guard**: Aegis Shieldwall (ISFJ)
- **Rebellion**: Maverick Gunsmoke (ISTP)

### Mage Branch
- **Archmage**: Elara Stormweaver (INTP)
- **Elemental Master**: Talos Elementis (ENFJ)
- **Chronomancer**: Tempus Clockwork (INTP)

### Archer Branch
- **Hawkeye**: Artemis Hawkgaze (ISTP)
- **Troubadour**: Lyric Songweaver (ESFP)
- **Trouvere**: Aria Melodica (ENFP)
- **Wind Hawk**: Zephyr Skyrider (ESTP)

### Merchant Branch
- **Bionic**: Nexus Gearhart (ISTJ)
- **Biochemist**: Malachite Vialborn (ENTP)
- **Meister**: Forge Mastercraft (ISTJ)

### Thief Branch
- **Abyss Chaser**: Nyx Shadowstep (INFJ)
- **Phantom Dancer**: Mirage Duskwalker (ISFP)
- **Night Watcher**: Vigil Duskgaze (INTJ)

### Acolyte Branch
- **Saint**: Seraphina Lightbringer (ESFJ)
- **Inquisitor**: Thorne Judgement (ENTJ)
- **Cardinal**: Sanctus Divinus (ENFJ)

### Expanded Classes
- **Gunslinger Ace**: Flint Sixkiller (ESTP)
- **Shadow Spectre**: Kage Nightshade (INFP)
- **Shiranui**: Kuro Shadowflame (INFP)
- **Soul Reaper**: Mortis Soulharvest (INTJ)
- **Celestial**: Asteria Starforge (INTJ)
- **Grand Master**: Omni Allskill (ESTJ)
- **Spirit Sovereign**: Wraith Soulbinder (ISFJ)
- **Elementalist**: Prism Elementbound (ENFP)
- **Lunar Knight**: Luna Moonshadow (ISFP)
- **Dokebi**: Oni Spiritwalker (ENTP)
- **Chung E**: Mei Charmweaver (ESFJ)
- **Soul Linker**: Nexus Soulbond (INFJ)
- **Hyper Novice**: Rookie Masterofall (ENFP)
- **Spirit Handler**: Whisper Spiritcaller (INFP)
- **Beast Tamer**: Feral Beastmaster (ESTP)
- **Spirit Whisperer**: Gaia Naturebound (INFJ)

## Secret Skills

Each AI Legend teaches a unique secret skill:

### Swordsman Branch
- **Dragon's Breath**: Channel the power of dragons to unleash a devastating breath attack
- **Impenetrable Bulwark**: Create an impenetrable shield that protects allies
- **Revolutionary Barrage**: Fire a barrage of bullets that can hit multiple targets

### Mage Branch
- **Arcane Convergence**: Focus arcane energy into a single devastating spell
- **Elemental Harmony**: Harmonize with the elements to enhance elemental magic
- **Temporal Distortion**: Manipulate time to gain advantages in battle

### Archer Branch
- **Piercing Sight**: See through any disguise or invisibility
- **Symphony of Souls**: Play a melody that resonates with the souls of allies
- **Harmonic Resonance**: Create a harmonic field that enhances allies and disrupts enemies
- **Cyclone Strike**: Harness the power of wind to strike from above

### Merchant Branch
- **Mechanical Overload**: Push mechanical systems beyond their normal limits
- **Catalytic Conversion**: Convert one substance into another through advanced alchemy
- **Masterwork Creation**: Create perfect items with enhanced properties

### Thief Branch
- **Void Embrace**: Embrace the void to become one with the shadows
- **Phantom Mirage**: Create a perfect illusion that can act independently
- **Shadow Surveillance**: Use shadows to spy on enemies and gather information

### Acolyte Branch
- **Divine Intervention**: Call upon divine power to save allies from certain death
- **Righteous Fury**: Channel divine wrath into devastating attacks
- **Divine Judgment**: Call down divine judgment on enemies

### Expanded Classes
- **Bullet Time**: Slow down perception of time to make impossible shots
- **Shadow Merge**: Merge with shadows to move undetected
- **Shadow Conflagration**: Combine shadow and fire to create devastating attacks
- **Soul Harvest**: Harvest the souls of fallen enemies to enhance abilities
- **Cosmic Alignment**: Align with cosmic forces to enhance abilities
- **Omnipotence**: Draw upon knowledge of all classes to adapt to any situation
- **Spirit Legion**: Summon a legion of spirits to fight alongside you
- **Elemental Convergence**: Combine all elements into a single devastating attack
- **Lunar Transformation**: Transform based on the phases of the moon
- **Spirit Transformation**: Transform into different spirit forms
- **Mystic Charm**: Charm enemies and allies with mystical eastern magic
- **Soul Convergence**: Link souls with allies to share abilities and power
- **Beginner's Luck**: Channel the unpredictable power of a novice
- **Spirit Communion**: Commune with spirits to gain their power
- **Wild Communion**: Commune with wild beasts to gain their power
- **Nature's Blessing**: Channel the power of nature to heal and protect

## Technical Implementation

The AI Legends system is implemented using several components:

### AI Integration

- Uses the AI module to connect to language models
- Supports Azure OpenAI, OpenAI, and DeepSeek providers
- Falls back to local provider if external APIs are unavailable
- Uses LangChain for memory management and context retrieval

### Map Server Integration

- Hooks into map server events for player interactions
- Manages AI Legend movement and appearance
- Handles combat mechanics and party interactions
- Processes PvP challenges and responses

### Database Integration

- Stores AI Legend data in the database
- Tracks player interactions and relationships
- Manages memory and conversation history
- Records statistics and logs for analysis

### Script Integration

- Custom scripts for each AI Legend
- Script commands for interacting with AI Legends
- Quest scripts for secret skills
- Event scripts for special appearances

## Troubleshooting

### Common Issues

1. **AI Legends Not Appearing**
   - Check if `ai_legends_enabled` is set to `true` in `ai_legends.conf`
   - Verify that the AI provider is properly configured
   - Check the logs for any errors

2. **AI Responses Are Slow**
   - Consider using a faster AI provider
   - Reduce the context window size
   - Enable caching for faster responses

3. **Memory Issues**
   - Check if LangChain is properly configured
   - Verify that the vector store is accessible
   - Adjust memory parameters for better performance

4. **Balance Issues**
   - Adjust the `ai_legends_power_advantage` setting
   - Modify individual AI Legend configurations
   - Review equipment and skill settings

### Logs and Debugging

The system logs various events to help with debugging:

- **AI Legend Logs**: Stored in the `ai_legend_logs` table
- **Server Logs**: Check the server logs for AI-related messages
- **Debug Mode**: Enable debug mode for more detailed logging

## Conclusion

The AI Legends system creates a dynamic and engaging gameplay experience by introducing intelligent AI-controlled characters that interact with players in meaningful ways. By leveraging advanced language models and persistent memory, these characters provide unique challenges, rewards, and storytelling opportunities that enhance the Ragnarok Online experience.

Through careful configuration and customization, server administrators can create a balanced and enjoyable experience that keeps players engaged and excited to explore the world and interact with these legendary characters.