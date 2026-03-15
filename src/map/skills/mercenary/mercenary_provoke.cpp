// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_provoke.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillMercenaryProvoke::SkillMercenaryProvoke() : SkillImpl(MER_PROVOKE) {
}

void SkillMercenaryProvoke::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_data* tstatus = status_get_status_data(*target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 i = 0;

	if( status_has_mode(tstatus,MD_STATUSIMMUNE) || battle_check_undead(tstatus->race,tstatus->def_ele) ) {
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	// Official chance is 70% + 3%*skill_lv + srcBaseLevel% - tarBaseLevel%
	if(!(i = sc_start(src, target, type, 70 + 3 * skill_lv + status_get_lv(src) - status_get_lv(target), skill_lv, skill_get_time(getSkillId(), skill_lv))))
	{
		if( sd )
			clif_skill_fail( *sd, getSkillId() );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, i != 0);
	unit_skillcastcancel(target, 2);

	if( dstmd )
	{
		dstmd->state.provoke_flag = src->id;
		mob_target(dstmd, src, skill_get_range2(src, getSkillId(), skill_lv, true));
	}
	// Provoke can cause Coma even though it's a nodamage skill
	if (sd && battle_check_coma(*sd, *target, BF_MISC))
		status_change_start(src, target, SC_COMA, 10000, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
}
