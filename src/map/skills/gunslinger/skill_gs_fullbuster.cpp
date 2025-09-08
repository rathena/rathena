#include "skill_gs_fullbuster.hpp"

SkillGS_FULLBUSTER::SkillGS_FULLBUSTER() : WeaponSkillImpl(GS_FULLBUSTER) {
}

void SkillGS_FULLBUSTER::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 100 * (skill_lv + 2);
}

void SkillGS_FULLBUSTER::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_FULLBUSTER, skill_lv, tick, flag);
}
