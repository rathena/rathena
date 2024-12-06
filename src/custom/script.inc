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

BUILDIN_FUNC(getskillname)
{
	uint16 skill_id = (script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2));
	if (!skill_get_index(skill_id))
		script_pushconststr(st, "Unknown Skill");
	else
		script_pushstrcopy(st,skill_get_desc(skill_id));
	return SCRIPT_CMD_SUCCESS;
}
