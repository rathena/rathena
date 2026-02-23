// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "typhoonwing.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillTyphoonWing::SkillTyphoonWing() : SkillImplRecursiveDamageSplash(KR_TYPHOON_WING) {
}

void SkillTyphoonWing::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 600 + 80 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 300;
	}

	skillratio += 4 * sstatus->dex;

	RE_LVL_DMOD(100);
}
