// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "primalclaw.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

#include "pulseofmadness.hpp"

SkillPrimalClaw::SkillPrimalClaw() : SkillImplRecursiveDamageSplash(AT_PRIMAL_CLAW) {
}

void SkillPrimalClaw::applyCounterAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& attack_type) const {
	// Update madness if the skill hits at least one target
	if (skill_area_temp[3] == 0) {
		skill_area_temp[3] = 1;

		SkillPulseOfMadness::updateMadness(src);
	}
}

void SkillPrimalClaw::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
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

	// Updates the status (even when the attack misses)
	sc_start4(src, src, skill_get_sc(getSkillId()), 100, getSkillId(), skill_lv, 0, 0, skill_get_time(getSkillId(), skill_lv));

	// Whether the skill hits at least one target
	skill_area_temp[3] = 0;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

int16 SkillPrimalClaw::getSplashSearchSize(block_list* src, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(src);

	// Madness (at least level 2) : changes area of effect to 7 x 7 cells.
	if (sc != nullptr && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3))) {
		return 3;
	}

	return skill_get_splash( this->getSkillId(), skill_lv );
}

void SkillPrimalClaw::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1250 + 1050 * (skill_lv - 1);

	if (sc != nullptr && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3))) {
		skillratio += 800;
	}
	skillratio += sstatus->pow * 5;

	RE_LVL_DMOD(100);
}
