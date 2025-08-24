#include "holywater.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillAL_HOLYWATER::SkillAL_HOLYWATER() : SkillImpl(AL_HOLYWATER)
{
}

void SkillAL_HOLYWATER::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	if (sd)
	{
		if (skill_produce_mix(sd, skill_id, ITEMID_HOLY_WATER, 0, 0, 0, 1, -1))
		{
			struct skill_unit *su;
			if ((su = map_find_skill_unit_oncell(bl, bl->x, bl->y, NJ_SUITON, nullptr, 0)) != nullptr)
				skill_delunit(su);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		else
			clif_skill_fail(*sd, skill_id);
	}
}