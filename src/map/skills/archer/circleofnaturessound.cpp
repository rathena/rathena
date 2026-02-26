// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "circleofnaturessound.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCircleOfNaturesSound::SkillCircleOfNaturesSound() : SkillImpl(WM_SIRCLEOFNATURE) {
}

void SkillCircleOfNaturesSound::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( flag&1 ) {	// These affect to to all party members near the caster.
		if( sc && sc->getSCE(type) ) {
			sc_start2(src,target,type,100,skill_lv,pc_checkskill(sd, WM_LESSON),skill_get_time(getSkillId(),skill_lv));
		}
	} else if( sd ) {
		if( sc_start2(src,target,type,100,skill_lv,pc_checkskill(sd, WM_LESSON),skill_get_time(getSkillId(),skill_lv)) )
			party_foreachsamemap(skill_area_sub,sd,skill_get_splash(getSkillId(),skill_lv),src,getSkillId(),skill_lv,tick,flag|BCT_PARTY|1,skill_castend_nodamage_id);
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
