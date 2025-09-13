#include "skill_mc_cartrevolution.hpp"

SkillMC_CARTREVOLUTION::SkillMC_CARTREVOLUTION() : WeaponSkillImpl(MC_CARTREVOLUTION) {
}

void SkillMC_CARTREVOLUTION::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	skillratio += 50;
	if (sd && sd->cart_weight)
		skillratio += 100 * sd->cart_weight / sd->cart_weight_max; // +1% every 1% weight
	else if (!sd)
		skillratio += 100; // Max damage for non players.
}

void SkillMC_CARTREVOLUTION::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	if (sd && pc_checkskill(sd, GN_REMODELING_CART))
		hitrate += pc_checkskill(sd, GN_REMODELING_CART) * 4;
}

void SkillMC_CARTREVOLUTION::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit
	// Fall through to splash attack implementation
	// This would normally be handled by the engine to call the splash attack logic
	// For now, we'll just call the basic attack
	skill_attack(BF_WEAPON, src, src, target, MC_CARTREVOLUTION, skill_lv, tick, flag);
}
