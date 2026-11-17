// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "laudaramus.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillLaudaRamus::SkillLaudaRamus() : SkillImpl(AB_LAUDARAMUS) {
}

void SkillLaudaRamus::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( flag&1 || !sd || !sd->status.party_id ) {
		if( tsc && (tsc->getSCE(SC_SLEEP) || tsc->getSCE(SC_STUN) || tsc->getSCE(SC_MANDRAGORA) || tsc->getSCE(SC_SILENCE) || tsc->getSCE(SC_DEEPSLEEP)) ){
			// Success Chance: (60 + 10 * Skill Level) %
			if( rnd()%100 > 60+10*skill_lv )  return;
			status_change_end(target, SC_SLEEP);
			status_change_end(target, SC_STUN);
			status_change_end(target, SC_MANDRAGORA);
			status_change_end(target, SC_SILENCE);
			status_change_end(target, SC_DEEPSLEEP);
		} else // Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
			clif_skill_nodamage(target, *target, getSkillId(), skill_lv,
				sc_start(src,target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	} else if( sd )
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv),
			src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
}
