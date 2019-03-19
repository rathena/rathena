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

/**
* Generate <ITEML> string for client
* itemlink(<item_id>{,<refine>,<card0>,<card1>,<card2>,<card3>});
* itemlink2(<item_id>,<refine>,<card0>,<card1>,<card2>,<card3>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>);
* @author [Cydh]
**/
BUILDIN_FUNC(itemlink) {
	unsigned int nameid = script_getnum(st, 2), refine = 0;
	struct s_item_link itemldata;
	memset(&itemldata, 0, sizeof(itemldata));
	FETCH(3, refine);
	FETCH(4, itemldata.cards[0]);
	FETCH(5, itemldata.cards[1]);
	FETCH(6, itemldata.cards[2]);
	FETCH(7, itemldata.cards[3]);
	if (itemldata.cards[0] || itemldata.cards[1] || itemldata.cards[2] || itemldata.cards[3])
		itemldata.flag.cards = 1;
#if PACKETVER >= 20150225
	char *command = (char *)script_getfuncname(st);
	if (command[strlen(command) - 1] == '2') {
		struct item it;
		memset(&it, 0, sizeof(it));
		script_getitem_randomoption(st, NULL, &it, command, 8);
		for (uint8 i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
			if (it.option[i].id)
				itemldata.flag.options = 1;
		}
		memcpy(&itemldata.options, &it.option, sizeof(it.option));
	}
#endif
	std::string itemlstr = createItemLink(nameid, refine, &itemldata);
	char *str = (char *)aMalloc((itemlstr.size() + 1) * sizeof(char));
	safestrncpy(str, itemlstr.c_str(), itemlstr.size() + 1);
	script_pushstr(st, str);
	return SCRIPT_CMD_SUCCESS;
}
