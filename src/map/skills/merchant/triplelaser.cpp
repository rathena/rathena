// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "triplelaser.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillTripleLaser::SkillTripleLaser() : WeaponSkillImpl(MT_TRIPLE_LASER) {
}

void SkillTripleLaser::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 650 + 1150 * skill_lv;
	skillratio += 12 * sstatus->pow;
	RE_LVL_DMOD(100);
}
