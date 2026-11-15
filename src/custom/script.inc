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

// ---- TranscendedRO Auto Attack (src/custom/autoattack.hpp) ----
#include "autoattack.hpp"

/**
 * autoattack(<flag>{,<char id>})
 *  flag  1 = start, 0 = stop, -1 = query only
 * Returns 1 if auto attack is running after the call, 0 otherwise.
 **/
BUILDIN_FUNC(autoattack)
{
	map_session_data* sd;

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	int32 flag = script_getnum(st, 2);

	if (flag == 1)
		autoattack::start(sd);
	else if (flag == 0)
		autoattack::stop(sd);

	script_pushint(st, autoattack::is_active(sd) ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}
