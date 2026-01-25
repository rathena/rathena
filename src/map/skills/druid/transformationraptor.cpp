// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "transformationraptor.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTransformationRaptor::SkillTransformationRaptor() : SkillImpl(DR_WERERAPTOR) {
}

void SkillTransformationRaptor::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);

	if (tsc && tsc->hasSCE(type)) {
		bool ended = status_change_end(target, type);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, ended);
		return;
	}

	if (tsc && tsc->hasSCE(SC_TRANSFORM_DELAY)) {
		map_session_data *sd = BL_CAST(BL_PC, src);
		if (sd) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, INFINITE_TICK));
}
