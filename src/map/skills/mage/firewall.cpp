// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firewall.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFireWall::SkillFireWall() : SkillImpl(MG_FIREWALL) {
}

void SkillFireWall::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio -= 50;
}
