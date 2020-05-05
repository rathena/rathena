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
// Custom Spellbook for Warlock Update
ACMD_FUNC(memorizeskill)
{
	nullpo_retr(-1,sd);

	skill_castend_nodamage_id(&sd->bl,&sd->bl,WL_READING_SB,1,gettick(),atoi(message));
	return 0;
}