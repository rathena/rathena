// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "darkbreath.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"

SkillDarkBreath::SkillDarkBreath() : SkillImpl(NPC_DARKBREATH) {
}

void SkillDarkBreath::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_emotion( *src, ET_ANGER );
	if (rnd() % 2 == 0)
		return; // 50% chance
	skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
}
