#include "skill_mg_energycoat.hpp"

SkillMG_ENERGYCOAT::SkillMG_ENERGYCOAT() : SkillImpl(MG_ENERGYCOAT) {
}

void SkillMG_ENERGYCOAT::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	sc_start(src, target, SC_ENERGYCOAT, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}