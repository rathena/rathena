// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itemappraisal.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillItemAppraisal::SkillItemAppraisal() : SkillImpl(MC_IDENTIFY) {
}

void SkillItemAppraisal::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd) {
		clif_item_identify_list(sd);
		if (sd->menuskill_id != getSkillId()) {
			// failed, dont consume anything
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
	}
}
