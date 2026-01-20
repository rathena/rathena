// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rejuvenation.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillRejuvenation::SkillRejuvenation() : SkillImpl(SA_FULLRECOVERY) {
}

void SkillRejuvenation::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (status_isimmune(target))
		return;
	status_percent_heal(target, 100, 100);
}
