// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthquake.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillEarthquake::SkillEarthquake() : SkillImpl(NPC_EARTHQUAKE) {
}

void SkillEarthquake::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), -1, DMG_SINGLE );
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
