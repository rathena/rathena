// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "curseexplosion.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillCurseExplosion::SkillCurseExplosion() : SkillImplRecursiveDamageSplash(SP_CURSEEXPLOSION) {
}

void SkillCurseExplosion::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_SOULCURSE))
		skillratio += -100 + 1200 + 300 * skill_lv;
	else
		skillratio += -100 + 400 + 100 * skill_lv;
	RE_LVL_DMOD(100);
}
