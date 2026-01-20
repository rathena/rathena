// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grimreaper.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillGrimReaper::SkillGrimReaper() : SkillImpl(SA_DEATH) {
}

void SkillGrimReaper::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if ( sd && dstmd && status_has_mode(&dstmd->status,MD_STATUSIMMUNE) ) {
		clif_skill_fail( *sd, getSkillId() );
		return;
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	status_kill(target);
}
