// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_change.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillChange::SkillChange() : StatusSkillImpl(HLIF_CHANGE) {
}

void SkillChange::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	status_percent_heal(target, 100, 100);
#endif

	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
