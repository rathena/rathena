// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tacticalrepositioning.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillTacticalRepositioning::SkillTacticalRepositioning() : SkillImpl(NW_TACTICAL_REPOSITIONING) {
}

void SkillTacticalRepositioning::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);

	if (sc == nullptr || !sc->hasSCE(SC_INTENSIVE_AIM) || !sc->hasSCE(SC_INTENSIVE_AIM_COUNT) || sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 <= 0 ||
		map_getcell(src->m, x, y, CELL_CHKNOPASS) ||
		!path_search_long(nullptr, src->m, src->x, src->y, static_cast<int16>(x), static_cast<int16>(y), CELL_CHKNOPASS) ||
		!unit_movepos(src, static_cast<int16>(x), static_cast<int16>(y), 1, true)) {
		if (sd != nullptr)
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL_POS);
		return;
	}

	clif_blown(src);

	clif_skill_poseffect(*src, getSkillId(), skill_lv, x, y, tick);

	// Consume one aim count (SC_INTENSIVE_AIM keeps its own counter in val4).
	int32 left = sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 - 1;

	sc->getSCE(SC_INTENSIVE_AIM)->val4 = std::max(0, left);
	// The generic overlap rule rejects a smaller val1. Remove the old count first.
	status_change_end(src, SC_INTENSIVE_AIM_COUNT);
	if (left > 0)
		sc_start(src, src, SC_INTENSIVE_AIM_COUNT, 100, left, INFINITE_TICK);
}
