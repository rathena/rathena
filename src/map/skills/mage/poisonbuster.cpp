// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "poisonbuster.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillPoisonBuster::SkillPoisonBuster() : SkillImplRecursiveDamageSplash(SO_POISON_BUSTER) {
}

void SkillPoisonBuster::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const status_change *tsc = status_get_sc(target);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1000 + 300 * skill_lv;
	skillratio += sstatus->int_;
	if( tsc && tsc->getSCE(SC_CLOUD_POISON) )
		skillratio += 200 * skill_lv;
	RE_LVL_DMOD(100);
	if( sc && sc->getSCE(SC_CURSED_SOIL_OPTION) )
		skillratio += (sd ? sd->status.job_level * 5 : 0);
}
