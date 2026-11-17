// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "summonfirespiritagni.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"

SkillSummonFireSpiritAgni::SkillSummonFireSpiritAgni() : SkillImpl(SO_SUMMON_AGNI) {
}

void SkillSummonFireSpiritAgni::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		int32 elemental_class = skill_get_elemental_type(getSkillId(),skill_lv);

		// Remove previous elemental first.
		if( sd->ed )
			elemental_delete(sd->ed);

		// Summoning the new one.
		if( !elemental_create(sd,elemental_class,skill_get_time(getSkillId(),skill_lv)) ) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
