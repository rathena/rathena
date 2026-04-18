// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fullbuster.hpp"

#include "map/status.hpp"

SkillFullBuster::SkillFullBuster() : WeaponSkillImpl(GS_FULLBUSTER) {
}

void SkillFullBuster::applyCounterAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& attack_type) const {
	sc_start(src, src, SC_BLIND, 2 * skill_lv, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillFullBuster::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 100 * (skill_lv + 2);
}
