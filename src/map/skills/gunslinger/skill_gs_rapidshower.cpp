#include "skill_gs_rapidshower.hpp"

SkillGS_RAPIDSHOWER::SkillGS_RAPIDSHOWER() : WeaponSkillImpl(GS_RAPIDSHOWER) {
}

void SkillGS_RAPIDSHOWER::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 400 + 50 * skill_lv;
}

void SkillGS_RAPIDSHOWER::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_RAPIDSHOWER, skill_lv, tick, flag);
}
