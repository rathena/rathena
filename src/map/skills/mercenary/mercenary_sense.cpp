// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_sense.hpp"

#include "map/clif.hpp"
#include "map/mercenary.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillMercenarySense::SkillMercenarySense() : SkillImpl(MER_ESTIMATION) {
}

void SkillMercenarySense::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	s_mercenary_data* mer = BL_CAST(BL_MER, src);
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if( !mer )
		return;
	sd = mer->master;
	if( sd == nullptr )
		return;
	if( dstsd )
	{ // Fail on Players
		clif_skill_fail( *sd, getSkillId() );
		return;
	}

	if (dstmd != nullptr)
		clif_skill_estimation( *sd, *dstmd );
	sd = nullptr;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
