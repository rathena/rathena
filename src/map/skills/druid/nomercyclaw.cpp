// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nomercyclaw.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillNoMercyClaw::SkillNoMercyClaw() : SkillImplRecursiveDamageSplash(DR_NOMERCY_CLAW) {
}

void SkillNoMercyClaw::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillNoMercyClaw::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillNoMercyClaw::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 80 * skill_lv;
	if (sc && sc->getSCE(SC_ENRAGE_WOLF)) {
		skillratio += 40 * skill_lv;
	}
	skillratio += sstatus->str; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}
