// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rampageblaster.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillRampageBlaster::SkillRampageBlaster() : WeaponSkillImpl(SR_RAMPAGEBLASTER) {
}

void SkillRampageBlaster::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_change* tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_EARTHSHAKER)) {
		skillratio += 1400 + 550 * skill_lv;
		RE_LVL_DMOD(120);
	} else {
		skillratio += 900 + 350 * skill_lv;
		RE_LVL_DMOD(150);
	}

	if (sc && sc->getSCE(SC_GT_CHANGE))
		skillratio += skillratio * 30 / 100;
}
