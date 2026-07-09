// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gateofhell.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillGateOfHell::SkillGateOfHell() : WeaponSkillImpl(SR_GATEOFHELL) {
}

void SkillGateOfHell::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE)
		skillratio += -100 + 800 * skill_lv;
	else
		skillratio += -100 + 500 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc->getSCE(SC_GT_REVITALIZE))
		skillratio += skillratio * 30 / 100;
}
