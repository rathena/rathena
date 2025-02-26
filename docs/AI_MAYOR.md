# AI Mayor System

This document provides a comprehensive guide to the AI Mayor system, which analyzes server statistics and creates events to retain current players and attract new ones.

## Overview

The AI Mayor system introduces an intelligent agent that acts as a virtual "mayor" for your Ragnarok Online server. This agent analyzes player behavior, server statistics, and economic trends to identify issues and opportunities, then creates tailored events and strategies to improve player retention and acquisition.

Key features of the Mayor system include:
- Weekly statistical analysis of server data
- Automated event creation and management
- Player retention and acquisition strategies
- Comprehensive reporting and trend identification
- Integration with other AI systems

## Key Functions

### Statistical Analysis

The Mayor Agent performs regular analysis of server statistics, including:

1. **Player Demographics**
   - Total player count
   - New player acquisition
   - Active player retention
   - Player distribution by level, class, and location
   - Player activity patterns by time and day

2. **Economic Analysis**
   - Item transaction volumes
   - Zeny flow and inflation/deflation
   - Popular items and services
   - Market trends and anomalies

3. **Activity Analysis**
   - Popular activities and content
   - MVP kill statistics
   - Quest completion rates
   - Event participation and satisfaction
   - PvP and WoE participation

### Event Creation

Based on the analysis, the Mayor Agent creates tailored events to address specific needs:

1. **Event Types**
   - Combat events for battle-focused players
   - Collection events for explorers and completionists
   - Social events to build community
   - Crafting events for production-focused players
   - Trading events to stimulate the economy
   - Special and seasonal events for variety

2. **Event Targeting**
   - Events for new players to improve onboarding
   - Events for casual players to increase engagement
   - Events for hardcore players to provide challenges
   - Events for returning players to re-engage them
   - Events for all players to build community

3. **Event Difficulty**
   - Beginner events for new players
   - Intermediate events for casual players
   - Advanced events for experienced players
   - Expert events for hardcore players
   - Mixed difficulty events for community building

### Strategy Development

The Mayor Agent develops strategies to improve player retention and acquisition:

1. **Retention Strategies**
   - Identifying at-risk player segments
   - Creating targeted engagement activities
   - Recommending quality-of-life improvements
   - Suggesting content updates and refreshes
   - Developing community-building initiatives

2. **Acquisition Strategies**
   - Identifying potential player demographics
   - Suggesting promotional events and activities
   - Recommending new player experience improvements
   - Developing referral and recruitment programs
   - Creating content that appeals to target audiences

### Reporting

The Mayor Agent generates regular reports on server health and performance:

1. **Weekly Reports**
   - Summary of key metrics and trends
   - Player statistics and behavior patterns
   - Economic health and trends
   - Event performance and feedback
   - Identified issues and opportunities
   - Recommended strategies and actions

2. **Event Reports**
   - Event participation and engagement
   - Player feedback and satisfaction
   - Economic impact of events
   - Achievement of event goals
   - Recommendations for future events

## Configuration

The AI Mayor system is highly configurable through the `ai_mayor.conf` file. Key configuration options include:

### Global Settings

```
ai_mayor_enabled: true                  // Enable or disable the Mayor AI agent globally
ai_mayor_provider: "azure_openai"       // AI provider to use (azure_openai, openai, deepseek)
ai_mayor_model: "gpt-4o"                // Model to use for the Mayor AI agent
```

### Analysis Settings

```
ai_mayor_analysis_frequency: "weekly"   // Frequency of analysis (daily, weekly, monthly)
ai_mayor_analysis_day: "monday"         // Day of the week for weekly analysis
ai_mayor_analysis_time: "03:00"         // Time of day for analysis (server time)
ai_mayor_analysis_lookback_days: 7      // Number of days to look back for analysis
```

### Event Settings

```
ai_mayor_max_active_events: 3           // Maximum number of active events at once
ai_mayor_min_event_duration: 3          // Minimum event duration in days
ai_mayor_max_event_duration: 14         // Maximum event duration in days
ai_mayor_event_cooldown: 7              // Cooldown between similar events in days
```

### Reward Settings

```
ai_mayor_min_rewards_per_event: 1       // Minimum number of rewards per event
ai_mayor_max_rewards_per_event: 5       // Maximum number of rewards per event
ai_mayor_reward_scaling_enabled: true   // Enable reward scaling based on difficulty
```

### Report Settings

```
ai_mayor_report_enabled: true           // Enable weekly reports
ai_mayor_report_format: "markdown"      // Report format (text, markdown, html)
ai_mayor_report_distribution: "admin"   // Report distribution (admin, staff, public)
```

### Memory and Context Settings

```
ai_mayor_use_langchain: true            // Use LangChain for memory management
ai_mayor_memory_vector_store: "chroma"  // Vector store to use for memory
ai_mayor_memory_embedding_model: "text-embedding-3-small" // Embedding model to use
```

## Database Schema

The AI Mayor system uses several database tables to store information:

