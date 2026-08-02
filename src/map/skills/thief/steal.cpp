// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "steal.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSteal::SkillSteal() : SkillImpl(TF_STEAL) {
}

void SkillSteal::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd) {
		if (pc_steal_item(sd, bl, skill_lv))
			clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);
		else
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
	}
}
