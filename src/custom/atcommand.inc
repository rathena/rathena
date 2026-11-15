// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

// ---- TranscendedRO Auto Attack (src/custom/autoattack.hpp) ----
#include "autoattack.hpp"

/**
 * @autoattack [on|off]
 * Toggles auto attack (GM/testing). Players normally use the @aa NPC menu,
 * which also checks VIP / ticket. Grant via groups.yml only if you want players to bypass that.
 **/
ACMD_FUNC(autoattack)
{
	nullpo_retr(-1, sd);

	bool want;
	if (message && *message && !strcmpi(message, "on"))
		want = true;
	else if (message && *message && !strcmpi(message, "off"))
		want = false;
	else
		want = !autoattack::is_active(sd);

	if (want)
		return autoattack::start(sd) ? 0 : -1;

	autoattack::stop(sd);
	return 0;
}
