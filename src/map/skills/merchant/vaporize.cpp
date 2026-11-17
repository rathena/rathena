// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vaporize.hpp"

#include "map/clif.hpp"
#include "map/homunculus.hpp"
#include "map/pc.hpp"

SkillVaporize::SkillVaporize() : SkillImpl(AM_REST) {
}

void SkillVaporize::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		if (hom_vaporize(sd,HOM_ST_REST))
			clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		else
			clif_skill_fail( *sd, getSkillId() );
	}
}
