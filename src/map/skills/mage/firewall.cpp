// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firewall.hpp"

SkillFireWall::SkillFireWall() : SkillImpl(MG_FIREWALL) {
}

void SkillFireWall::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio -= 50;
}
