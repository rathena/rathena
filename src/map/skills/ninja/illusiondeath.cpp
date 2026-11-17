// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "illusiondeath.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillIllusionDeath::SkillIllusionDeath() : SkillImpl(KO_JYUSATSU) {
}

void SkillIllusionDeath::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if( dstsd && tsc && !tsc->getSCE(type) &&
		rnd()%100 < ((45+5*skill_lv) + skill_lv*5 - status_get_int(target)/2) ){//[(Base chance of success) + (Skill Level x 5) - (int32 / 2)]%.
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
			status_change_start(src,target,type,10000,skill_lv,0,0,0,skill_get_time(getSkillId(),skill_lv),SCSTART_NOAVOID|SCSTART_NOTICKDEF));
		status_percent_damage(src, target, tstatus->hp * skill_lv * 5, 0, false); // Does not kill the target.
		if( status_get_lv(target) <= status_get_lv(src) )
			status_change_start(src,target,SC_COMA,10,skill_lv,0,src->id,0,0,SCSTART_NONE);
	}else if( sd )
		clif_skill_fail( *sd, getSkillId() );
}
