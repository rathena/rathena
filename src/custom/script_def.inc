// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the definition of custom script commands in this file.
 * Format:
 *	BUILDIN_DEF(command_name,"parameters"),
 **/

//BUILDIN_DEF(example,""),

// HTTP Functions for AI Integration - ENABLED (Legacy)
BUILDIN_DEF(httppost,"ss"),  // httppost("<url>", "<json_body>")
BUILDIN_DEF(httpget,"s"),    // httpget("<url>")

// NPC Movement Functions for AI-Driven NPCs - ENABLED
BUILDIN_DEF(npcwalk,"sii"),   // npcwalk("<npc_name>", <x>, <y>)
BUILDIN_DEF(npcwalkid,"iii"), // npcwalkid(<npc_id>, <x>, <y>)

// Native AI Commands - Phase 1 (ZeroMQ IPC) - ENABLED
BUILDIN_DEF(ai_npc_dialogue,"sis?"),        // ai_npc_dialogue("<npc_id>", <player_id>, "<message>", {options})
BUILDIN_DEF(ai_npc_decide_action,"ss"),     // ai_npc_decide_action("<npc_id>", "<context_json>")
BUILDIN_DEF(ai_npc_get_emotion,"s"),        // ai_npc_get_emotion("<npc_id>")
BUILDIN_DEF(ai_request_async,"sss?"),       // ai_request_async("<type>", "<data_json>", "<callback>", {timeout})
BUILDIN_DEF(ai_check_result,"s"),           // ai_check_result("<request_id>")
