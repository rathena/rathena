// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "axeboomerang.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillAxeBoomerang::SkillAxeBoomerang() : WeaponSkillImpl(NC_AXEBOOMERANG) {
}

void SkillAxeBoomerang::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += 150 + 50 * skill_lv;
	if (sd) {
		int16 index = sd->equip_index[EQI_HAND_R];

		if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
			skillratio += sd->inventory_data[index]->weight / 10;// Weight is divided by 10 since 10 weight in coding make 1 whole actual weight. [Rytech]
	}
	RE_LVL_DMOD(100);
}
