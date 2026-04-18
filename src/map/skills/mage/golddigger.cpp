// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "golddigger.hpp"

#include "map/clif.hpp"
#include "map/log.hpp"
#include "map/pc.hpp"

SkillGoldDigger::SkillGoldDigger() : SkillImpl(SA_FORTUNE) {
}

void SkillGoldDigger::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if(sd) pc_getzeny(sd,status_get_lv(target)*100,LOG_TYPE_STEAL);
}
