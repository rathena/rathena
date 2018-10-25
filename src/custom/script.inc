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

static int getobjects_count[11]; /// Store number of object found by getobjects command

/**
* Get object's data requested by getobjects
* @author [Cydh]
*/
static int buildin_getobjects_sub(struct block_list *bl, va_list ap) {
	struct script_state *st;
	int get_what, type;
	char varname[23];

	if (!bl)
		return 0;

	get_what = va_arg(ap, int);
	st = va_arg(ap, struct script_state *);
	type = (int)(log((int)bl->type)/log(2));

	memset(varname, '\0', sizeof(varname));
	sprintf(varname, ".@obj_%d_id", bl->type);
	setd_sub(st, NULL, varname, getobjects_count[type], (void*)__64BPRTSIZE(bl->id), NULL);
	if (get_what & 0x2) {
		memset(varname, '\0', sizeof(varname));
		sprintf(varname, ".@obj_%d_name$", bl->type);
		setd_sub(st, NULL, varname, getobjects_count[type], (void*)status_get_name(bl), NULL);
	}
	if (get_what & 0x4 && bl->type == BL_PC) {
		memset(varname, '\0', sizeof(varname));
		sprintf(varname, ".@obj_%d_cid", bl->type);
		setd_sub(st, NULL, varname, getobjects_count[type], (void*)__64BPRTSIZE((BL_CAST(BL_PC, bl))->status.char_id), NULL);
	}
	if (get_what & 0x8) {
		memset(varname, '\0', sizeof(varname));
		sprintf(varname, ".@obj_%d_class", bl->type);
		setd_sub(st, NULL, varname, getobjects_count[type], (void*)__64BPRTSIZE(status_get_class(bl)), NULL);
	}

	getobjects_count[type]++;
	return 1;
}

/**
* getobjects(<type>,<data>{,"<map name>"{,<x1>,<y1>,<x2>,<y2>}})
* @param type See enum bl_type
* @param data 0x1:GID, 0x2:Name, 0x4:Char ID (BL_PC), 0x8:Class
* @author [Cydh]
*/
BUILDIN_FUNC(getobjects) {
	int type = script_getnum(st, 2);
	int get_what = script_getnum(st, 3);
	int m = -1, x1 = 0, y1 = 0, x2 = 0, y2 = 0;

	if (script_hasdata(st, 4)) {
		int mapindex = 0;
		const char *str = script_getstr(st, 4);
		mapindex = mapindex_name2id(str);
		if (!mapindex) {
			ShowError("buildin_getobjects: Map '%s' if not found.\n", str);
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
		m = map_mapindex2mapid(mapindex);
	}
	else {
		struct block_list *bl = map_id2bl(st->rid ? st->rid : st->oid);
		if (!bl) {
			ShowError("buildin_getobjects: Script is not attached, cannot get map!\n");
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}
		m = bl->m;
	}
	if (m < 0) {
		ShowError("buildin_getobjects: Script is not on map!\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	memset(&getobjects_count, 0, sizeof(getobjects_count));
	FETCH(5, x1);
	FETCH(6, y1);
	FETCH(7, x2);
	FETCH(8, y2);

	if (x1 && y1 && x2 && y2) {
		map_foreachinarea(buildin_getobjects_sub, m, x1, y1, x2, y2, type, get_what, st);
	}
	else {
		map_foreachinmap(buildin_getobjects_sub, m, type, get_what, st);
	}

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}