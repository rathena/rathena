// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "zen.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillZen::SkillZen() : SkillImpl(CH_SOULCOLLECT) {
}

void SkillZen::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd) {
		int32 limit = 5;
		if( sd->sc.getSCE(SC_RAISINGDRAGON) )
			limit += sd->sc.getSCE(SC_RAISINGDRAGON)->val1;
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		for (int32 i = 0; i < limit; i++)
			pc_addspiritball(sd,skill_get_time(getSkillId(),skill_lv),limit);
	}
}
