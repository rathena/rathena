// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "greatleadership.hpp"

SkillGreatLeadership::SkillGreatLeadership() : SkillImpl(static_cast<e_skill>(GD_LEADERSHIP)) {
}

void SkillGreatLeadership::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag |= 1; // Prevent immediate ammo deletion; consumed on group delete.
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
