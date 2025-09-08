#include "skill_mg_lightningbolt.hpp"

SkillMG_LIGHTNINGBOLT::SkillMG_LIGHTNINGBOLT() : SkillImpl(MG_LIGHTNINGBOLT) {
}

void SkillMG_LIGHTNINGBOLT::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}