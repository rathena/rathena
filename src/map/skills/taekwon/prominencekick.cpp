// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "prominencekick.hpp"

SkillProminenceKick::SkillProminenceKick() : SkillImplRecursiveDamageSplash(SJ_PROMINENCEKICK) {
}

void SkillProminenceKick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 50 + 50 * skill_lv;
}

int64 SkillProminenceKick::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	int64 dmg = SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);

	// Trigger the 2nd hit. (100% fire damage.)
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag|8|SD_ANIMATION);

	return dmg;
}
