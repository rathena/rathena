// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulofheavenandearth.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSoulOfHeavenAndEarth::SkillSoulOfHeavenAndEarth() : SkillImpl(SOA_SOUL_OF_HEAVEN_AND_EARTH) {
}

void SkillSoulOfHeavenAndEarth::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {

		// Animations don't play when outside visitargete range
		if (check_distance_bl(src, target, AREA_SIZE))
			clif_skill_nodamage(target, *target, getSkillId(), skill_lv);

		status_percent_heal(target, 0, 100);

		if( src != target && sc != nullptr && sc->getSCE(SC_TOTEM_OF_TUTELARY) != nullptr ){
			status_heal(target, 0, 0, 3 * skill_lv, 0);
		}

		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
	else if (sd)
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
}
