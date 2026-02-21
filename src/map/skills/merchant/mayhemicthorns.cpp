// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mayhemicthorns.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMayhemicThorns::SkillMayhemicThorns() : SkillImplRecursiveDamageSplash(BO_MAYHEMIC_THORNS) {
}

void SkillMayhemicThorns::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 200 + 300 * skill_lv;
	skillratio += 5 * sstatus->pow;
	if (sc && sc->getSCE(SC_RESEARCHREPORT))
		skillratio += 150;
	RE_LVL_DMOD(100);
}

void SkillMayhemicThorns::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
