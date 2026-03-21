// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulrevolution.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSoulRevolution::SkillSoulRevolution() : SkillImpl(SP_SOULREVOLVE) {
}

void SkillSoulRevolution::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (!(tsc && (tsc->getSCE(SC_SPIRIT) || tsc->getSCE(SC_SOULGOLEM) || tsc->getSCE(SC_SOULSHADOW) || tsc->getSCE(SC_SOULFALCON) || tsc->getSCE(SC_SOULFAIRY)))) {
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		return;
	}
	status_heal(target, 0, 50*skill_lv, 2);
	status_change_end(target, SC_SPIRIT);
	status_change_end(target, SC_SOULGOLEM);
	status_change_end(target, SC_SOULSHADOW);
	status_change_end(target, SC_SOULFALCON);
	status_change_end(target, SC_SOULFAIRY);
}
