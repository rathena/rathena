// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcarrowstorm.hpp"

SkillNpcArrowStorm::SkillNpcArrowStorm() : SkillImplRecursiveDamageSplash(NPC_ARROWSTORM) {
}

void SkillNpcArrowStorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	if (skill_lv > 4)
		base_skillratio += 1900;
	else
		base_skillratio += 900;
}
