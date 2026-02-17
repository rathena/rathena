// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "midnightfallen.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMidnightFallen::SkillMidnightFallen() : WeaponSkillImpl(NW_MIDNIGHT_FALLEN) {
}

void SkillMidnightFallen::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	int32 splash = skill_get_splash(getSkillId(), skill_lv);
	if (sd != nullptr) {
		if (sd->status.weapon == W_GATLING)
			splash += 1;
		else if (sd->status.weapon == W_GRENADE)
			splash += 2;
	}
	map_foreachinallarea(skill_area_sub, src->m, x - splash, y - splash, x + splash, y + splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
}

void SkillMidnightFallen::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 2400 + 800 * skill_lv;
	if (sd != nullptr && sc != nullptr && sc->hasSCE(SC_HIDDEN_CARD)) {
		if (sd->weapontype1 == W_GATLING)
			skillratio += 200 * skill_lv;
		else if (sd->weapontype1 == W_GRENADE)
			skillratio += 340 * skill_lv;
		else if (sd->weapontype1 == W_SHOTGUN)
			skillratio += 400 * skill_lv;
	}
	skillratio += 5 * sstatus->con; //!TODO: check con ratio
	RE_LVL_DMOD(100);
}
