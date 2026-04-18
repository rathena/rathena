// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulgathering.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillSoulGathering::SkillSoulGathering() : SkillImpl(SOA_SOUL_GATHERING) {
}

void SkillSoulGathering::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if( sd != nullptr ){
		int32 limit = 5 + pc_checkskill(sd, SP_SOULENERGY) * 3;

		for (int32 i = 0; i < limit; i++)
			pc_addsoulball(*sd,limit);
	}
}
