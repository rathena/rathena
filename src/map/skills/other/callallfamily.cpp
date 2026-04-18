// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "callallfamily.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillCallAllFamily::SkillCallAllFamily() : SkillImpl(WE_CALLALLFAMILY) {
}

void SkillCallAllFamily::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		map_session_data *p_sd = pc_get_partner(sd);
		map_session_data *c_sd = pc_get_child(sd);

		if (!p_sd && !c_sd) { // Fail if no family members are found
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}

		// Partner must be on the same map and in same party
		if (p_sd && !status_isdead(*p_sd) && p_sd->m == sd->m && p_sd->status.party_id == sd->status.party_id)
			pc_setpos(p_sd, map_id2index(sd->m), sd->x, sd->y, CLR_TELEPORT);
		// Child must be on the same map and in same party as the parent casting
		if (c_sd && !status_isdead(*c_sd) && c_sd->m == sd->m && c_sd->status.party_id == sd->status.party_id)
			pc_setpos(c_sd, map_id2index(sd->m), sd->x, sd->y, CLR_TELEPORT);
	}
}
