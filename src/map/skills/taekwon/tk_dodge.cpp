// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tk_dodge.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDodge::SkillDodge() : SkillImpl(TK_DODGE) {
}

void SkillDodge::castendNoDamageId(struct block_list *src, struct block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(type)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, status_change_end(target, type));
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
