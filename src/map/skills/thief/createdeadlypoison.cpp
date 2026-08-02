// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "createdeadlypoison.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillCreateDeadlyPoison::SkillCreateDeadlyPoison() : SkillImpl(ASC_CDP) {
}

void SkillCreateDeadlyPoison::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if(sd) {
		if(skill_produce_mix(sd, getSkillId(), ITEMID_POISON_BOTTLE, 0, 0, 0, 1, -1)) //Produce a Poison Bottle.
			clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		else
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_STUFF_INSUFFICIENT );
	}
}
