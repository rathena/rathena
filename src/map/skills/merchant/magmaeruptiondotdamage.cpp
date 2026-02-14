// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magmaeruptiondotdamage.hpp"

SkillMagmaEruptionDotDamage::SkillMagmaEruptionDotDamage() : SkillImpl(NC_MAGMA_ERUPTION_DOTDAMAGE) {
}

void SkillMagmaEruptionDotDamage::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32, enum damage_lv) const {
	sc_start4(src, target, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill_get_time2(getSkillId(), skill_lv));
}
