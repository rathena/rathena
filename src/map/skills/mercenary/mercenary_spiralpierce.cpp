// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_spiralpierce.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillMercenarySpiralPierce::SkillMercenarySpiralPierce() : WeaponSkillImpl(ML_SPIRALPIERCE) {
}

void SkillMercenarySpiralPierce::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += 50 + 50 * skill_lv;
	RE_LVL_DMOD(100);
#endif
}
