// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "omegaabyssstrike.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillOmegaAbyssStrike::SkillOmegaAbyssStrike() : SkillImpl(ABC_ABYSS_STRIKE) {
}

void SkillOmegaAbyssStrike::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillOmegaAbyssStrike::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 2650 * skill_lv;
	skillratio += 10 * sstatus->spl;
	if (tstatus->race == RC_DEMON || tstatus->race == RC_ANGEL)
		skillratio += 200 * skill_lv;
	RE_LVL_DMOD(100);
}
