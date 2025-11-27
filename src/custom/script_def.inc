// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the definition of custom script commands in this file.
 * Format:
 *	BUILDIN_DEF(command_name,"parameters"),
 **/

//BUILDIN_DEF(example,""),

// ============================================================================
// AI IPC Commands - Database-based IPC for NPC-to-AI communication
// These replace HTTP-based communication for better reliability
// Implementations in: src/custom/script_ai_ipc.cpp
// ============================================================================

// Core AI Database IPC Commands
BUILDIN_DEF(ai_db_request,"sss?"),    // ai_db_request(type, endpoint, data{, priority}) -> request_id
BUILDIN_DEF(ai_db_response,"i"),      // ai_db_response(request_id) -> response_json
BUILDIN_DEF(ai_db_wait,"ii"),         // ai_db_wait(request_id, timeout_ms) -> response_json
BUILDIN_DEF(ai_db_status,"i"),        // ai_db_status(request_id) -> status_code
BUILDIN_DEF(ai_db_cancel,"i"),        // ai_db_cancel(request_id) -> success

// HTTP Compatibility Wrappers (use AI IPC internally)
BUILDIN_DEF(httpget,"s"),             // httpget(url) -> response (backward compat)
BUILDIN_DEF(httppost,"ss"),           // httppost(url, data) -> response (backward compat)

// NPC Movement Functions for AI-Driven NPCs - ENABLED
BUILDIN_DEF(npcwalk,"sii"),   // npcwalk("<npc_name>", <x>, <y>)
BUILDIN_DEF(npcwalkid,"iii"), // npcwalkid(<npc_id>, <x>, <y>)

// Native AI Commands - Phase 1 (ZeroMQ IPC) - ENABLED
BUILDIN_DEF(ai_npc_dialogue,"sis?"),        // ai_npc_dialogue("<npc_id>", <player_id>, "<message>", {options})
BUILDIN_DEF(ai_npc_decide_action,"ss"),     // ai_npc_decide_action("<npc_id>", "<context_json>")
BUILDIN_DEF(ai_npc_get_emotion,"s"),        // ai_npc_get_emotion("<npc_id>")
BUILDIN_DEF(ai_request_async,"sss?"),       // ai_request_async("<type>", "<data_json>", "<callback>", {timeout})
BUILDIN_DEF(ai_check_result,"s"),           // ai_check_result("<request_id>")
