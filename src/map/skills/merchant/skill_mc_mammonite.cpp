#include "skill_mc_mammonite.hpp"

SkillMC_MAMMONITE::SkillMC_MAMMONITE() : WeaponSkillImpl(MC_MAMMONITE) {
}

void SkillMC_MAMMONITE::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	skillratio += 50 * skill_lv;
}

void SkillMC_MAMMONITE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, MC_MAMMONITE, skill_lv, tick, flag);
}
