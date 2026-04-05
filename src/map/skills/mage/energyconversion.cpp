// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "energyconversion.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillEnergyConversion::SkillEnergyConversion() : SkillImpl(AG_ENERGY_CONVERSION) {
}

void SkillEnergyConversion::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (status_get_sp(src) == status_get_max_sp(src)) {
		if( sd != nullptr ){
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		}
		return;
	}
	
	// Apply the SP gain to the caster
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	status_heal(target, 0, (skill_lv * (skill_lv + 1) / 2) * 40, 1);
}
