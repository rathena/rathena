// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "attackmachine.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillAttackMachine::SkillAttackMachine() : SkillImplRecursiveDamageSplash(MT_A_MACHINE) {
}

void SkillAttackMachine::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 150 + 700 * skill_lv;
	skillratio += 5 * sstatus->pow;	// TODO : unknown pow ratio

	RE_LVL_DMOD(100);
}
