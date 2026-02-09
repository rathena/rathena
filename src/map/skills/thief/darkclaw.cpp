// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "darkclaw.hpp"

#include "map/status.hpp"

SkillDarkClaw::SkillDarkClaw() : WeaponSkillImpl(GC_DARKCROW) {
}

void SkillDarkClaw::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv - 1);
}

void SkillDarkClaw::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	sc_start(src, target, SC_DARKCROW, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)); // Should be applied even on miss
}
