// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shieldreflect.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillShieldReflect::SkillShieldReflect() : StatusSkillImpl(CR_REFLECTSHIELD) {
}

void SkillShieldReflect::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_DARKCROW)) { // SC_DARKCROW prevents using reflecting skills
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		return;
	}

	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
