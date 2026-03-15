// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "jackfrost.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillJackFrost::SkillJackFrost() : SkillImplRecursiveDamageSplash(WL_JACKFROST) {
}

void SkillJackFrost::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_MISTY_FROST))
		skillratio += -100 + 1200 + 600 * skill_lv;
	else
		skillratio += -100 + 1000 + 300 * skill_lv;
	RE_LVL_DMOD(100);
}
