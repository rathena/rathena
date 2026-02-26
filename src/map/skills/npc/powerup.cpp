// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powerup.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillPowerUp::SkillPowerUp() : SkillImpl(NPC_POWERUP) {
}

void SkillPowerUp::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start2(src,target,skill_get_sc(getSkillId()),100,200,100,skill_get_time(getSkillId(), skill_lv)));
}
