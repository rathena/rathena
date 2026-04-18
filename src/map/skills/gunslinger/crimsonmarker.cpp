// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crimsonmarker.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCrimsonMarker::SkillCrimsonMarker() : SkillImpl(RL_C_MARKER) {
}

void SkillCrimsonMarker::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	sc_type type = skill_get_sc(getSkillId());
	status_change* tsc = status_get_sc(target);
	status_change_entry* tsce = (tsc && type != SC_NONE) ? tsc->getSCE(type) : nullptr;
	int32 i;

	if (sd) {
		// If marked by someone else remove it
		if (tsce && tsce->val2 != src->id) {
			status_change_end(target, type);
		}

		// Check if marked before
		ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == target->id);
		if (i == MAX_SKILL_CRIMSON_MARKER) {
			// Find empty slot
			ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, !sd->c_marker[i]);
			if (i == MAX_SKILL_CRIMSON_MARKER) {
				clif_skill_fail(*sd, getSkillId());
				return;
			}
		}

		sd->c_marker[i] = target->id;
		status_change_start(src, target, type, 10000, skill_lv, src->id, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NOAVOID | SCSTART_NOTICKDEF | SCSTART_NORATEDEF);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	} else {
		// If mob casts this, at least SC_C_MARKER as debuff
		status_change_start(src, target, type, 10000, skill_lv, src->id, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NOAVOID | SCSTART_NOTICKDEF | SCSTART_NORATEDEF);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
