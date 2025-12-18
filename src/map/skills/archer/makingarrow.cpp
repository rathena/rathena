// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "makingarrow.hpp"

#include "../../pc.hpp"
#include "../../map.hpp"

SkillMakingArrow::SkillMakingArrow() : SkillImpl(AC_MAKINGARROW)
{
}

void SkillMakingArrow::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const
{
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd != nullptr)
	{
		clif_arrow_create_list(*sd);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
