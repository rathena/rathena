// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ancilla.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillAncilla::SkillAncilla() : SkillImpl(AB_ANCILLA) {
}

void SkillAncilla::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd ) {
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		skill_produce_mix(sd, getSkillId(), ITEMID_ANCILLA, 0, 0, 0, 1, -1);
	}
}
