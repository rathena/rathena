// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wandofhermode.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

SkillWandOfHermode::SkillWandOfHermode() : SkillImpl(CG_HERMODE) {
}

void SkillWandOfHermode::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	skill_castend_song(src, getSkillId(), skill_lv, tick);
#endif
}

void SkillWandOfHermode::castendPos2(block_list *src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32 &flag) const {
#ifndef RENEWAL
	skill_clear_unitgroup(src);
	if (auto sg = skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0); sg != nullptr)
		sc_start4(src, src, SC_DANCING, 100, getSkillId(), 0, skill_lv, sg->group_id, skill_get_time(getSkillId(), skill_lv));
	flag |= 1;
#endif
}
