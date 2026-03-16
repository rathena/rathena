// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragoncombo.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillDragonCombo::SkillDragonCombo() : WeaponSkillImpl(SR_DRAGONCOMBO) {
}

void SkillDragonCombo::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target, SC_STUN, 1 + skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillDragonCombo::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += 100 + 80 * skill_lv;
	RE_LVL_DMOD(100);
}
