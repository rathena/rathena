// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "preparepotion.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillPreparePotion::SkillPreparePotion() : SkillImpl(AM_PHARMACY) {
}

void SkillPreparePotion::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd) {
		clif_skill_produce_mix_list( *sd, getSkillId(), 22);
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
