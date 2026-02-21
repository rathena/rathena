// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "milleniumshield2.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMilleniumShield2::SkillMilleniumShield2() : SkillImpl(NPC_MILLENNIUMSHIELD) {
}

void SkillMilleniumShield2::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)))
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
