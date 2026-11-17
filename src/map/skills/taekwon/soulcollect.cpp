// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulcollect.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSoulCollect::SkillSoulCollect() : SkillImpl(SP_SOULCOLLECT) {
}

void SkillSoulCollect::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc != nullptr && type != SC_NONE) ? tsc->getSCE(type) : nullptr;

	if( tsce )
	{
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,status_change_end(target, type));
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start2(src, target, type, 100, skill_lv, pc_checkskill(sd, SP_SOULENERGY), skill_get_time(getSkillId(), skill_lv)));
}
