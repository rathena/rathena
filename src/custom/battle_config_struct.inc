// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config structure
 * Place the definition of custom battle configuration structure members in this file.
 *
 * Make sure that the name you want is not already taken by looking into battle.hpp
 * Do not forget to add initilization logic to battle_config_init.inc
 *
 * Format:
 *	 <datatype> name;
 **/

int32 rune_tablet_min_group_level; /// Rune Tablet: minimum group level allowed to use it (99 = GM only)
int32 rune_tablet_decomposition_uniform; /// Rune Tablet: local uniform quantity policy for card decomposition (0 = off)
int32 rune_tablet_max_equipped; /// Rune Tablet: equipped Rune Sets (1 = official single slot, up to MAX_RUNE_TABLET_EQUIPPED)
int32 rune_tablet_equip_per_category; /// Rune Tablet: at most one equipped set per category (1 = yes)
