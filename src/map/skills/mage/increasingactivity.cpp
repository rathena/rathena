// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "increasingactivity.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillIncreasingActivity::SkillIncreasingActivity() : SkillImpl(EM_INCREASING_ACTIVITY) {
}

void SkillIncreasingActivity::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (target->type == BL_PC) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		status_heal(target, 0, 0, 10 * skill_lv, 0);
	} else if (sd)
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
}
