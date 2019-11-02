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
	struct s_item_link itemldata;

	memset(&itemldata, 0, sizeof(s_item_link));
	itemldata.item.nameid = script_getnum(st, 2);

	FETCH(3, itemldata.item.refine);
	FETCH(4, itemldata.item.card[0]);
	FETCH(5, itemldata.item.card[1]);
	FETCH(6, itemldata.item.card[2]);
	FETCH(7, itemldata.item.card[3]);

	if (itemldata.item.card[0] || itemldata.item.card[1] || itemldata.item.card[2] || itemldata.item.card[3])
		itemldata.flag.cards = 1;

#if PACKETVER >= 20150225
	char *command = (char *)script_getfuncname(st);
	if (command[strlen(command) - 1] == '2') {
		script_getitem_randomoption(st, NULL, &itemldata.item, command, 8);
		for (uint8 i = 0; i < MAX_ITEM_RDM_OPT; ++i) {
			if (itemldata.item.option[i].id)
				itemldata.flag.options = 1;
		}
	}
#endif

	std::string itemlstr = createItemLink(&itemldata);
	char *str = (char *)aMalloc((itemlstr.size() + 1) * sizeof(char));
	safestrncpy(str, itemlstr.c_str(), itemlstr.size() + 1);
	script_pushstr(st, str);
	return SCRIPT_CMD_SUCCESS;
}

/**
* Encode number into base62
* base62_encode(<number>);
* @author [Cydh]
**/
BUILDIN_FUNC(base62_encode) {
	int num = script_getnum(st, 2);
	if (num < 0) {
		script_pushconststr(st,"");
		ShowError("buildin_base62_encode: Cannot process negative number '%d'!\n", num);
		return SCRIPT_CMD_FAILURE;
	}
	std::string b62 = base62_encode(num);
	char *str = (char *)aMalloc((b62.size() + 1) * sizeof(char));
	safestrncpy(str, b62.c_str(), b62.size() + 1);
	script_pushstr(st, str);
	return SCRIPT_CMD_SUCCESS;
}

/**
* Decode base62 string into number
* base62_decode("<string>");
* @author [Cydh]
**/
BUILDIN_FUNC(base62_decode) {
	script_pushint(st, base62_decode(script_getstr(st, 2)));
	return SCRIPT_CMD_SUCCESS;
}
