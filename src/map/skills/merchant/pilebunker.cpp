// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pilebunker.hpp"

#include <config/core.hpp>

SkillPileBunker::SkillPileBunker() : WeaponSkillImpl(NC_PILEBUNKER) {
}

void SkillPileBunker::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += 200 + 100 * skill_lv + status_get_str(src);
	RE_LVL_DMOD(100);
}
