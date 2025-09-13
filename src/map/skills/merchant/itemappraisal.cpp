// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemappraisal.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillItemAppraisal::SkillItemAppraisal() : SkillImpl(MC_IDENTIFY) {
}

void SkillItemAppraisal::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (sd) {
		clif_item_identify_list(sd);
		if (sd->menuskill_id != MC_IDENTIFY) {
			// failed, dont consume anything
			map_freeblock_unlock();
		} else {
			// consume sp only if succeeded
			struct s_skill_condition req = skill_get_requirement(sd, MC_IDENTIFY, skill_lv);
			status_zap(src, 0, req.sp);
		}
	}
}
