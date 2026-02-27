// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "equipswitch.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillEquipSwitch::SkillEquipSwitch() : SkillImpl(ALL_EQSWITCH) {
}

void SkillEquipSwitch::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ){
		clif_equipswitch_reply( sd, false );

		for( int32 i = 0, position = 0; i < EQI_MAX; i++ ){
			if( sd->equip_switch_index[i] >= 0 && !( position & equip_bitmask[i] ) ){
				position |= pc_equipswitch( sd, sd->equip_switch_index[i] );
			}
		}
	}
}
