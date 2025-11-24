// Custom Script Command Definitions
// This file defines the custom script commands for AI integration

// HTTP Commands for AI Service Integration (Legacy - use embedded Python instead)
BUILDIN_DEF(httppost, "ss"),
BUILDIN_DEF(httpget, "s"),

// NPC Movement Commands for AI-Driven NPCs
BUILDIN_DEF(npcwalk, "sii"),
BUILDIN_DEF(npcwalkid, "iii"),

// === Embedded Python AI Bridge Commands ===
// High-performance direct Python calls via CPython API

// AI Dialogue Generation
// Usage: .@response$ = ai_dialogue(<npc_id>, "<player_message>");
BUILDIN_DEF(ai_dialogue, "is"),

// AI Decision Making
// Usage: .@decision$ = ai_decision(<npc_id>, "<context_json>");
BUILDIN_DEF(ai_decision, "is"),

// AI Memory Storage
// Usage: ai_remember(<npc_id>, "<memory_data>");
BUILDIN_DEF(ai_remember, "is"),

// AI Quest Generation
// Usage: .@quest$ = ai_quest(<npc_id>, <player_level>);
BUILDIN_DEF(ai_quest, "ii"),

// AI-Aware NPC Movement
// Usage: ai_walk(<npc_id>, <x>, <y>, "<map>");
BUILDIN_DEF(ai_walk, "iiis")