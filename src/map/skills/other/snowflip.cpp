// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "snowflip.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSnowFlip::SkillSnowFlip() : SkillImpl(ECL_SNOWFLIP) {
}

void SkillSnowFlip::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_end(target, SC_SLEEP);
	status_change_end(target, SC_BLEEDING);
	status_change_end(target, SC_BURNING);
	status_change_end(target, SC_DEEPSLEEP);

	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), 1, DMG_SINGLE );
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
