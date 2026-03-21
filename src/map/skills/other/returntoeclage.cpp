// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "returntoeclage.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillReturnToEclage::SkillReturnToEclage() : SkillImpl(ECLAGE_RECALL) {
}

void SkillReturnToEclage::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd != nullptr ){
		// Destination position.
		uint16 x = 47;
		uint16 y = 31;
		uint16 mapindex  = mapindex_name2id(MAP_ECLAGE_IN);

		sc_start( src, target, type, 100, skill_lv, skill_get_cooldown( getSkillId(), skill_lv ) );

		if(!mapindex)
		{ //Given map not found?
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}

		pc_setpos(sd, mapindex, x, y, CLR_TELEPORT);
	}
}
