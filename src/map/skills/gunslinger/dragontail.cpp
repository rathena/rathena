// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragontail.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDragonTail::SkillDragonTail() : SkillImplRecursiveDamageSplash(RL_D_TAIL) {
}

int32 SkillDragonTail::getSplashTarget(block_list* src) const {
	return BL_CHAR;
}

void SkillDragonTail::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	const status_change* tsc = status_get_sc(target);

	// TODO: do we need to check the src id?
	if (tsc != nullptr && tsc->hasSCE(SC_C_MARKER) && tsc->getSCE(SC_C_MARKER)->val2 == src->id) {
		skill_area_temp[0] |= SKILL_ALTDMG_FLAG;
	}

	// Disable skill animation
	skill_area_temp[1] = 0;

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillDragonTail::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 500 + 200 * skill_lv;

	if (wd->miscflag & SKILL_ALTDMG_FLAG) {
		skillratio *= 2;
	}

	RE_LVL_DMOD(100);
}
