// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rebirth.hpp"

#include "map/mob.hpp"
#include "map/status.hpp"

SkillRebirth::SkillRebirth() : SkillImpl(NPC_REBIRTH) {
}

void SkillRebirth::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* md = BL_CAST(BL_MOB, src);

	if( md && md->state.rebirth )
		return; // only works once
	sc_start(src,target,skill_get_sc(getSkillId()),100,skill_lv,INFINITE_TICK);
}
