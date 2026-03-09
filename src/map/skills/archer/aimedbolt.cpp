// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "aimedbolt.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillAimedBolt::SkillAimedBolt() : WeaponSkillImpl(RA_AIMEDBOLT) {
}

void SkillAimedBolt::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	if (sc && sc->getSCE(SC_FEARBREEZE))
		skillratio += -100 + 800 + 35 * skill_lv;
	else
		skillratio += -100 + 500 + 20 * skill_lv;	
	RE_LVL_DMOD(100);
}
