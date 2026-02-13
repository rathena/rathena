// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "assumptio.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillAssumptio::SkillAssumptio() : StatusSkillImpl(HP_ASSUMPTIO) {
}

void SkillAssumptio::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if( sd && dstmd )
		clif_skill_fail( *sd, getSkillId() );
	else
		StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
