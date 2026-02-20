// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rampageblaster.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillRampageBlaster::SkillRampageBlaster() : SkillImplRecursiveDamageSplash(SR_RAMPAGEBLASTER) {
}

void SkillRampageBlaster::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_change* tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_EARTHSHAKER)) {
		skillratio += 1400 + 550 * skill_lv;
		RE_LVL_DMOD(120);
	} else {
		skillratio += 900 + 350 * skill_lv;
		RE_LVL_DMOD(150);
	}

	if (sc != nullptr && sc->hasSCE(SC_GT_CHANGE))
		skillratio += skillratio * 30 / 100;
}

void SkillRampageBlaster::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;


	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
