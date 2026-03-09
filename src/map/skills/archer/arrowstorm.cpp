// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "arrowstorm.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillArrowStorm::SkillArrowStorm() : SkillImplRecursiveDamageSplash(RA_ARROWSTORM) {
}

void SkillArrowStorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	if (sc && sc->getSCE(SC_FEARBREEZE))
		skillratio += -100 + 200 + 250 * skill_lv;
	else
		skillratio += -100 + 200 + 180 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillArrowStorm::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);

	status_change_end(src, SC_CAMOUFLAGE);
}
