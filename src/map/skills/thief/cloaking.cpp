// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cloaking.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCloaking::SkillCloaking() : SkillImpl(AS_CLOAKING) {
}

void SkillCloaking::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc && type != SC_NONE)?tsc->getSCE(type):nullptr;
	int32 i = 0;

	if (tsce) {
		i = status_change_end(target, type);
		if( i )
			clif_skill_nodamage(src,*target,getSkillId(),-1,i);
		else if( sd )
			clif_skill_fail( *sd, getSkillId() );
		return;
	}
	i = sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
	if( i )
		clif_skill_nodamage(src,*target,getSkillId(),-1,i);
	else if( sd )
		clif_skill_fail( *sd, getSkillId(),  USESKILL_FAIL_LEVEL );
}
