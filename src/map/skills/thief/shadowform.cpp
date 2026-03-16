// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowform.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillShadowForm::SkillShadowForm() : SkillImpl(SC_SHADOWFORM) {
}

void SkillShadowForm::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if( sd && dstsd && src != target && !dstsd->shadowform_id ) {
		if( clif_skill_nodamage(src,*target,getSkillId(),skill_lv,sc_start4(src,src,type,100,skill_lv,target->id,4+skill_lv,0,skill_get_time(getSkillId(), skill_lv))) )
			dstsd->shadowform_id = src->id;
	}
	else if( sd )
		clif_skill_fail( *sd, getSkillId() );
}
