// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pinionshot.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillPinionShot::SkillPinionShot() : SkillImplRecursiveDamageSplash(AT_PINION_SHOT) {
}

void SkillPinionShot::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	skill_area_temp[1] = 0;

	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd == nullptr) {
		// if src is not BL_PC : only deals damage
		SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
		return;
	}

	int32 charge_amount = 1;

	if (status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_ZEPHYR_CHARGE))
		charge_amount += sc->getSCE(SC_ZEPHYR_CHARGE)->val3;

	if (charge_amount < 6) {
		// Updates the status (even when the attack misses) and deals damage
		sc_start4(src, src, SC_ZEPHYR_CHARGE, 100, getSkillId(), skill_lv, charge_amount, 0, skill_get_time(getSkillId(), skill_lv));
		SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
		return;
	}

	// at 6 charges : deals damage then removes SC_ZEPHYR_CHARGE and casts AT_ZEPHYR_LINK
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);

	status_change_end(src, SC_ZEPHYR_CHARGE);

	if (sd->status.party_id == 0) {
		skill_castend_nodamage_id(src, src, AT_ZEPHYR_LINK, 1, tick, 0);
	}
	else {
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(AT_ZEPHYR_LINK, 1), src, AT_ZEPHYR_LINK, 1, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
	}
}

void SkillPinionShot::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 2600 * skill_lv;
	skillratio += sstatus->con * 5;

	RE_LVL_DMOD(100);
}
