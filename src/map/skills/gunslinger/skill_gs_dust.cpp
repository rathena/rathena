#include "skill_gs_dust.hpp"

SkillGS_DUST::SkillGS_DUST() : WeaponSkillImpl(GS_DUST) {
}

void SkillGS_DUST::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 50 * skill_lv;
}

void SkillGS_DUST::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_DUST, skill_lv, tick, flag);
}
