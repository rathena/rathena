// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cantocandidus.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCantoCandidus::SkillCantoCandidus() : SkillImpl(AB_CANTO) {
}

void SkillCantoCandidus::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	int32 agi_lv = ((sd) ? pc_checkskill(sd,AL_INCAGI) : skill_get_max(AL_INCAGI)) + (((sd) ? sd->status.job_level : 50) / 10);
	if( sd == nullptr || sd->status.party_id == 0 || flag&1 )
		clif_skill_nodamage(target, *target, getSkillId(), skill_lv, sc_start(src,target,type,100,agi_lv, skill_get_time(getSkillId(),skill_lv)));
	else if( sd )
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
}
