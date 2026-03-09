// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skysun.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSkySun::SkillSkySun() : SkillImplRecursiveDamageSplash(SKE_SKY_SUN) {
}

void SkillSkySun::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillSkySun::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1500 * skill_lv;
	skillratio += skill_lv * 7 * pc_checkskill(sd, SKE_SKY_MASTERY);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillSkySun::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
