#include "skill_mg_coldbolt.hpp"

SkillMG_COLDBOLT::SkillMG_COLDBOLT() : SkillImpl(MG_COLDBOLT) {
}

void SkillMG_COLDBOLT::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}