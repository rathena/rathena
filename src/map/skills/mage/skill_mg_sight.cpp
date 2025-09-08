#include "skill_mg_sight.hpp"

SkillMG_SIGHT::SkillMG_SIGHT() : SkillImpl(MG_SIGHT) {
}

void SkillMG_SIGHT::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	sc_start2(src, target, SC_SIGHT, 100, skill_lv, getSkillId(), skill_get_time(getSkillId(), skill_lv));
}