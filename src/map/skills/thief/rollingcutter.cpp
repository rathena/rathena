// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rollingcutter.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillRollingCutter::SkillRollingCutter() : SkillImplRecursiveDamageSplash(GC_ROLLINGCUTTER) {
}

void SkillRollingCutter::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 50 + 80 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillRollingCutter::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	int16 count = 1;
	skill_area_temp[2] = 0;
	map_foreachinrange(skill_area_sub,src,skill_get_splash(getSkillId(),skill_lv),BL_CHAR,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|SD_PREAMBLE|SD_SPLASH|1,skill_castend_damage_id);
	if( tsc && tsc->getSCE(SC_ROLLINGCUTTER) )
	{ // Every time the skill is casted the status change is reseted adding a counter.
		count += (int16)tsc->getSCE(SC_ROLLINGCUTTER)->val1;
		if( count > 10 )
			count = 10; // Max coounter
		status_change_end(target, SC_ROLLINGCUTTER);
	}
	sc_start(src,target,SC_ROLLINGCUTTER,100,count,skill_get_time(getSkillId(),skill_lv));
	clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
}
