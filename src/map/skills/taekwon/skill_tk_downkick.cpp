// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_tk_downkick.hpp"

#include "map/status.hpp"

SkillDownkick::SkillDownkick() : WeaponSkillImpl(TK_DOWNKICK) {
}

void SkillDownkick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
	base_skillratio += 60 + 20 * skill_lv;
}

void SkillDownkick::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 3333, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