- `mayor_server_statistics`: Overall server statistics
- `mayor_player_distribution_level`: Player distribution by level
- `mayor_player_distribution_class`: Player distribution by class
- `mayor_player_distribution_map`: Player distribution by map
- `mayor_player_activity_hour`: Player activity by hour
- `mayor_player_activity_day`: Player activity by day
- `mayor_popular_activities`: Popular activities
- `mayor_item_transactions`: Item transactions
- `mayor_zeny_flow`: Zeny flow
- `mayor_mvp_kills`: MVP kills
- `mayor_quest_completions`: Quest completions
- `mayor_event_participation`: Event participation
- `mayor_event_satisfaction`: Event satisfaction
- `mayor_events`: Events
- `mayor_event_rewards`: Event rewards
- `mayor_event_parameters`: Event custom parameters
- `mayor_analysis`: Analysis results
- `mayor_player_segments`: Player segments
- `mayor_trends`: Identified trends
- `mayor_issues`: Identified issues
- `mayor_opportunities`: Identified opportunities
- `mayor_retention_strategies`: Player retention strategies
- `mayor_acquisition_strategies`: Player acquisition strategies
- `mayor_weekly_reports`: Weekly reports
- `mayor_logs`: Mayor activity logs
- `mayor_memory`: Mayor memory entries

## Implementation Details

### AI Integration

The Mayor Agent uses advanced language models (Azure OpenAI, OpenAI, or DeepSeek) to analyze data and generate insights. The system includes:

- **Persistent Memory**: Using LangChain for context management, the Mayor remembers past analyses and events
- **Contextual Analysis**: Analysis is performed with awareness of server history and trends
- **Adaptive Learning**: The system learns from the success or failure of past events and strategies

### Map Server Integration

The Mayor system integrates with the map server to:

- Collect player activity and behavior data
- Deploy and manage events
- Monitor event participation and feedback
- Distribute rewards and announcements

### Database Integration

The system uses the database to:

- Store and aggregate server statistics
- Track event performance and player feedback
- Manage event schedules and parameters
- Store analysis results and recommendations

## Event Creation Process

The Mayor Agent follows a systematic process for creating events:

1. **Analysis**: Analyze server statistics to identify needs and opportunities
2. **Ideation**: Generate event ideas based on the analysis
3. **Selection**: Select the most appropriate event based on current needs
4. **Design**: Design the event, including objectives, mechanics, and rewards
5. **Scripting**: Generate the event script using templates
6. **Validation**: Validate the script for errors and balance issues
7. **Deployment**: Deploy the event to the server
8. **Announcement**: Announce the event to players
9. **Monitoring**: Monitor event participation and feedback
10. **Evaluation**: Evaluate event success and gather insights for future events

## Event Types and Examples

### Combat Events

Combat events focus on battling monsters and bosses:

- **Monster Invasion**: Waves of monsters invade a city or area
- **Boss Challenge**: Special boss with unique mechanics
- **Hunting Competition**: Competition to kill the most monsters
- **Survival Challenge**: Survive waves of increasingly difficult monsters
- **Team Battle**: Teams compete to defeat monsters or each other

### Collection Events

Collection events focus on gathering items or resources:

- **Treasure Hunt**: Find hidden treasures throughout the world
- **Resource Gathering**: Collect specific resources for rewards
- **Fragment Collection**: Collect fragments to assemble a special item
- **Rare Drop Event**: Increased drop rates for rare items
- **Scavenger Hunt**: Find specific items in specific locations

### Exploration Events

Exploration events focus on discovering new areas or secrets:

- **Map Exploration**: Explore specific maps for rewards
- **Hidden Location**: Find hidden locations throughout the world
- **Landmark Tour**: Visit specific landmarks for rewards
- **Maze Navigation**: Navigate through a maze or dungeon
- **Secret Discovery**: Uncover hidden secrets or lore

### Social Events

Social events focus on player interaction and community building:

- **Community Festival**: Celebration with various activities
- **Group Challenge**: Challenges that require group cooperation
- **Trading Fair**: Event focused on item trading
- **Fashion Show**: Competition for best character appearance
- **Wedding Festival**: Special benefits for in-game marriages

### Crafting Events

Crafting events focus on production and creation:

- **Crafting Competition**: Competition to craft the most or best items
- **Special Recipe**: Limited-time recipes for unique items
- **Material Boost**: Increased gathering rates for crafting materials
- **Mastercraft Challenge**: Create the highest quality items
- **Invention Contest**: Create new items with unique combinations

### Trading Events

Trading events focus on economic activity:

- **Merchant Fair**: Special market with unique vendors
- **Auction House**: Special auctions for rare items
- **Bargain Hunt**: Find the best deals from NPCs
- **Trade Route**: Transport goods between cities for profit
- **Market Manipulation**: Influence market prices through collective action

### Special Events

Special events are unique and don't fit other categories:

