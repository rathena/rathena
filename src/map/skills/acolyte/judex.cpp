// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "judex.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillJudex::SkillJudex() : SkillImplRecursiveDamageSplash(AB_JUDEX) {
}

void SkillJudex::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 300 + 70 * skill_lv;
	RE_LVL_DMOD(100);
}
