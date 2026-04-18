// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stonecurse.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillStoneCurse::SkillStoneCurse() : SkillImpl(MG_STONECURSE) {
}

void SkillStoneCurse::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	status_data *tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(&*target);
	sc_type type = skill_get_sc(getSkillId());

	if (status_has_mode(tstatus, MD_STATUSIMMUNE)) {
		if (sd)
			clif_skill_fail(*sd, getSkillId());
		return;
	}

	if (status_isimmune(target) || !tsc)
		return;

	int32 brate = 0;

	if (sd && sd->sc.getSCE(SC_PETROLOGY_OPTION))
		brate = sd->sc.getSCE(SC_PETROLOGY_OPTION)->val3;

	// Except for players, the skill animation shows even if the status change doesn't start
	// Players get a skill has failed message instead
	if (sc_start2(src, target, type, (skill_lv * 4 + 20) + brate, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv), skill_get_time(getSkillId(), skill_lv)) || sd == nullptr)
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	else {
		clif_skill_fail( *sd, getSkillId() );
		// Level 6-10 doesn't consume a red gem if it fails [celest]
		if (skill_lv > 5)
		{ // not to consume items
			flag |= SKILL_NOCONSUME_REQ;
		}
	}
}
