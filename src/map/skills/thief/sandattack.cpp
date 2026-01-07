// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sandattack.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillSandAttack::SkillSandAttack() : WeaponSkillImpl(TF_SPRINKLESAND) {
}

void SkillSandAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 30;
}

void SkillSandAttack::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	sc_start(src, target, SC_BLIND, (sd != nullptr) ? 20 : 15, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
