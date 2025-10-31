// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "holylight.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillHolyLight::SkillHolyLight() : SkillImpl(AL_HOLYLIGHT) {
}

void SkillHolyLight::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_end(target, SC_P_ALTER);
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
