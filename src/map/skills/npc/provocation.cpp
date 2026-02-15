// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "provocation.hpp"

#include "map/mob.hpp"

SkillProvocation::SkillProvocation() : SkillImpl(NPC_PROVOCATION) {
}

void SkillProvocation::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (md != nullptr) {
		mob_unlocktarget(md, tick);
	}
}
