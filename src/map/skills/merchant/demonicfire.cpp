// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "demonicfire.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillDemonicFire::SkillDemonicFire() : SkillImplRecursiveDamageSplash(GN_DEMONIC_FIRE) {
}

void SkillDemonicFire::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (skill_lv > 20)	// Fire expansion Lv.2
		skillratio += 10 + 20 * (skill_lv - 20) + status_get_int(src) * 10;
	else if (skill_lv > 10) { // Fire expansion Lv.1
		skillratio += 10 + 20 * (skill_lv - 10) + status_get_int(src) + ((sd) ? sd->status.job_level : 50);
		RE_LVL_DMOD(100);
	} else
		skillratio += 10 + 20 * skill_lv;
}

void SkillDemonicFire::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Ammo should be deleted right away.
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
