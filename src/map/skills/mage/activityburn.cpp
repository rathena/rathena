// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "activityburn.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillActivityBurn::SkillActivityBurn() : SkillImpl(EM_ACTIVITY_BURN) {
}

void SkillActivityBurn::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (target->type == BL_PC && rnd() % 100 < 20 + 10 * skill_lv) {
		uint8 ap_burn[5] = { 20, 30, 50, 60, 70 };

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		status_zap(target, 0, 0, ap_burn[skill_lv - 1]);
	} else if (sd)
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
}
