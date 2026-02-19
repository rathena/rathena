// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulexhale.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSoulExhale::SkillSoulExhale() : SkillImpl(PF_SOULCHANGE) {
}

void SkillSoulExhale::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	status_change* tsc = status_get_sc(target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	map_session_data* sd = BL_CAST(BL_PC, src);
	uint32 sp1 = 0, sp2 = 0;

	if (dstmd != nullptr) {
		if (dstmd->state.soul_change_flag) {
			if (sd != nullptr) {
				clif_skill_fail(*sd, getSkillId());
			}
			return;
		}

		dstmd->state.soul_change_flag = 1;
		sp2 = sstatus->max_sp * 3 / 100;
		status_heal(src, 0, sp2, 2);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		return;
	}

	sp1 = sstatus->sp;
	sp2 = tstatus->sp;
#ifdef RENEWAL
	sp1 /= 2;
	sp2 /= 2;
	if (tsc != nullptr && tsc->hasSCE(SC_EXTREMITYFIST)) {
		sp1 = tstatus->sp;
	}
#endif
	if (tsc != nullptr && tsc->hasSCE(SC_NORECOVER_STATE)) {
		sp1 = tstatus->sp;
	}

	status_set_sp(src, sp2, 3);
	status_set_sp(target, sp1, 3);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
