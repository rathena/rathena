#include "skill_gs_chainaction.hpp"

SkillGS_CHAINACTION::SkillGS_CHAINACTION() : WeaponSkillImpl(GS_CHAINACTION) {
}

void SkillGS_CHAINACTION::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, GS_CHAINACTION, skill_lv, tick, flag);
}
