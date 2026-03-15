// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "coluceoheal.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillColuceoHeal::SkillColuceoHeal() : SkillImpl(AB_CHEAL) {
}

void SkillColuceoHeal::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );

	if( !sd || sd->status.party_id == 0 || flag&1 ) {
		if( sd && tstatus && !battle_check_undead(tstatus->race, tstatus->def_ele) && !tsc->getSCE(SC_BERSERK) ) {
			int32 partycount = (sd->status.party_id ? party_foreachsamemap(party_sub_count, sd, 0) : 0);

			int32 i = skill_calc_heal(src, target, AL_HEAL, pc_checkskill(sd, AL_HEAL), true);

			if( partycount > 1 )
				i += (i / 100) * (partycount * 10) / 4;
			if( (dstsd && pc_ismadogear(dstsd)) || status_isimmune(target))
				i = 0; // Should heal by 0 or won't do anything?? in iRO it breaks the healing to members.. [malufett]

			clif_skill_nodamage(src, *target, getSkillId(), i);
			if( tsc && tsc->getSCE(SC_AKAITSUKI) && i )
				i = ~i + 1;
			status_heal(target, i, 0, 0);
		}
	} else if( sd )
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
}
