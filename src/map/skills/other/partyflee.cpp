// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "partyflee.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillPartyFlee::SkillPartyFlee() : StatusSkillImpl(ALL_PARTYFLEE) {
}

void SkillPartyFlee::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd  && !(flag&1) ) {
		if( !sd->status.party_id ) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
	} else
		StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
