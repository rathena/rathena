// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "holylight.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillAL_HOLYLIGHT::SkillAL_HOLYLIGHT() : SkillImpl(AL_HOLYLIGHT) {
}

void SkillAL_HOLYLIGHT::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change_end(target, SC_P_ALTER);
}
