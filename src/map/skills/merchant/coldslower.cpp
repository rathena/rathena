// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "coldslower.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillColdSlower::SkillColdSlower() : SkillImplRecursiveDamageSplash(NC_COLDSLOWER) {
}

void SkillColdSlower::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change *tsc = status_get_sc(target);

	// Status chances are applied officially through a check
	// The skill first trys to give the frozen status to targets that are hit
	sc_start(src, target, SC_FREEZE, 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
	if (tsc && !tsc->getSCE(SC_FREEZE)) // If it fails to give the frozen status, it will attempt to give the freezing status
		sc_start(src, target, SC_FREEZING, 20 + skill_lv * 10, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillColdSlower::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += 200 + 300 * skill_lv;
	RE_LVL_DMOD(150);
}

void SkillColdSlower::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Cast center might be relevant later (e.g. for knockback direction)
	skill_area_temp[4] = x;
	skill_area_temp[5] = y;
	int32 i = skill_get_splash(getSkillId(),skill_lv);
	map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR|BL_SKILL,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
}
