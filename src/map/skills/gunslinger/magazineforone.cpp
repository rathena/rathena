// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magazineforone.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillMagazineForOne::SkillMagazineForOne() : WeaponSkillImpl(NW_MAGAZINE_FOR_ONE) {
}

void SkillMagazineForOne::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);

	if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
		status_change_end(src, SC_INTENSIVE_AIM_COUNT);
}

void SkillMagazineForOne::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 500 * skill_lv;
	skillratio += 5 * sstatus->con;
	if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
		skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 100 * skill_lv;
	if (sd && sd->weapontype1 == W_REVOLVER)
		skillratio += 50 + 300 * skill_lv;
	RE_LVL_DMOD(100);
}
