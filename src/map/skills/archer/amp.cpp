// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "amp.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillAmp::SkillAmp() : StatusSkillImpl(BD_ADAPTATION) {
}

void SkillAmp::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
#else
	status_change *tsc = status_get_sc(target);

	if(tsc && tsc->getSCE(SC_DANCING)){
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		status_change_end(target, SC_DANCING);
	}
#endif
}
