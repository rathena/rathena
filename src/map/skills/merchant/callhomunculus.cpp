// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "callhomunculus.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/homunculus.hpp"
#include "map/pc.hpp"

SkillCallHomunculus::SkillCallHomunculus() : SkillImpl(AM_CALLHOMUN) {
}

void SkillCallHomunculus::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && !hom_call(sd))
		clif_skill_fail( *sd, getSkillId() );
#ifdef RENEWAL
	else if (sd && hom_is_active(sd->hd))
		skill_area_temp[0] = 1; // Already passed pre-cast checks
#endif
}