- **Server Anniversary**: Celebration of server milestones
- **Player Appreciation**: Event thanking players for their support
- **Unique Encounter**: One-time special encounter or story
- **Mystery Event**: Event with unknown mechanics or rewards
- **Collaborative Challenge**: Server-wide goal that requires all players

### Seasonal Events

Seasonal events are tied to real-world seasons or holidays:

- **Winter Festival**: Winter-themed activities and rewards
- **Spring Celebration**: Spring-themed activities and rewards
- **Summer Adventure**: Summer-themed activities and rewards
- **Autumn Harvest**: Autumn-themed activities and rewards
- **Holiday Events**: Events for specific holidays

## Player Retention Strategies

The Mayor Agent develops various strategies to improve player retention:

### Content Engagement

- **Content Rotation**: Regularly rotate available content to keep it fresh
- **Progressive Challenges**: Create progressively difficult challenges
- **Achievement Systems**: Develop comprehensive achievement systems
- **Content Refreshes**: Periodically refresh existing content
- **Player-Generated Content**: Enable and promote player-created content

### Community Building

- **Guild Activities**: Promote and support guild activities
- **Community Events**: Create events that build community
- **Communication Channels**: Establish effective communication channels
- **Recognition Programs**: Recognize player contributions and achievements
- **Mentorship Programs**: Connect experienced players with newcomers

### Quality of Life

- **User Interface Improvements**: Enhance the user interface
- **Convenience Features**: Add features that make the game more convenient
- **Performance Optimization**: Improve game performance
- **Bug Fixing**: Promptly address bugs and issues
- **Player Feedback Systems**: Establish effective feedback systems

### Reward Systems

- **Login Rewards**: Reward players for regular login
- **Milestone Rewards**: Reward players for reaching milestones
- **Loyalty Programs**: Reward long-term players
- **Comeback Incentives**: Provide incentives for returning players
- **Referral Rewards**: Reward players for referring friends

## Player Acquisition Strategies

The Mayor Agent develops various strategies to improve player acquisition:

### Marketing and Promotion

- **Targeted Advertising**: Advertise to specific demographics
- **Content Creation**: Create engaging content for social media
- **Influencer Partnerships**: Partner with relevant influencers
- **Community Showcases**: Showcase community achievements
- **Special Promotions**: Create special promotions for new players

### New Player Experience

- **Streamlined Onboarding**: Improve the new player experience
- **Tutorial Enhancements**: Enhance tutorials and guidance
- **Starter Packages**: Provide helpful starter packages
- **Early Progression**: Ensure satisfying early progression
- **New Player Protection**: Protect new players from negative experiences

### Referral Programs

- **Friend Referral**: Reward players for referring friends
- **Guild Recruitment**: Encourage guilds to recruit new players
- **Community Ambassadors**: Establish community ambassador programs
- **Return Campaigns**: Create campaigns to bring back former players
- **Cross-Promotion**: Cross-promote with complementary games or communities

## Troubleshooting

### Common Issues

1. **Analysis Not Running**
   - Check if `ai_mayor_enabled` is set to `true` in `ai_mayor.conf`
   - Verify that the analysis schedule is properly configured
   - Check the logs for any errors

2. **AI Responses Are Slow**
   - Consider using a faster AI provider
   - Reduce the context window size
   - Enable caching for faster responses

3. **Events Not Generating**
   - Check if event generation is enabled
   - Verify that there is sufficient data for analysis
   - Check for errors in the event script templates

4. **Database Issues**
   - Check database connection settings
   - Verify that all required tables exist
   - Check for database errors in the logs

### Logs and Debugging

The system logs various events to help with debugging:

- **Mayor Logs**: Stored in the `mayor_logs` table
- **Server Logs**: Check the server logs for AI-related messages
- **Debug Mode**: Enable debug mode for more detailed logging

## Integration with Other AI Systems

The Mayor Agent can integrate with other AI systems in the rAthena AI World:

### AI Legends Integration

The Mayor can coordinate with AI Legends to:
- Create events featuring AI Legend characters
- Analyze player interactions with AI Legends
- Develop strategies involving AI Legends
- Create narrative connections between events and AI Legends

### Beggar Integration

The Mayor can coordinate with the Nameless Beggar to:
- Create events that involve the Beggar character
- Analyze player interactions with the Beggar
- Develop strategies involving the Beggar's mysterious items
- Create narrative connections between events and the Beggar's story

### World Evolution Integration

The Mayor can coordinate with the World Evolution system to:
- Create events that respond to world state changes
- Analyze the impact of world evolution on player behavior
- Develop strategies that leverage world evolution
- Create narrative connections between events and world changes

## Conclusion

The AI Mayor system provides a powerful tool for server administrators to analyze server health, create engaging events, and develop effective strategies for player retention and acquisition. By leveraging advanced AI technology, the system can identify trends, issues, and opportunities that might otherwise go unnoticed, and respond with tailored solutions that enhance the player experience and server community.

Through careful configuration and customization, server administrators can create a dynamic and responsive game environment that keeps players engaged and excited to explore the world of Ragnarok Online.