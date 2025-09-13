// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cartrevolution.hpp"

SkillCartRevolution::SkillCartRevolution() : WeaponSkillImpl(MC_CARTREVOLUTION) {
}

void SkillCartRevolution::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 50;
	if (sd && sd->cart_weight)
		base_skillratio += 100 * sd->cart_weight / sd->cart_weight_max; // +1% every 1% weight
	else if (!sd)
		base_skillratio += 100; // Max damage for non players.
}

void SkillCartRevolution::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	if (sd && pc_checkskill(sd, GN_REMODELING_CART))
		hitrate += pc_checkskill(sd, GN_REMODELING_CART) * 4;
}

void SkillCartRevolution::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit
	// Fall through to splash attack implementation
	// This would normally be handled by the engine to call the splash attack logic
	// For now, we'll just call the basic attack
	skill_attack(BF_WEAPON, src, src, target, MC_CARTREVOLUTION, skill_lv, tick, flag);
}
