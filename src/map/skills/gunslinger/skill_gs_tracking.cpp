#include "skill_gs_tracking.hpp"

SkillGS_TRACKING::SkillGS_TRACKING() : WeaponSkillImpl(GS_TRACKING) {
}

void SkillGS_TRACKING::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 100 * (skill_lv + 1);
}

void SkillGS_TRACKING::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_TRACKING, skill_lv, tick, flag);
}
