// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "banding.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillBanding::SkillBanding() : SkillImpl(LG_BANDING) {
}

void SkillBanding::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	std::shared_ptr<s_skill_unit_group> sg;
	status_change* sc = status_get_sc(src);

	if( sc && sc->getSCE(SC_BANDING) )
		status_change_end(src,SC_BANDING);
	else if( (sg = skill_unitsetting(src,getSkillId(),skill_lv,src->x,src->y,0)) != nullptr )
		sc_start4(src,src,SC_BANDING,100,skill_lv,0,0,sg->group_id,skill_get_time(getSkillId(),skill_lv));
	clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
}
