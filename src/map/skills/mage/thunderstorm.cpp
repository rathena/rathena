// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thunderstorm.hpp"

SkillThunderStorm::SkillThunderStorm() : SkillImpl(MG_THUNDERSTORM) {
}

void SkillThunderStorm::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillThunderStorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	// in Renewal Thunder Storm boost is 100% (in pre-re, 80%)
#ifndef RENEWAL
	base_skillratio -= 20;
#endif
}
