// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "clawwave.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillClawWave::SkillClawWave() : SkillImplRecursiveDamageSplash(KR_CLAW_WAVE) {
}

void SkillClawWave::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 880 + 70 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_ENRAGE_WOLF)) {
		skillratio += 320;
	}

	skillratio += 5 * sstatus->str;

	RE_LVL_DMOD(100);
}

void SkillClawWave::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
