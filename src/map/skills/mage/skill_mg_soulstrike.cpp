#include "skill_mg_soulstrike.hpp"

SkillMG_SOULSTRIKE::SkillMG_SOULSTRIKE() : SkillImpl(MG_SOULSTRIKE) {
}

void SkillMG_SOULSTRIKE::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}