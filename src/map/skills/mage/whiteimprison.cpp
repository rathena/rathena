// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "whiteimprison.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWhiteImprison::SkillWhiteImprison() : SkillImpl(WL_WHITEIMPRISON) {
}

void SkillWhiteImprison::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 i = 0;

	if( (src == target || battle_check_target(src, target, BCT_ENEMY)>0) && status_get_class_(target) != CLASS_BOSS && !status_isimmune(target) ) // Should not work with Bosses.
	{
		int32 rate = ( sd? sd->status.job_level : 50 ) / 4;

		if( src == target ) rate = 100; // Success Chance: On self, 100%
		else if(target->type == BL_PC) rate += 20 + 10 * skill_lv; // On Players, (20 + 10 * Skill Level) %
		else rate += 40 + 10 * skill_lv; // On Monsters, (40 + 10 * Skill Level) %

		if( sd )
			skill_blockpc_start(*sd,getSkillId(),4000);

		if( !(tsc && tsc->getSCE(type)) ){
			i = sc_start2(src,target,type,rate,skill_lv,src->id,(src == target)?5000:(target->type == BL_PC)?skill_get_time(getSkillId(),skill_lv):skill_get_time2(getSkillId(), skill_lv));
			clif_skill_nodamage(src,*target,getSkillId(),skill_lv,i);
			if( sd && !i )
				clif_skill_fail( *sd, getSkillId() );
		}
	}else
	if( sd )
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_TOTARGET );
}
