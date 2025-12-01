// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chargearrow.hpp"

SkillChargeArrow::SkillChargeArrow() : WeaponSkillImpl(AC_CHARGEARROW)
{
}

void SkillChargeArrow::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const
{
	base_skillratio += 50;
}
