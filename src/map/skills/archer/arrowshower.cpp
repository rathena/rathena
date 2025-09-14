// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "arrowshower.hpp"

#include "../../status.hpp"

SkillArrowShower::SkillArrowShower() : SkillImplRecursiveDamageSplash(AC_SHOWER) {
}

void SkillArrowShower::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += 50 + 10 * skill_lv;
#else
	base_skillratio += -25 + 5 * skill_lv;
#endif
}

void SkillArrowShower::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_end(src, SC_CAMOUFLAGE);

	SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);
}
