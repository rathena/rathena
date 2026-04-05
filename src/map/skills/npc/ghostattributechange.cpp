// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ghostattributechange.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillGhostAttributeChange::SkillGhostAttributeChange() : SkillImpl(NPC_CHANGETELEKINESIS) {
}

void SkillGhostAttributeChange::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start2(src,target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_ele(getSkillId(),skill_lv),
		skill_get_time(getSkillId(), skill_lv)));
}
