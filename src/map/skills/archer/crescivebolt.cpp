// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crescivebolt.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillCresciveBolt::SkillCresciveBolt() : WeaponSkillImpl(WH_CRESCIVE_BOLT) {
}

void SkillCresciveBolt::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 500 + 1300 * skill_lv;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
	if (sc) {
		if (sc->getSCE(SC_CRESCIVEBOLT))
			skillratio += skillratio * (20 * sc->getSCE(SC_CRESCIVEBOLT)->val1) / 100;

		if (sc->getSCE(SC_CALAMITYGALE)) {
			skillratio += skillratio * 20 / 100;

			if (tstatus->race == RC_BRUTE || tstatus->race == RC_FISH)
				skillratio += skillratio * 50 / 100;
		}
	}
}

void SkillCresciveBolt::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	if( sc && sc->getSCE(SC_CRESCIVEBOLT) )
		sc_start(src, src, SC_CRESCIVEBOLT, 100, min( 3, 1 + sc->getSCE(SC_CRESCIVEBOLT)->val1 ), skill_get_time(getSkillId(), skill_lv));
	else
		sc_start(src, src, SC_CRESCIVEBOLT, 100, 1, skill_get_time(getSkillId(), skill_lv));
}
