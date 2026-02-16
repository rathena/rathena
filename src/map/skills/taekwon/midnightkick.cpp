// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "midnightkick.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMidnightKick::SkillMidnightKick() : WeaponSkillImpl(SKE_MIDNIGHT_KICK) {
}

void SkillMidnightKick::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillMidnightKick::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 800 + 1500 * skill_lv;
	skillratio += pc_checkskill(sd, SKE_SKY_MASTERY) * 5 * skill_lv;
	skillratio += 5 * sstatus->pow;

	if (sc != nullptr && (sc->getSCE(SC_MIDNIGHT_MOON) != nullptr || sc->getSCE(SC_SKY_ENCHANT) != nullptr)) {
		skillratio += 950 + 250 * skill_lv;
	}

	RE_LVL_DMOD(100);
}
