// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "poisoningweapon.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillPoisoningWeapon::SkillPoisoningWeapon() : SkillImpl(GC_POISONINGWEAPON) {
}

void SkillPoisoningWeapon::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd ) {
		clif_poison_list( *sd, skill_lv );
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
