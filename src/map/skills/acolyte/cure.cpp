// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cure.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillCure::SkillCure() : SkillImpl(AL_CURE)
{
}

void SkillCure::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	if (status_isimmune(bl))
	{
		clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, false);
		return;
	}
	status_change_end(bl, SC_SILENCE);
	status_change_end(bl, SC_BLIND);
	status_change_end(bl, SC_CONFUSION);
	status_change_end(bl, SC_BITESCAR);
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);
}
