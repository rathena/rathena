// Custom Script Command Definitions
// This file defines ADDITIONAL custom script commands for AI integration
//
// NOTE: The following commands are now defined in script_def.inc and
// implemented in script_ai_ipc.cpp (database IPC-based):
//   - httppost, httpget (HTTP compatibility wrappers)
//   - ai_db_request, ai_db_response, ai_db_wait, ai_db_status, ai_db_cancel
//   - npcwalk, npcwalkid (movement commands)
//
// Do NOT duplicate BUILDIN_DEF entries here to avoid "multiple definition" errors.

// === Embedded Python AI Bridge Commands ===
// High-performance direct Python calls via CPython API
// These are ADDITIONAL commands not conflicting with script_def.inc

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

// AI-Aware NPC Movement (different from npcwalk - uses embedded Python)
// Usage: ai_walk(<npc_id>, <x>, <y>, "<map>");
BUILDIN_DEF(ai_walk, "iiis")