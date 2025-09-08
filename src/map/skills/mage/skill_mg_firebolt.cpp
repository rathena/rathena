#include "skill_mg_firebolt.hpp"

SkillMG_FIREBOLT::SkillMG_FIREBOLT() : SkillImpl(MG_FIREBOLT) {
}

void SkillMG_FIREBOLT::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}