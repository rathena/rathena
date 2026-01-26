// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "aerosync.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillAeroSync::SkillAeroSync() : SkillImpl(AT_AERO_SYNC) {
}

void SkillAeroSync::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		return;
	}

	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);
	status_change *sc = status_get_sc(src);

	if (!sd || !tsd || !sc) {
		return;
	}

	if (!sc->hasSCE(SC_WERERAPTOR) || !sc->hasSCE(SC_FLIP_FLAP) || sc->hasSCE(SC_FLIP_FLAP_TARGET)) {
		clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		return;
	}

	if (sd->status.party_id == 0 || sd->status.party_id != tsd->status.party_id) {
		clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		return;
	}

	const int32 range = skill_get_range2(src, getSkillId(), skill_lv, true);
	if (range > 0 && distance_xy(src->x, src->y, target->x, target->y) > range) {
		clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		return;
	}

	if (!unit_movepos(src, target->x, target->y, 2, true)) {
		clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		return;
	}

	clif_blown(src);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	const int32 flip_lv = pc_checkskill(sd, AT_FLIP_FLAP);
	if (flip_lv <= 0) {
		return;
	}

	const t_tick duration = skill_get_time(AT_FLIP_FLAP, flip_lv);
	sc_start(src, target, SC_FLIP_FLAP_TARGET, 100, flip_lv, duration);
}
