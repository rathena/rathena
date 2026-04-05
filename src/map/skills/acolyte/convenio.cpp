// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "convenio.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"

SkillConvenio::SkillConvenio() : SkillImpl(AB_CONVENIO) {
}

void SkillConvenio::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd) {
		party_data *p = party_search(sd->status.party_id);
		int32 i = 0, count = 0;

		// Only usable in party
		if (p == nullptr) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}

		// Only usable as party leader.
		ARR_FIND(0, MAX_PARTY, i, p->data[i].sd == sd);
		if (i == MAX_PARTY || !p->party.member[i].leader) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}

		// Do the teleport part
		for (i = 0; i < MAX_PARTY; ++i) {
			map_session_data *pl_sd = p->data[i].sd;

			if (pl_sd == nullptr || pl_sd == sd || pl_sd->status.party_id != p->party.party_id || pc_isdead(pl_sd) ||
				sd->m != pl_sd->m)
				continue;

			// Respect /call configuration
			if( pl_sd->status.disable_call ){
				continue;
			}

			if (!(map_getmapflag(sd->m, MF_NOTELEPORT) || map_getmapflag(sd->m, MF_PVP) || map_getmapflag(sd->m, MF_BATTLEGROUND) || map_flag_gvg2(sd->m))) {
				pc_setpos(pl_sd, map_id2index(sd->m), sd->x, sd->y, CLR_TELEPORT);
				count++;
			}
		}
		if (!count)
			clif_skill_fail( *sd, getSkillId() );
	}
}
