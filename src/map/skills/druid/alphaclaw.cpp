// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "alphaclaw.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

#include "pulseofmadness.hpp"

SkillAlphaClaw::SkillAlphaClaw() : SkillImplRecursiveDamageSplash(AT_ALPHA_CLAW) {
}

void SkillAlphaClaw::applyCounterAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& attack_type) const {
	// Update madness if the skill hits at least one target
	if (skill_area_temp[3] == 0) {
		skill_area_temp[3] = 1;

		SkillPulseOfMadness::updateMadness(src);
	}
}

void SkillAlphaClaw::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillAlphaClaw::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	status_change_end(src, SC_PRIMAL_CLAW);
	status_change_end(src, SC_FERAL_CLAW);

	// Whether the skill hits at least one target
	skill_area_temp[3] = 0;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

int16 SkillAlphaClaw::getSplashSearchSize(block_list* src, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(src);

	// Madness (at least level 2) : changes area of effect to 9 x 9 cells.
	if (sc != nullptr && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3))) {
		return 4;
	}

	return skill_get_splash( this->getSkillId(), skill_lv );
}

void SkillAlphaClaw::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 2600 + 1600 * (skill_lv - 1);

	if (sc != nullptr && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3))) {
		skillratio += 800;
	}
	skillratio += sstatus->pow * 10;

	RE_LVL_DMOD(100);
}
