// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "abyssdagger.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillAbyssDagger::SkillAbyssDagger() : SkillImplRecursiveDamageSplash(ABC_ABYSS_DAGGER) {
}

void SkillAbyssDagger::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillAbyssDagger::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 350 + 1400 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
