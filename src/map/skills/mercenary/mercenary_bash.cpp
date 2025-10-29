// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_bash.hpp"

SkillMercenaryBash::SkillMercenaryBash() : WeaponSkillImpl(MS_BASH) {
}

void SkillMercenaryBash::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// It is proven that bonus is applied on final hitrate, not hit.
	// Base 100% + 30% per level
	base_skillratio += 30 * skill_lv;
}

void SkillMercenaryBash::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// +5% hit per level
	hit_rate += hit_rate * 5 * skill_lv / 100;
}
