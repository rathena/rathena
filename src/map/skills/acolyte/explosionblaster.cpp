// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "explosionblaster.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillExplosionBlaster::SkillExplosionBlaster() : SkillImplRecursiveDamageSplash(IQ_EXPOSION_BLASTER) {
}

void SkillExplosionBlaster::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
}

void SkillExplosionBlaster::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* tsc = status_get_sc(target);

	skillratio += -100 + 450 + 2600 * skill_lv;
	skillratio += 10 * sstatus->pow;

	if (tsc != nullptr && tsc->getSCE(SC_HOLY_OIL)) {
		skillratio += 950 * skill_lv;
	}

	RE_LVL_DMOD(100);
}

