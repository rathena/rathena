// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "improviseddefense.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillImprovisedDefense::SkillImprovisedDefense() : SkillImpl(NJ_TATAMIGAESHI) {
}

void SkillImprovisedDefense::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 10 * skill_lv;
#ifdef RENEWAL
	base_skillratio *= 2;
#endif
}

void SkillImprovisedDefense::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (skill_unitsetting(src,getSkillId(),skill_lv,src->x,src->y,0))
		sc_start(src,src,skill_get_sc(getSkillId()),100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}
