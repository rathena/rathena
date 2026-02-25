// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_caprice.hpp"

#include <array>

SkillCaprice::SkillCaprice() : SkillImpl(HVAN_CAPRICE) {
}

void SkillCaprice::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	static const std::array<e_skill, 4> subskills = { MG_COLDBOLT, MG_FIREBOLT, MG_LIGHTNINGBOLT, WZ_EARTHSPIKE };
	e_skill subskill_id = subskills.at(rnd() % subskills.size());
	skill_attack(skill_get_type(subskill_id), src, src, target, subskill_id, skill_lv, tick, flag);
}
