// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "galestorm.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGaleStorm::SkillGaleStorm() : SkillImplRecursiveDamageSplash(WH_GALESTORM) {
}

void SkillGaleStorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 1350 * skill_lv;
	skillratio += 10 * sstatus->con;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_CALAMITYGALE) && (tstatus->race == RC_BRUTE || tstatus->race == RC_FISH))
		skillratio += skillratio * 50 / 100;
}

void SkillGaleStorm::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	// Give AP if 3 or more targets are hit.
	if (sd && map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, skill_area_sub_count) >= 3)
		status_heal(src, 0, 0, 10, 0);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

