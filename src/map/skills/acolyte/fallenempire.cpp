// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fallenempire.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillFallenEmpire::SkillFallenEmpire() : WeaponSkillImpl(SR_FALLENEMPIRE) {
}

void SkillFallenEmpire::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	// ATK [(Skill Level x 300 + 100) x Caster Base Level / 150] %
	skillratio += 300 * skill_lv;
	RE_LVL_DMOD(150);
}
