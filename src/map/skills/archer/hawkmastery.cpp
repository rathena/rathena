// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hawkmastery.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillHawkMastery::SkillHawkMastery() : SkillImpl(WH_HAWK_M) {
}

void SkillHawkMastery::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd) {
		if (!pc_isfalcon(sd))
			pc_setoption(sd, sd->sc.option | OPTION_FALCON);
		else
			pc_setoption(sd, sd->sc.option&~OPTION_FALCON);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
