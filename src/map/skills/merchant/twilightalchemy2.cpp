// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "twilightalchemy2.hpp"

#include "map/clif.hpp"
#include "map/itemdb.hpp"
#include "map/pc.hpp"

SkillTwilightAlchemy2::SkillTwilightAlchemy2() : SkillImpl(AM_TWILIGHT2) {
}

void SkillTwilightAlchemy2::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		//Prepare 200 Slim White Potions.
		if (!skill_produce_mix(sd, getSkillId(), ITEMID_WHITE_SLIM_POTION, 0, 0, 0, 200, -1))
			clif_skill_fail( *sd, getSkillId() );
	}
}
