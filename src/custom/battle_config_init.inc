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

{ "feature.goldpc_active",              &battle_config.feature_goldpc_active,           1,      0,      1,              },
{ "feature.goldpc_time",                &battle_config.feature_goldpc_time,          3600,      0,   3600,              },
{ "feature.goldpc_max_points",          &battle_config.feature_goldpc_max_points,     300,      0,    300,              },
{ "feature.goldpc_vip",                 &battle_config.feature_goldpc_vip,              1,      0,      1,              },
