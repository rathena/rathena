// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "truthofice.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillTruthOfIce::SkillTruthOfIce() : SkillImpl(DR_TRUTH_OF_ICE) {
}

void SkillTruthOfIce::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change* tsc = status_get_sc(target);

	if (tsc && tsc->hasSCE(type)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, status_change_end(target, type));
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, INFINITE_TICK));
}
