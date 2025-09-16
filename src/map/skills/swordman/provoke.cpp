// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "provoke.hpp"

#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../mob.hpp"
#include "../../battle.hpp"
#include "../../pc.hpp"

SkillProvoke::SkillProvoke() : SkillImpl(SM_PROVOKE)
{
}

void SkillProvoke::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	sc_type type = skill_get_sc(getSkillId());
	status_data *tstatus = status_get_status_data(*bl);
	map_session_data *sd = BL_CAST(BL_PC, src);
	mob_data *dstmd = BL_CAST(BL_MOB, bl);

	if (status_has_mode(tstatus, MD_STATUSIMMUNE) || battle_check_undead(tstatus->race, tstatus->def_ele))
	{
		map_freeblock_unlock();
		return;
	}
	// Official chance is 70% + 3%*skill_lv + srcBaseLevel% - tarBaseLevel%
	int32 success = sc_start(src, bl, type, 70 + 3 * skill_lv + status_get_lv(src) - status_get_lv(bl), skill_lv, skill_get_time(getSkillId(), skill_lv));
	if (!success)
	{
		if (sd)
			clif_skill_fail(*sd, getSkillId());
		map_freeblock_unlock();
		return;
	}
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, success != 0);
	unit_skillcastcancel(bl, 2);

	if (dstmd)
	{
		dstmd->state.provoke_flag = src->id;
		mob_target(dstmd, src, skill_get_range2(src, getSkillId(), skill_lv, true));
	}
	// Provoke can cause Coma even though it's a nodamage skill
	if (sd && battle_check_coma(*sd, *bl, BF_MISC))
		status_change_start(src, bl, SC_COMA, 10000, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
}
