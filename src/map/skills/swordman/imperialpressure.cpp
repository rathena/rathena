// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "imperialpressure.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillImperialPressure::SkillImperialPressure() : SkillImplRecursiveDamageSplash(IG_IMPERIAL_PRESSURE) {
}

void SkillImperialPressure::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 5600 + 1850 * skill_lv;
	skillratio += 7 * sstatus->spl;
	skillratio += 50 * pc_checkskill(sd, IG_SPEAR_SWORD_M);
	RE_LVL_DMOD(100);
}

void SkillImperialPressure::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
