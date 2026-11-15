// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chopchop.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillChopChop::SkillChopChop() : SkillImplRecursiveDamageSplash(KR_CHOP_CHOP) {
}

void SkillChopChop::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 740 + 80 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_ENRAGE_WOLF)) {
		skillratio += 400;
	}

	skillratio += 5 * sstatus->str;

	RE_LVL_DMOD(100);
}

void SkillChopChop::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// TODO : unknown usage of EFST_TASK_CHOP_CHOP (tick 9999, no icon, not refreshed), which starts (most probably) around skill_check_condition_castbegin
	// and ends at the same timer of the last EFST_POSTDELAY
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	this->castendDamageId(src, target, skill_lv, tick, flag);
}
