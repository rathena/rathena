// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "illusionshock.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillIllusionShock::SkillIllusionShock() : StatusSkillImpl(KO_KYOUGAKU) {
}

void SkillIllusionShock::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if( dstsd && tsc && !tsc->getSCE(type) && rnd()%100 < tstatus->int_/2 ){
		StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
	}else if( sd )
		clif_skill_fail( *sd, getSkillId() );
}
