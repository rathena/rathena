// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config initializations
 * Place the definition of custom battle configuration initializations in this file.
 *
 * Do not forget to add a comma at the end of the line, if you want to add another config
 *
 * Format:
 *	 { "name", &battle_config.<variable name>, <default value>, <minimum value>, <maximum value> },
 **/

/**
* Extended Vending system [Lilith]
**/
{ "extended_vending",					&battle_config.extended_vending,				1,		0,		1,				},
{ "show_broadcast_info",				&battle_config.show_broadcast_info,				1,		0,		1,				},
{ "show_item_vending",					&battle_config.show_item_vending,				1,		0,		1,				},
{ "ex_vending_info",					&battle_config.ex_vending_info,					1,		0,		1,				},
{ "ex_vending_report",					&battle_config.ex_vending_report,				1,		0,		1,				}, // [Easycore]
{ "ex_buying_bound",					&battle_config.ex_buying_bound,					1,		0,		1,				}, // [Easycore]
{ "item_zeny",							&battle_config.item_zeny,						0,		0,		INT32_MAX,		},
{ "item_cash",							&battle_config.item_cash,						0,		0,		INT32_MAX,		},
