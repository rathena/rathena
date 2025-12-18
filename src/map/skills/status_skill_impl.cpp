// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "status_skill_impl.hpp"

#include "../clif.hpp"
#include "../status.hpp"

StatusSkillImpl::StatusSkillImpl(e_skill skillId) : SkillImpl(skillId) {};

void StatusSkillImpl::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const
{
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
