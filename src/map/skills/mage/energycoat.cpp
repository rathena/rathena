// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "energycoat.hpp"

SkillEnergyCoat::SkillEnergyCoat() : SkillImpl(MG_ENERGYCOAT) {
}

void SkillEnergyCoat::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	sc_start(src, target, SC_ENERGYCOAT, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}