// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magmaeruption.hpp"

SkillMagmaEruption::SkillMagmaEruption() : WeaponSkillImpl(NC_MAGMA_ERUPTION) {
}

void SkillMagmaEruption::calculateSkillRatio(const Damage*, const block_list*, const block_list*, uint16 skill_lv, int32& skillratio, int32) const {
	skillratio += 350 + 50 * skill_lv;
}

void SkillMagmaEruption::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32, enum damage_lv) const {
	sc_start(src, target, SC_STUN, 90, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
