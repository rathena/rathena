// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ruwach.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillRuwach::SkillRuwach() : SkillImpl(AL_RUWACH)
{
}

void SkillRuwach::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start2(src, bl, type, 100, skill_lv, getSkillId(), skill_get_time(getSkillId(), skill_lv)));
}
