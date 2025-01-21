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
 * Duplicate any npc on live server
 * duplicatenpc "<Source NPC name>","<New NPC shown name>","<New NPC hidden name>","<mapname>",<map_x>,<map_y>,<dir>{, spriteid{, map_xs, map_ys}}};
 *------------------------------------------*/
BUILDIN_FUNC(duplicatenpc)
{
	int map_x = script_getnum(st, 6);
	int map_y = script_getnum(st, 7);
	int dir = script_getnum(st, 8);
	int spriteid, map_xs = -1, map_ys = -1, sourceid, type, mapid, i;
	const char *sourcename = script_getstr(st, 2);
	const char *new_shown_name = script_getstr(st, 3);
	const char *new_hidden_name = script_getstr(st, 4);
	const char *mapname = script_getstr(st, 5);

	char new_npc_name[NPC_NAME_LENGTH] = "";
	struct npc_data *nd_source, *nd_target;

	if(script_hasdata(st, 10))
		map_xs = (script_getnum(st, 10) < -1) ? -1 : script_getnum(st, 10);

	if(script_hasdata(st, 11))
		map_ys = (script_getnum(st, 11) < -1) ? -1 : script_getnum(st, 10);

	if(map_xs == -1 && map_ys != -1)
		map_xs = 0;

	if(map_xs != - 1 && map_ys == -1)
		map_ys = 0;

	if(strlen(new_shown_name) + strlen(new_hidden_name) > NPC_NAME_LENGTH) {
		ShowError("buildin_duplicatenpc: New NPC shown name + New NPC hidden name is too long (max %d chars). (%s)\n", sourcename, NPC_NAME_LENGTH);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	nd_source = npc_name2id(sourcename);

	if(script_hasdata(st, 9))
		spriteid = (script_getnum(st, 9) < -1) ? -1 : script_getnum(st, 9);
	else
		spriteid = nd_source->class_;

	if(nd_source == NULL) {
		ShowError("buildin_duplicatenpc: original npc not found for duplicate. (%s)\n", sourcename);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	sourceid = nd_source->bl.id;
	type = nd_source->subtype;
	mapid = map_mapname2mapid(mapname);

	if(mapid < 0) {
		ShowError("buildin_duplicatenpc: target map not found. (%s)\n", mapname);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	CREATE(nd_target, struct npc_data, 1);
	
	strcat(new_npc_name, new_shown_name);
	strncat(new_npc_name, "#", 1);
	strncat(new_npc_name, new_hidden_name, strlen(new_hidden_name));

	safestrncpy(nd_target->name, new_npc_name , sizeof(nd_target->name));
	safestrncpy(nd_target->exname, new_npc_name, sizeof(nd_target->exname));

	nd_target->bl.prev = nd_target->bl.next = NULL;
	nd_target->bl.m = mapid;
	nd_target->bl.x = map_x;
	nd_target->bl.y = map_y;
	nd_target->bl.id = npc_get_new_npc_id();
	nd_target->class_ = spriteid;
	nd_target->speed = 200;
	nd_target->src_id = sourceid;
	nd_target->bl.type = BL_NPC;
	nd_target->subtype = (enum npc_subtype)type;

	switch(type) {
		case NPCTYPE_SCRIPT:
			nd_target->u.scr.xs = map_xs;
			nd_target->u.scr.ys = map_ys;
			nd_target->u.scr.script = nd_source->u.scr.script;
			nd_target->u.scr.label_list = nd_source->u.scr.label_list;
			nd_target->u.scr.label_list_num = nd_source->u.scr.label_list_num;
			nd_target->dynamicnpc.removal_tid = INVALID_TIMER;
			break;
		case NPCTYPE_SHOP:
		case NPCTYPE_CASHSHOP:
		case NPCTYPE_ITEMSHOP:
		case NPCTYPE_POINTSHOP:
		case NPCTYPE_MARKETSHOP:
			nd_target->u.shop.shop_item = nd_source->u.shop.shop_item;
			nd_target->u.shop.count = nd_source->u.shop.count;
			break;
		case NPCTYPE_WARP:
			if( !battle_config.warp_point_debug )
				nd_target->class_ = JT_WARPNPC;
			else
				nd_target->class_ = JT_GUILD_FLAG;
			nd_target->u.warp.xs = map_xs;
			nd_target->u.warp.ys = map_ys;
			nd_target->u.warp.mapindex = nd_source->u.warp.mapindex;
			nd_target->u.warp.x = nd_source->u.warp.x;
			nd_target->u.warp.y = nd_source->u.warp.y;
			nd_target->trigger_on_hidden = nd_source->trigger_on_hidden;
			break;
	}

	map_addnpc(mapid, nd_target);
	status_change_init(&nd_target->bl);
	unit_dataset(&nd_target->bl);
	nd_target->ud.dir = dir;
	npc_setcells(nd_target);
	map_addblock(&nd_target->bl);

	if(spriteid >= 0) {
		status_set_viewdata(&nd_target->bl, nd_target->class_);
		clif_spawn(&nd_target->bl);
	}

	strdb_put(npcname_db, nd_target->exname, nd_target);

	if(type == NPCTYPE_SCRIPT) {
		for (i = 0; i < nd_target->u.scr.label_list_num; i++) {
			char* lname = nd_target->u.scr.label_list[i].name;
			int pos = nd_target->u.scr.label_list[i].pos;

			if ((lname[0] == 'O' || lname[0] == 'o') && (lname[1] == 'N' || lname[1] == 'n')) {
				struct event_data* ev;
				char buf[NPC_NAME_LENGTH*2+3];
				snprintf(buf, ARRAYLENGTH(buf), "%s::%s", nd_target->exname, lname);

				CREATE(ev, struct event_data, 1);
				ev->nd = nd_target;
				ev->pos = pos;
				if(strdb_put(ev_db, buf, ev))
					ShowWarning("npc_parse_duplicate : duplicate event %s (%s)\n", buf, nd_target->name);
			}
		}

		for (i = 0; i < nd_target->u.scr.label_list_num; i++) {
			int t = 0, k = 0;
			char *lname = nd_target->u.scr.label_list[i].name;
			int pos = nd_target->u.scr.label_list[i].pos;
			if (sscanf(lname, "OnTimer%d%n", &t, &k) == 1 && lname[k] == '\0') {
				struct npc_timerevent_list *te = nd_target->u.scr.timer_event;
				int j, k = nd_target->u.scr.timeramount;
				if (te == NULL)
					te = (struct npc_timerevent_list *)aMalloc(sizeof(struct npc_timerevent_list));
				else
					te = (struct npc_timerevent_list *)aRealloc( te, sizeof(struct npc_timerevent_list) * (k+1) );
				for (j = 0; j < k; j++) {
					if (te[j].timer > t) {
						memmove(te+j+1, te+j, sizeof(struct npc_timerevent_list)*(k-j));
						break;
					}
				}
				te[j].timer = t;
				te[j].pos = pos;
				nd_target->u.scr.timer_event = te;
				nd_target->u.scr.timeramount++;
			}
		}
		nd_target->u.scr.timerid = INVALID_TIMER;
	}

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Remove any npc duplicate on live server
 * duplicateremove "<NPC name>";
 *------------------------------------------*/
BUILDIN_FUNC(duplicateremove)
{
	struct npc_data *nd;

	if(script_hasdata(st, 2)) {
		nd = npc_name2id(script_getstr(st, 2));
		if(nd == NULL) {
			script_pushint(st, -1);
			return SCRIPT_CMD_FAILURE;
		}
	} else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if(!nd->src_id)
		npc_unload_duplicates(nd);
	else
		npc_unload(nd,true);

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

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

