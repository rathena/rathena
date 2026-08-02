// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "medialevotum.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMedialeVotum::SkillMedialeVotum() : StatusSkillImpl(CD_MEDIALE_VOTUM) {
}

void SkillMedialeVotum::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1) {
		if (sd == nullptr || sd->status.party_id == 0 || (flag & 2)) {
			int32 heal_amount = skill_calc_heal(src, target, getSkillId(), skill_lv, 1);

			clif_skill_nodamage(nullptr, *target, AL_HEAL, heal_amount);
			status_heal(target, heal_amount, 0, 0);
		} else if (sd)
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag | BCT_PARTY | 3, skill_castend_nodamage_id);
	} else {
		StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
	}
}
