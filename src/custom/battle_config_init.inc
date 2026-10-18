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

{ "rune_tablet_min_group_level",      &battle_config.rune_tablet_min_group_level,      99,     0,      99,             },
{ "rune_tablet_decomposition_uniform", &battle_config.rune_tablet_decomposition_uniform, 0,     0,      1,              },
{ "rune_tablet_max_equipped",         &battle_config.rune_tablet_max_equipped,         1,      1,      MAX_RUNE_TABLET_EQUIPPED, },
{ "rune_tablet_equip_per_category",   &battle_config.rune_tablet_equip_per_category,   0,      0,      1,              },
