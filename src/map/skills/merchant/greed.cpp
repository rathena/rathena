// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "greed.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillGreed::SkillGreed() : SkillImpl(BS_GREED) {
}

void SkillGreed::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd){
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		map_foreachinallrange(skill_greed,target,
			skill_get_splash(getSkillId(), skill_lv),BL_ITEM,target);
	}
}
