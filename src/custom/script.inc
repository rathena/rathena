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
 * Costume Items
 *------------------------------------------*/
BUILDIN_FUNC(costume)
{
	int i = -1, num, ep;
	TBL_PC *sd;
	item* item;
	num = script_getnum(st, 2); // Equip Slot
	if (!script_rid2sd(sd))
		return 0;
	if (equip_index_check(num))
		i = pc_checkequip(sd, equip_bitmask[num]);
	if (i < 0)
		return 0;
	item = &sd->inventory.u.items_inventory[i];
	ep = item->equip;
	if (!(ep&EQP_HEAD_LOW) && !(ep&EQP_HEAD_MID) && !(ep&EQP_HEAD_TOP))
		return 0;
	log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, item);
	pc_unequipitem(sd, i, 2);
	clif_delitem(sd, i, 1, 3);
	// --------------------------------------------------------------------
	item->refine = 0;
	item->attribute = 0;
	item->card[0] = CARD0_CREATE;
	item->card[1] = 0;
	item->card[2] = GetWord(battle_config.reserved_costume_id, 0);
	item->card[3] = GetWord(battle_config.reserved_costume_id, 1);
	if (ep&EQP_HEAD_TOP) { ep &= ~EQP_HEAD_TOP; ep |= EQP_COSTUME_HEAD_TOP; }
	if (ep&EQP_HEAD_LOW) { ep &= ~EQP_HEAD_LOW; ep |= EQP_COSTUME_HEAD_LOW; }
	if (ep&EQP_HEAD_MID) { ep &= ~EQP_HEAD_MID; ep |= EQP_COSTUME_HEAD_MID; }
	// --------------------------------------------------------------------
	log_pick_pc(sd, LOG_TYPE_SCRIPT, 1, item);
	clif_additem(sd, i, 1, 0);
	pc_equipitem(sd, i, ep);
	clif_misceffect(&sd->bl, 3);
	return 0;
}

