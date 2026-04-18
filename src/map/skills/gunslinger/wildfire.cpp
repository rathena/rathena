// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wildfire.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWildFire::SkillWildFire() : WeaponSkillImpl(NW_WILD_FIRE) {
}

void SkillWildFire::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);

	int32 i = skill_get_splash(getSkillId(), skill_lv);
	if (sd && sd->status.weapon == W_GRENADE)
		i += 2;
	map_foreachinallarea(skill_area_sub,
		src->m, x - i, y - i, x + i, y + i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1,
		skill_castend_damage_id);
	if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
		status_change_end(src, SC_INTENSIVE_AIM_COUNT);
}

void SkillWildFire::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1500 + 3000 * skill_lv;
	skillratio += 5 * sstatus->con;
	if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
		skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 500 * skill_lv;
	if (sd && sd->weapontype1 == W_SHOTGUN)
		skillratio += 200 * skill_lv;
	RE_LVL_DMOD(100);
}
