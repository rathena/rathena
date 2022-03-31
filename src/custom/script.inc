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

BUILDIN_FUNC(isspawn)
{
	uint16 mob_ids[MAX_SEARCH] = {0};
	int count = 0;

	int mod_id;
	mod_id = script_getnum(st,2);
	if (mobdb_checkid(mod_id)) {
		mob_ids[0] = mod_id;
		count = 1;
	}

	if (count <= 0){
	push_val(st->stack,C_INT,false);
	return SCRIPT_CMD_SUCCESS;
	}

	if (count >= MAX_SEARCH)
	count = MAX_SEARCH;

	for (int i = 0; i < count; i++) {
		uint16 mob_id = mob_ids[i];
		std::shared_ptr<s_mob_db> mob = mob_db.find(mob_id);

		if(!mob)
		continue;

		const std::vector<spawn_info> spawns = mob_get_spawns(mob_id);
		if (spawns.size() <= 0){
		push_val(st->stack,C_INT,false);
		return SCRIPT_CMD_SUCCESS;
		}
		else {
			for(auto& spawn : spawns){
				int16 mapid = map_mapindex2mapid(spawn.mapindex);
				if (mapid < 0)
					continue;
					push_val(st->stack,C_INT,true);
					return SCRIPT_CMD_SUCCESS;
			}
		}
	}

	push_val(st->stack,C_INT,false);
	return SCRIPT_CMD_SUCCESS;
}
