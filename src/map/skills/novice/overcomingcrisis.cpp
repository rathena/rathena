// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "overcomingcrisis.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillOvercomingCrisis::SkillOvercomingCrisis() : StatusSkillImpl(HN_OVERCOMING_CRISIS) {
}

void SkillOvercomingCrisis::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
	status_percent_heal(target, 100, 0);
}
