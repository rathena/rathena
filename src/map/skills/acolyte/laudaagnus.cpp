// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "laudaagnus.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillLaudaAgnus::SkillLaudaAgnus() : SkillImpl(AB_LAUDAAGNUS) {
}

void SkillLaudaAgnus::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( flag&1 || !sd || !sd->status.party_id ) {
		if( tsc && (tsc->getSCE(SC_FREEZE) || tsc->getSCE(SC_STONE) || tsc->getSCE(SC_BLIND) ||
			tsc->getSCE(SC_BURNING) || tsc->getSCE(SC_FREEZING) || tsc->getSCE(SC_CRYSTALIZE))) {
			// Success Chance: (60 + 10 * Skill Level) %
			if( rnd()%100 > 60+10*skill_lv ) return;
			status_change_end(target, SC_FREEZE);
			status_change_end(target, SC_STONE);
			status_change_end(target, SC_BLIND);
			status_change_end(target, SC_BURNING);
			status_change_end(target, SC_FREEZING);
			status_change_end(target, SC_CRYSTALIZE);
		} else //Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
			clif_skill_nodamage(target, *target, getSkillId(), skill_lv,
				sc_start(src,target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	} else if( sd )
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv),
			src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
}
