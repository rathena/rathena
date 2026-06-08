// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "savagelunge.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

#include "pulseofmadness.hpp"

SkillSavageLunge::SkillSavageLunge() : SkillImplRecursiveDamageSplash(AT_SAVAGE_LUNGE) {
}

void SkillSavageLunge::applyCounterAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& attack_type) const {
	// Update madness if the skill hits at least one target
	if (skill_area_temp[3] == 0) {
		skill_area_temp[3] = 1;

		SkillPulseOfMadness::updateMadness(src);
	}
}

void SkillSavageLunge::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
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

	// Whether the skill hits at least one target
	skill_area_temp[3] = 0;

	if (status_change* sc = status_get_sc(src); sc != nullptr && !sc->hasSCE(SC_ALPHA_PHASE) && !sc->hasSCE(SC_INSANE2) && !sc->hasSCE(SC_INSANE3)) {
		// Single Target
		SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
		return;
	}

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillSavageLunge::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	if (sc != nullptr && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3))) {
		skillratio += -100 + 9000 + 2000 * (skill_lv - 1);
	}
	else {
		skillratio += -100 + 7500 + 1500 * (skill_lv - 1);
	}
	skillratio += sstatus->pow * 10;

	RE_LVL_DMOD(100);
}
