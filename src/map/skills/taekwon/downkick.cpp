// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "downkick.hpp"

#include "map/status.hpp"

SkillDownKick::SkillDownKick() : WeaponSkillImpl(TK_DOWNKICK) {
}

void SkillDownKick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 60 + 20 * skill_lv;
}

void SkillDownKick::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 3333, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
