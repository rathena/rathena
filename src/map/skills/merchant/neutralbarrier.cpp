// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "neutralbarrier.hpp"

SkillNeutralBarrier::SkillNeutralBarrier() : SkillImpl(NC_NEUTRALBARRIER) {
}

void SkillNeutralBarrier::castendPos2(block_list* src, int32, int32, uint16 skill_lv, t_tick, int32&) const {
	status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->getSCE(SC_NEUTRALBARRIER_MASTER)) {
		skill_clear_unitgroup(src);
		return;
	}

	skill_clear_unitgroup(src);
	std::shared_ptr<s_skill_unit_group> sg = skill_unitsetting(src, getSkillId(), skill_lv, src->x, src->y, 0);
	if (sg != nullptr) {
		sc_start2(src, src, SC_NEUTRALBARRIER_MASTER, 100, skill_lv, sg->group_id, skill_get_time(getSkillId(), skill_lv));
	}
}
