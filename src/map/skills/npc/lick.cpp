// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lick.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillLick::SkillLick() : SkillImpl(NPC_LICK) {
}

void SkillLick::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	status_zap(target, 0, 100);
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start(src,target,type,(skill_lv*20),skill_lv,skill_get_time2(getSkillId(),skill_lv)));
}
