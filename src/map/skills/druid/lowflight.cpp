// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lowflight.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillLowFlight::SkillLowFlight() : SkillImplRecursiveDamageSplash(DR_LOW_FLIGHT) {
}

void SkillLowFlight::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 20 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 20 * skill_lv;
	}

	skillratio += 4 * sstatus->dex;

	RE_LVL_DMOD(100);
}

void SkillLowFlight::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Move the src 1 cell near the target, between the src and the target
	uint8 dir = map_calc_dir(target, src->x, src->y);

	if (!unit_movepos(src, target->x+dirx[dir], target->y+diry[dir], 2, true)) {
		if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	clif_blown(src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
