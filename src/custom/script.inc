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


/*
* Sends a message via script cmd to server
*/
BUILDIN_FUNC(discord) {
 	const char* msg = script_getstr(st,2);
 	const char* channel = script_getstr(st,3);
 	const int ret = discord_script(msg, channel);
 	if(ret == 0)
 		return SCRIPT_CMD_SUCCESS;
 	else
 		return SCRIPT_CMD_FAILURE;
}