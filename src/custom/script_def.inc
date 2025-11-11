// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the definition of custom script commands in this file.
 * Format:
 *	BUILDIN_DEF(command_name,"parameters"),
 **/

//BUILDIN_DEF(example,""),

// HTTP Functions for AI Integration
BUILDIN_DEF(httppost,"ss"),  // httppost("<url>", "<json_body>")
BUILDIN_DEF(httpget,"s"),    // httpget("<url>")

// NPC Movement Functions for AI-Driven NPCs
BUILDIN_DEF(npcwalk,"sii"),   // npcwalk("<npc_name>", <x>, <y>)
BUILDIN_DEF(npcwalkid,"iii"), // npcwalkid(<npc_id>, <x>, <y>)
