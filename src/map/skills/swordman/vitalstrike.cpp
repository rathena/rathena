// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vitalstrike.hpp"

#include "map/status.hpp"

SkillVitalStrike::SkillVitalStrike() : SkillImpl(LK_JOINTBEAT) {
}

void SkillVitalStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);

	base_skillratio += 10 * skill_lv - 50;

	// The 2x damage is only for the BREAK_NECK ailment.
	if (wd->miscflag & BREAK_NECK || (tsc && tsc->getSCE(SC_JOINTBEAT) && tsc->getSCE(SC_JOINTBEAT)->val2 & BREAK_NECK))
		base_skillratio *= 2;
}

void SkillVitalStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);

	flag = 1 << rnd() % 6;
	if (flag != BREAK_NECK && tsc && tsc->getSCE(SC_JOINTBEAT) && tsc->getSCE(SC_JOINTBEAT)->val2 & BREAK_NECK)
		flag = BREAK_NECK; // Target should always receive double damage if neck is already broken
	if (skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag))
		status_change_start(src, target, SC_JOINTBEAT, (50 * (skill_lv + 1) - (270 * tstatus->str) / 100) * 10, skill_lv, flag & BREAK_FLAGS, src->id, 0, skill_get_time2(getSkillId(), skill_lv), SCSTART_NONE);
}
