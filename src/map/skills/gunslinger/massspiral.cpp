// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "massspiral.hpp"

#include "map/status.hpp"

SkillMassSpiral::SkillMassSpiral() : WeaponSkillImpl(RL_MASS_SPIRAL) {
}

void SkillMassSpiral::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 200 * skill_lv;
}

void SkillMassSpiral::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_BLEEDING, 30 + 10 * skill_lv, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv));
}
