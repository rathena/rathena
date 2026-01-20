// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elementalchangefire.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillElementalChangeFire::SkillElementalChangeFire() : SkillImpl(SA_ELEMENTFIRE) {
}

void SkillElementalChangeFire::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );
	mob_data* dstmd = BL_CAST( BL_MOB, target );

	if (sd && (!dstmd || status_has_mode(tstatus,MD_STATUSIMMUNE))) // Only works on monsters (Except status immune monsters).
		return;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start2(src,target, type, 100, skill_lv, skill_get_ele(getSkillId(),skill_lv),
			skill_get_time(getSkillId(), skill_lv)));
}
