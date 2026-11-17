// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "invisible.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillInvisible::SkillInvisible() : SkillImpl(NPC_INVISIBLE) {
}

void SkillInvisible::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Have val4 passed as 6 is for "infinite cloak" (do not end on attack/skill use).
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start4(src,target,skill_get_sc(getSkillId()),100,skill_lv,0,0,6,skill_get_time(getSkillId(),skill_lv)));
}
