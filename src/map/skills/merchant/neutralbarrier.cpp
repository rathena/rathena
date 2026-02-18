// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "neutralbarrier.hpp"

#include "map/status.hpp"

SkillNeutralBarrier::SkillNeutralBarrier() : SkillImpl(NC_NEUTRALBARRIER) {
}

void SkillNeutralBarrier::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	std::shared_ptr<s_skill_unit_group> sg;

	if (sc != nullptr && sc->getSCE(SC_NEUTRALBARRIER_MASTER)) {
		skill_clear_unitgroup(src);
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	skill_clear_unitgroup(src); // To remove previous skills - cannot used combined
	if( (sg = skill_unitsetting(src,getSkillId(),skill_lv,src->x,src->y,0)) != nullptr ) {
		sc_start2(src,src,SC_NEUTRALBARRIER_MASTER,100,skill_lv,sg->group_id,skill_get_time(getSkillId(),skill_lv));
	}
}
