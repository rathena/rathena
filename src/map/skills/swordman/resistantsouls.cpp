// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "resistantsouls.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillResistantSouls::SkillResistantSouls() : SkillImpl(CR_PROVIDENCE) {
}

void SkillResistantSouls::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if(sd && dstsd){ //Check they are not another crusader [Skotlex]
		if ((dstsd->class_&MAPID_SECONDMASK) == MAPID_CRUSADER) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start(src,target,skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
