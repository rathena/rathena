// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dancingknife.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDancingKnife::SkillDancingKnife() : SkillImplRecursiveDamageSplash(SHC_DANCING_KNIFE) {
}

void SkillDancingKnife::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 * skill_lv + 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillDancingKnife::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	if (flag & 1) {
		skill_area_temp[1] = 0;

		// Note: doesn't force player to stand before attacking
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL | SD_SPLASH, skill_castend_damage_id);
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	}
}
