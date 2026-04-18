// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spreadattack.hpp"

SkillSpreadAttack::SkillSpreadAttack() : SkillImplRecursiveDamageSplash(GS_SPREADATTACK) {
}

void SkillSpreadAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 30 * skill_lv;
#else
	base_skillratio += 20 * (skill_lv - 1);
#endif
}
