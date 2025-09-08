#include "skill_mg_fireball.hpp"

SkillMG_FIREBALL::SkillMG_FIREBALL() : SkillImpl(MG_FIREBALL) {
}

void SkillMG_FIREBALL::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}