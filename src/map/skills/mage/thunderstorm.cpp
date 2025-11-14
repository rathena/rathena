// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thunderstorm.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillThunderStorm::SkillThunderStorm() : SkillImpl(MG_THUNDERSTORM) {
}

void SkillThunderStorm::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	// in Renewal Thunder Storm boost is 100% (in pre-re, 80%)
#ifndef RENEWAL
	base_skillratio -= 20;
#endif
}
