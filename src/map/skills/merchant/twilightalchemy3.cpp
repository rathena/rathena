// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "twilightalchemy3.hpp"

#include "map/clif.hpp"
#include "map/itemdb.hpp"
#include "map/pc.hpp"

SkillTwilightAlchemy3::SkillTwilightAlchemy3() : SkillImpl(AM_TWILIGHT3) {
}

void SkillTwilightAlchemy3::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		int32 ebottle = pc_search_inventory(sd,ITEMID_EMPTY_BOTTLE);
		int16 alcohol_idx = -1, acid_idx = -1, fire_idx = -1;
		if( ebottle >= 0 )
			ebottle = sd->inventory.u.items_inventory[ebottle].amount;
		//check if you can produce all three, if not, then fail:
		if (!(alcohol_idx = skill_can_produce_mix(sd,ITEMID_ALCOHOL,-1, 100)) //100 Alcohol
			|| !(acid_idx = skill_can_produce_mix(sd,ITEMID_ACID_BOTTLE,-1, 50)) //50 Acid Bottle
			|| !(fire_idx = skill_can_produce_mix(sd,ITEMID_FIRE_BOTTLE,-1, 50)) //50 Flame Bottle
			|| ebottle < 200 //200 empty bottle are required at total.
		) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		skill_produce_mix(sd, getSkillId(), ITEMID_ALCOHOL, 0, 0, 0, 100, alcohol_idx-1);
		skill_produce_mix(sd, getSkillId(), ITEMID_ACID_BOTTLE, 0, 0, 0, 50, acid_idx-1);
		skill_produce_mix(sd, getSkillId(), ITEMID_FIRE_BOTTLE, 0, 0, 0, 50, fire_idx-1);
	}
}
