// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dancingblade.hpp"

#include "map/status.hpp"

SkillDancingBlade::SkillDancingBlade() : SkillImpl(NPC_DANCINGBLADE) {
}

void SkillDancingBlade::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick + status_get_amotion(src), target->id, 0, 0, NPC_DANCINGBLADE_ATK, skill_lv, 0, 0);
}
