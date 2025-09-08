#include "skill_mg_napalmbeat.hpp"

SkillMG_NAPALMBEAT::SkillMG_NAPALMBEAT() : SkillImpl(MG_NAPALMBEAT) {
}

void SkillMG_NAPALMBEAT::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}