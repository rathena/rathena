// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "encore.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillEncore::SkillEncore() : SkillImpl(BD_ENCORE) {
}

void SkillEncore::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (sd != nullptr) {
		unit_skilluse_id(src,src->id,sd->skill_id_dance,sd->skill_lv_dance);
		// Need to remove remembered skill to prevent permanent halving of SP cost
		sd->skill_id_old = 0;
	}
}
