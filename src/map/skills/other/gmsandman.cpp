// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gmsandman.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillGmSandman::SkillGmSandman() : SkillImpl(GM_SANDMAN) {
}

void SkillGmSandman::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	if( tsc ) {
		if( tsc->opt1 == OPT1_SLEEP )
			tsc->opt1 = 0;
		else
			tsc->opt1 = OPT1_SLEEP;
		clif_changeoption(target);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
