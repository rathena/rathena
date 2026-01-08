// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "statusrecovery.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillStatusRecovery::SkillStatusRecovery() : SkillImpl(PR_STRECOVERY) {
}

void SkillStatusRecovery::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change* tsc = status_get_sc(target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if(status_isimmune(target)) {
		clif_skill_nodamage(src,*target,getSkillId(), skill_lv, false);
		return;
	}
	if (battle_check_undead(tstatus->race, tstatus->def_ele))
		skill_addtimerskill(src, tick + 1000, target->id, 0, 0, getSkillId(), skill_lv, 100, flag);
	else {
		// Bodystate is reset to "normal" for non-undead
		if (tsc) {
			// The following are bodystate status changes
			status_change_end(target, SC_STONE);
			status_change_end(target, SC_FREEZE);
			status_change_end(target, SC_STUN);
			status_change_end(target, SC_SLEEP);
			status_change_end(target, SC_STONEWAIT);
			status_change_end(target, SC_BURNING);
			status_change_end(target, SC_WHITEIMPRISON);
		}
		// Resetting bodystate to normal always also resets the monster AI to idle
		if (dstmd)
			mob_unlocktarget(dstmd, tick);
	}
	if (tsc) {
		// Ends SC_NETHERWORLD and SC_NORECOVER_STATE (even on undead)
		status_change_end(target, SC_NETHERWORLD);
		status_change_end(target, SC_NORECOVER_STATE);
	}
	clif_skill_nodamage(src,*target, getSkillId(),skill_lv);
}
