// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cloakingexceed.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCloakingExceed::SkillCloakingExceed() : SkillImpl(GC_CLOAKINGEXCEED) {
}

void SkillCloakingExceed::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );
	bool i = 0;

	if (status_get_sc(target) != nullptr && type != SC_NONE) {
		i = status_change_end(target, type);
		if( i )
			clif_skill_nodamage(src,*target,getSkillId(),-1,i);
		else if( sd )
			clif_skill_fail( *sd, getSkillId() );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	i = sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
	if( i )
		clif_skill_nodamage(src,*target,getSkillId(),-1,i);
	else if( sd )
		clif_skill_fail( *sd, getSkillId(),  USESKILL_FAIL_LEVEL );
}
