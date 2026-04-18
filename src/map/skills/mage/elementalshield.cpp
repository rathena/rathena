// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elementalshield.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"

SkillElementalShield::SkillElementalShield() : SkillImpl(SO_ELEMENTAL_SHIELD) {
}

void SkillElementalShield::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (!sd || sd->status.party_id == 0 || flag&1) {
		if (sd && sd->status.party_id == 0) {
			clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
			if (sd->ed && skill_get_state(getSkillId()) == ST_ELEMENTALSPIRIT2)
				elemental_delete(sd->ed);
		}
		skill_unitsetting(target, MG_SAFETYWALL, skill_lv + 5, target->x, target->y, 0);
		skill_unitsetting(target, AL_PNEUMA, 1, target->x, target->y, 0);
	}
	else {
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		if (sd->ed && skill_get_state(getSkillId()) == ST_ELEMENTALSPIRIT2)
			elemental_delete(sd->ed);
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(),skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
	}
}
