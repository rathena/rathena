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
/*==========================================
 * Get the damage done to the specified mob_id
 * getmonsterdamage <mob id>;
 * by [Haruka Mayumi]
 *------------------------------------------*/
BUILDIN_FUNC(getmonsterdamage)
{
	struct block_list* bl;
	uint8 i;
	
	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}
	
	if(bl->type != BL_MOB) {
		ShowWarning("buildin_getmonsterdamage: Invalid object type!\n");
		return SCRIPT_CMD_FAILURE;
	}
	
	TBL_MOB* md = map_id2md(bl->id);
	
	for (i = 0; i < DAMAGELOG_SIZE && md->dmglog[i].id; i++) {
		setd_sub_num(st, NULL, ".@dmglog_id", i, md->dmglog[i].id, NULL);
		setd_sub_num(st, NULL, ".@dmglog_dmg", i, md->dmglog[i].dmg, NULL);
		setd_sub_num(st, NULL, ".@dmglog_flag", i, md->dmglog[i].flag, NULL);
	}
	
	script_pushint(st, i);
	return SCRIPT_CMD_SUCCESS;
}