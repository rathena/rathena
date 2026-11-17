// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "widesouldrain.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillWideSoulDrain::SkillWideSoulDrain() : SkillImpl(NPC_WIDESOULDRAIN) {
}

void SkillWideSoulDrain::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag&1)
		status_percent_damage(src,target,0,((skill_lv-1)%5+1)*20,false);
	else {
		skill_area_temp[2] = 0; //For SD_PREAMBLE
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		map_foreachinallrange(skill_area_sub, target,
			skill_get_splash(getSkillId(), skill_lv),BL_CHAR,
			src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
			skill_castend_nodamage_id);
	}
}
