// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powerofgaia.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillPowerOfGaia::SkillPowerOfGaia() : SkillImpl(EL_POWER_OF_GAIA) {
}

void SkillPowerOfGaia::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	skill_unitsetting(src,getSkillId(),skill_lv,target->x,target->y,0);
}
