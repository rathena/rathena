// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wargrider.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillWargRider::SkillWargRider() : SkillImpl(RA_WUGRIDER) {
}

void SkillWargRider::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd ) {
		if( !pc_isridingwug(sd) && pc_iswug(sd) ) {
			pc_setoption(sd,sd->sc.option&~OPTION_WUG);
			pc_setoption(sd,sd->sc.option|OPTION_WUGRIDER);
		} else if( pc_isridingwug(sd) ) {
			pc_setoption(sd,sd->sc.option&~OPTION_WUGRIDER);
			pc_setoption(sd,sd->sc.option|OPTION_WUG);
		}
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
