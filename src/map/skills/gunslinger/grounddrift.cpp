// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grounddrift.hpp"

SkillGroundDrift::SkillGroundDrift() : SkillImpl(GS_GROUNDDRIFT) {
}

void SkillGroundDrift::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 100 + 20 * skill_lv;
#endif
}

void SkillGroundDrift::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Ammo should be deleted right away.
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
