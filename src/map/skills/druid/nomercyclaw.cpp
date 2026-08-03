// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nomercyclaw.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillNoMercyClaw::SkillNoMercyClaw() : SkillImplRecursiveDamageSplash(DR_NOMERCY_CLAW) {
}

void SkillNoMercyClaw::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 80 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_WOLF)) {
		skillratio += 40 * skill_lv;
	}

	skillratio += 5 * sstatus->str;

	RE_LVL_DMOD(100);
}

void SkillNoMercyClaw::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillNoMercyClaw::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
