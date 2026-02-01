// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "status_skill_impl.hpp"

#include "../clif.hpp"
#include "../status.hpp"

StatusSkillImpl::StatusSkillImpl(e_skill skillId, bool end_if_running) : SkillImpl(skillId) {
	this->end_if_running = end_if_running;
};

void StatusSkillImpl::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const
{
	sc_type type = skill_get_sc(getSkillId());

	if (type == SC_NONE) {
		return;
	}

	if (this->end_if_running) {
		status_change* tsc = status_get_sc(target);

		if (tsc != nullptr && tsc->hasSCE(type)) {
			clif_skill_nodamage(src, *target, getSkillId(), skill_lv, status_change_end(target, type));
			return;
		}
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
