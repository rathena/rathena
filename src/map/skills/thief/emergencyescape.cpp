// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "emergencyescape.hpp"

#include "map/clif.hpp"
#include "map/unit.hpp"

SkillEmergencyEscape::SkillEmergencyEscape() : SkillImpl(SC_ESCAPE) {
}

void SkillEmergencyEscape::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
	skill_blown(src, src, skill_get_blewcount(getSkillId(), skill_lv), unit_getdir(src), BLOWN_IGNORE_NO_KNOCKBACK); // Don't stop the caster from backsliding if special_state.no_knockback is active
	clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
	flag |= 1;
}
