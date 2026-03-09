// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "souldivision.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillSoulDivision::SkillSoulDivision() : StatusSkillImpl(SP_SOULDIVISION) {
}

void SkillSoulDivision::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (target->type != BL_PC) {
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		return;
	}

	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);

	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
}
