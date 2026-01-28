// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hallucinationwalk.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHallucinationWalk::SkillHallucinationWalk() : SkillImpl(GC_HALLUCINATIONWALK) {
}

void SkillHallucinationWalk::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	int32 heal = status_get_max_hp(target) / 10;
	if( status_get_hp(target) < heal ) { // if you haven't enough HP skill fails.
		if( sd ) clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_HP_INSUFFICIENT );
		return;
	}
	if( !status_charge(target,heal,0) )
	{
		if( sd ) clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_HP_INSUFFICIENT );
		return;
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
}
