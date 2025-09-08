#include "skill_gs_tripleaction.hpp"

SkillGS_TRIPLEACTION::SkillGS_TRIPLEACTION() : WeaponSkillImpl(GS_TRIPLEACTION) {
}

void SkillGS_TRIPLEACTION::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	base_skillratio += 50 * skill_lv;
}

void SkillGS_TRIPLEACTION::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_TRIPLEACTION, skill_lv, tick, flag | SD_ANIMATION);
}
