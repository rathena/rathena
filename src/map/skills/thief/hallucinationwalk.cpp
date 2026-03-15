// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hallucinationwalk.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHallucinationWalk::SkillHallucinationWalk() : StatusSkillImpl(GC_HALLUCINATIONWALK) {
}

void SkillHallucinationWalk::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
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
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
