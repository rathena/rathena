// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "followersummons.hpp"

#include "map/mob.hpp"

SkillFollowerSummons::SkillFollowerSummons() : SkillImpl(NPC_SUMMONSLAVE) {
}

void SkillFollowerSummons::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	if(md && md->skill_idx >= 0)
		mob_summonslave(md,md->db->skill[md->skill_idx]->val,skill_lv,getSkillId());
}
