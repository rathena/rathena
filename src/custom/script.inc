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

/**
 * aa_skillinfo(<skill id>,<type>)
 *  type 0 = skill name (description), 1 = category bitmask (1 attack, 2 buff), 2 = max level
 **/
BUILDIN_FUNC(aa_skillinfo)
{
	uint16 skill_id = static_cast<uint16>(script_getnum(st, 2));
	int32 type = script_getnum(st, 3);

	switch (type) {
	case 0:
		if (skill_id == 0 || !skill_db.find(skill_id))
			script_pushconststr(st, "Unknown");
		else
			script_pushstrcopy(st, skill_get_desc(skill_id));
		break;
	case 1:
		script_pushint(st, autoattack::skill_category(skill_id));
		break;
	default:
		script_pushint(st, (skill_id == 0 || !skill_db.find(skill_id)) ? 0 : skill_get_max(skill_id));
		break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * aa_applyloot(<allowed>{,<char id>})
 * Copies AA_LOOTALL / AA_LOOTTYPE / AA_LOOTID[] / AA_LOOTON[] into the engine autoloot settings.
 * allowed = 0 clears them (e.g. VIP expired).
 **/
BUILDIN_FUNC(aa_applyloot)
{
	map_session_data* sd;

	if (!script_charid2sd(3, sd))
		return SCRIPT_CMD_FAILURE;

	autoattack::apply_loot(sd, script_getnum(st, 2) != 0);
	return SCRIPT_CMD_SUCCESS;
}
