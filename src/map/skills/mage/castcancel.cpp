// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "castcancel.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillCastCancel::SkillCastCancel() : SkillImpl(SA_CASTCANCEL) {
}

void SkillCastCancel::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	unit_skillcastcancel(src,1);
	if(sd) {
		int32 sp = skill_get_sp(sd->skill_id_old,sd->skill_lv_old);
		sp = sp * (90 - (skill_lv-1)*20) / 100;
		if(sp < 0) sp = 0;
		status_zap(src, 0, sp);
	}
}
