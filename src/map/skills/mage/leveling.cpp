// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "leveling.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillLeveling::SkillLeveling() : SkillImpl(SA_LEVELUP) {
}

void SkillLeveling::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (sd && pc_nextbaseexp(sd))
		pc_gainexp(sd, nullptr, pc_nextbaseexp(sd) * 10 / 100, 0, 0);
}
