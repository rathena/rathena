// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "blessing.hpp"

#include "../../map.hpp"
#include "../../party.hpp"
#include "../../pc.hpp"

SkillBlessing::SkillBlessing() : SkillImpl(AL_BLESSING)
{
}

void SkillBlessing::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	map_session_data *dstsd = BL_CAST(BL_PC, bl);
	status_change *tsc = status_get_sc(bl);
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv);
	if (dstsd != nullptr && tsc && tsc->getSCE(SC_CHANGEUNDEAD))
	{
		status_data* tstatus = status_get_status_data(*bl);
		if (tstatus->hp > 1)
			skill_attack(BF_MISC, src, src, bl, getSkillId(), skill_lv, tick, flag);
		return;
	}
	sc_start(src, bl, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
