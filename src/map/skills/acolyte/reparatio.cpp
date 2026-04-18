// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "reparatio.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillReparatio::SkillReparatio() : SkillImpl(CD_REPARATIO) {
}

void SkillReparatio::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_data* tstatus = status_get_status_data(*target);

	if (target->type != BL_PC) { // Only works on players.
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		return;
	}

	int32 heal_amount = 0;

	if (!status_isimmune(target))
		heal_amount = tstatus->max_hp;

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	clif_skill_nodamage(nullptr, *target, AL_HEAL, heal_amount);
	status_heal(target, heal_amount, 0, 0);
}
