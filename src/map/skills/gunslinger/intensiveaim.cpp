// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "intensiveaim.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillIntensiveAim::SkillIntensiveAim() : SkillImpl(NW_INTENSIVE_AIM) {
}

void SkillIntensiveAim::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	enum sc_type type = skill_get_sc(getSkillId());
	status_change* tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(type)) {
		status_change_end(src, SC_INTENSIVE_AIM_COUNT);
		status_change_end(target, type);
	} else {
		status_change_end(src, SC_INTENSIVE_AIM_COUNT);
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
}
