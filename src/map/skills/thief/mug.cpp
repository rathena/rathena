// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mug.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillMug::SkillMug() : SkillImpl(RG_STEALCOIN) {
}

void SkillMug::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	mob_data *dstmd = BL_CAST(BL_MOB, target);

	if(sd) {
		if(pc_steal_coin(sd,target))
		{
			dstmd->state.provoke_flag = src->id;
			mob_target(dstmd, src, skill_get_range2(src, getSkillId(), skill_lv, true));
			clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

		}
		else
			clif_skill_fail( *sd, getSkillId() );
	}
}
