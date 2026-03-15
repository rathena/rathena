// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "createbomb.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillCreateBomb::SkillCreateBomb() : SkillImpl(GN_MAKEBOMB) {
}

void SkillCreateBomb::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		int32 qty = 1;
		sd->skill_id_old = getSkillId();
		sd->skill_lv_old = skill_lv;
		if( skill_lv > 1 )
			qty = 10;
		clif_cooking_list( *sd, 28, getSkillId(), qty, 5 );
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
