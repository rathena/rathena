// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "holywater.hpp"

#include "../../pc.hpp"

SkillHolyWater::SkillHolyWater() : SkillImpl(AL_HOLYWATER)
{
}

void SkillHolyWater::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd)
	{
		if (skill_produce_mix(sd, getSkillId(), ITEMID_HOLY_WATER, 0, 0, 0, 1, -1))
		{
			if (skill_unit* su = map_find_skill_unit_oncell(bl, bl->x, bl->y, NJ_SUITON, nullptr, 0); su != nullptr)
				skill_delunit(su);
			clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);
		}
		else
			clif_skill_fail(*sd, getSkillId());
	}
}
