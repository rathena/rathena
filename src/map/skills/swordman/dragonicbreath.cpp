// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragonicbreath.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDragonicBreath::SkillDragonicBreath() : SkillImplRecursiveDamageSplash(DK_DRAGONIC_BREATH) {
}

void SkillDragonicBreath::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 50 + 350 * skill_lv;
	skillratio += 7 * sstatus->pow;

	if (sc && sc->getSCE(SC_DRAGONIC_AURA)) {
		skillratio += 3 * sstatus->pow;
		skillratio += (skill_lv * (sstatus->max_hp * 25 / 100) * 7) / 100;	// Skill level x 0.07 x ((MaxHP / 4) + (MaxSP / 2))
		skillratio += (skill_lv * (sstatus->max_sp * 50 / 100) * 7) / 100;
	} else {
		skillratio += (skill_lv * (sstatus->max_hp * 25 / 100) * 5) / 100;	// Skill level x 0.05 x ((MaxHP / 4) + (MaxSP / 2))
		skillratio += (skill_lv * (sstatus->max_sp * 50 / 100) * 5) / 100;
	}

	RE_LVL_DMOD(100);
}

void SkillDragonicBreath::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
