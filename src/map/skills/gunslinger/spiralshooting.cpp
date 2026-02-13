// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiralshooting.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpiralShooting::SkillSpiralShooting() : SkillImpl(NW_SPIRAL_SHOOTING) {
}

void SkillSpiralShooting::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);

	if (flag & 1) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	} else {
		int32 splash = skill_get_splash(getSkillId(), skill_lv);

		if (sd && sd->weapontype1 == W_GRENADE)
			splash += 2;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinrange(skill_area_sub, target, splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			status_change_end(src, SC_INTENSIVE_AIM_COUNT);
	}
}

void SkillSpiralShooting::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1200 + 1700 * skill_lv;
	skillratio += 5 * sstatus->con;
	if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
		skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 150 * skill_lv;
	if (sd && sd->weapontype1 == W_RIFLE)
		skillratio += 200 + 1100 * skill_lv;
	RE_LVL_DMOD(100);
}
