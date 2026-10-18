// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

// Rune Tablet: included here rather than in src/map/script.cpp, whose upstream
// line endings are mixed, so the patch leaves that file untouched.
#include "map/rune_tablet.hpp"

/**
 * runetablet;
 * Opens the native Rune Tablet window from a Rune Stone NPC.
 * Returns 1 when the window was opened, 0 otherwise (the reason is shown to the player).
 */
BUILDIN_FUNC(runetablet)
{
	map_session_data* sd = nullptr;

	if (!script_rid2sd(sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, rune_tablet_open(*sd, true) ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}
