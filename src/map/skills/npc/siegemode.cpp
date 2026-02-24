// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "siegemode.hpp"

#include "map/clif.hpp"

SkillSiegeMode::SkillSiegeMode() : SkillImpl(NPC_SIEGEMODE) {
}

void SkillSiegeMode::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Not implemented/used: Gives EFST_SIEGEMODE which reduces speed to 1000.
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
