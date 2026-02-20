// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "boostknuckle.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillBoostKnuckle::SkillBoostKnuckle() : WeaponSkillImpl(NC_BOOSTKNUCKLE) {
}

void SkillBoostKnuckle::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 260 * skill_lv + sstatus->dex; // !TODO: What's the DEX bonus?
	RE_LVL_DMOD(100);
}
